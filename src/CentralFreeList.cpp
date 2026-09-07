#include "CentralFreeList.h"

void CentralFreeList::set_size_class(size_t size) {
    size_class = size;
}

void CentralFreeList::init_arena(MetaArena* arena) {
    mem_arena = arena;
}

void CentralFreeList::init_pm(PageMap* map) {
    pm = map;
}

void CentralFreeList::init_ph(PageHeap* heap) {
    ph = heap;
}

void CentralFreeList::init_span_objects(Span* s) {
    assert(size_class != 0);
    size_t num_objects = (s->num_pages << K_PAGE_SHIFT) / size_class;

    s->size_class = size_class;
    s->objects = reinterpret_cast<FreeBlock*>(s->starting_page_id << K_PAGE_SHIFT);
    //s->status = SpanState::IN_USE;
    s->current_count = 0;

    FreeBlock* w = s->objects;
    for(size_t idx = 1; idx < num_objects; idx += 1) {
        w->next = reinterpret_cast<FreeBlock*>(reinterpret_cast<uintptr_t>(w) + s->size_class);
        w = w->next;
    }
    w->next = nullptr;
}

FreeBlock* CentralFreeList::popBlock(FreeBlock*& list) {
    //Caller should check if list is null
    FreeBlock* head = list;
    list = list->next;
    return head;
}

void CentralFreeList::pushBlock(FreeBlock*& head, FreeBlock* obj) {
    obj->next = head;
    head = obj;
}

void CentralFreeList::unlinkSpan(Span* s) {
    assert(s);
    if(s == free_list) {
        free_list = s->next;
    } else {
        s->prev->next = s->next;
    }
}

bool CentralFreeList::refillFreeList() {
    if(!ph) {
        init_ph(&page_heap());
    } 
    //No need to worry about misses, because every size class is mapped
    Span* refill = ph->pageAlloc(free_list_refill_sizes.at(size_class));
    if(!refill) {return false;}
    init_span_objects(refill);

    refill->next = free_list;
    free_list = refill;
    return true;
}

FreeBlock* CentralFreeList::popFromList() {
        if(!free_list) {
            if(refillFreeList()) {
                return popFromList();
            } else { //Failure case
                return nullptr;
            }
        } else {
            Span* s = free_list;
            while(s && !s->objects) {
                s = s->next;
            }
            if(s) {
                s->current_count++;
                return popBlock(s->objects);
            } else if(refillFreeList()) {
                return popFromList();
            } else {
                return nullptr;
            }
        }
}

//Assume that the transfer cache/thread cache wont call return on large_obj
//Who calls CentralFreeList? Transfer Cache or Thread Cache
void CentralFreeList::returnToList(FreeBlock* blk) {
    size_t page_id = reinterpret_cast<uintptr_t>(blk) >> K_PAGE_SHIFT;
    Span* parent_span = pm->get(page_id);
    if(!parent_span) {return;}
    assert(parent_span->size_class == size_class);
    pushBlock(parent_span->objects, blk); 

    //If all objects in span are sitting in free_list, return to pageheap
    if(parent_span->current_count == 0) {
        unlinkSpan(parent_span);
        parent_span->objects = nullptr; //Clears out objects
        ph->pageFree(parent_span);
    } else { //Decrement num_checked out
        parent_span->current_count--;
    }
    
    //TODO:
    //Write tests for CentralFreeList
    //Figure out refactors to enable large obj allocation in pageheap
    
}