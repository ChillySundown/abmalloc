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
    size_t num_objects = (s->num_pages << K_PAGE_SHIFT) / size_class;

    s->size_class = size_class;
    s->objects = reinterpret_cast<FreeBlock*>(s->starting_page_id << K_PAGE_SHIFT);
    FreeBlock* w = s->objects;
    for(size_t idx = 0; idx < num_objects; idx += 1) {
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

void CentralFreeList::refillFreeList() {
    if(!ph) {
        init_ph(&page_heap());
    } 
    //No need to worry about misses, because every size class is mapped
    Span* refill = ph->pageAlloc(free_list_refill_sizes.at(size_class));
    init_span_objects(refill);
    
}

FreeBlock* CentralFreeList::popFromList() {
        if(!free_list) {
            //Refill free list
        } else {
            Span* s = free_list;
            while(s && !s->objects) {
                s = s->next;
            }
            if(s) {
                s->current_count++;
                return popBlock(s->objects);
            } else {
                //Refill free list
            }
        }
}