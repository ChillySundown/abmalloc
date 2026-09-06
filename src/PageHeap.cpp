#include "PageHeap.h"
#include "globals.h"

void PageHeap::init_arena(MetaArena* arena) {
    mem_arena = arena;
}

void PageHeap::init_pm(PageMap* map) {
    pm = map;
}

MetaArena* PageHeap::getMetaArena() {
    return mem_arena;
}

PageMap* PageHeap::getPageMap() {
    return pm;
}

Span** PageHeap::getFreeLists() {
    return free_page_lists;
}

Span* PageHeap::popFreeSpan() {
    if(!free_spans) {
        if(!mem_arena) {
            return nullptr;
        }
        return static_cast<Span*>(mem_arena->allocate(sizeof(Span)));
    }
    Span* s = free_spans;
    free_spans = free_spans->next;
    if(free_spans) {
        free_spans->prev = nullptr;
    }
    return s;
}

//Pushes a retired span onto the free list;
void PageHeap::pushFreeSpan(Span* s) {
    if(free_spans) {
        free_spans->prev = s;
    }
    //Clears value of span for cleanliness
    s->starting_page_id = 0;
    s->num_pages = 0;
    s->prev = nullptr;

    //Pushes s onto free_list
    s->next = free_spans;
    free_spans = s;

}

//Pushes a Span onto free_list[index]
void PageHeap::pushPages(size_t page_size, Span* s) {
    assert(page_size != 0);
    size_t index = std::min(page_size-1, MAX_PAGEHEAP_IDX);
    s->next = free_page_lists[index]; //Might cause index error
    if(free_page_lists[index]) {
        free_page_lists[index]->prev = s;
    }
    free_page_lists[index] = s;
}

//Refills page heap
bool PageHeap::refillPageHeap(size_t page_size) {
    size_t req_bytes = std::max(page_size * K_PAGE_SIZE, PAGEHEAP_REFILL_SIZE);
    void* fresh_mem = mmap(nullptr, req_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(fresh_mem == MAP_FAILED) {return false;}

    size_t start = reinterpret_cast<uintptr_t>(fresh_mem) >> K_PAGE_SHIFT;
    size_t length = req_bytes >> K_PAGE_SHIFT;

    bool ensure_pagemap = pm->ensure(start, length);
    Span* new_span = popFreeSpan();
    if(!new_span || !ensure_pagemap) {
        munmap(fresh_mem, req_bytes);
        return false;
    }

    new_span->starting_page_id = start;
    new_span->num_pages = length;
    if(req_bytes == (page_size * K_PAGE_SIZE)) {
        new_span->status = SpanState::LARGE_OBJ;
    }
    //All spans default state is SpanState::FREE

    for(size_t idx = start; idx < (start + length); idx++) {
        pm->set(idx, new_span);
    }
    total_mapped_pages += std::max(page_size, PAGEHEAP_REFILL_SIZE / K_PAGE_SIZE);
    pushPages(new_span->num_pages, new_span);
    return true;
}

//Given an index in the pageheap, either pop a one-page span or carve a smaller span from a larger page
Span* PageHeap::popPages(size_t index, size_t page_length) {
    //Assume that s->num_pages will never be smaller than page_length
    Span* s = free_page_lists[index];
    PageMap* global_map = pm;

    //If num_pages is free, return the span
    if(s->num_pages == page_length) {
        unlinkPages(s);
        s->status = SpanState::IN_USE;
        return s;
    } else { //If greater page size than requested, carve from span and return new span
        while(s && s->num_pages < page_length) {
            s = s->next;
        }
    }
    Span* new_span = popFreeSpan();
    if(!new_span) {return nullptr;}
    new_span->starting_page_id = (s->starting_page_id + s->num_pages) - page_length;
    new_span->num_pages = page_length;
    new_span->status = SpanState::IN_USE;
    //Maps each page in the span to the new_span
    for(size_t idx = new_span->starting_page_id; idx < new_span->starting_page_id + new_span->num_pages; idx++) {
        global_map->set(idx, new_span);
    }
    
    //Relocate span to new region
    unlinkPages(s);
    s->num_pages -= page_length; 
    pushPages(s->num_pages, s);

    //s->starting_page_id -= page_length;
    return new_span;
}

void PageHeap::unlinkPages(Span* s) {
    assert(s != nullptr);
    size_t idx = std::min(s->num_pages-1, MAX_PAGEHEAP_IDX);
    if(s == free_page_lists[idx]) {
        free_page_lists[idx] = s->next; //Uhh what if we keep moving head forward and have memory leak
    } else {
        Span* prev_span = s->prev;
        prev_span->next = s->next;
        if(s->next && s->next->prev) {
            s->next->prev = prev_span;
        }
    }
}

void PageHeap::retireSpan(Span* s) {
    s->status = SpanState::FREE;
    pushFreeSpan(s);
}

void PageHeap::mergeSpans(Span* s, Span* r) {
    if(!s || !r) {return;}
    size_t start = r->starting_page_id;
    size_t len = r->num_pages;

    s->num_pages += r->num_pages;
    retireSpan(r);
    for(size_t idx = start; idx < start + len; idx++) {
        pm->set(idx, s);
    }
}
Span* PageHeap::pageAlloc(size_t page_size) {
    if(page_size <= 0) {return nullptr;}
    size_t size_index = std::min(page_size - 1, MAX_PAGEHEAP_IDX);
    if(size_index > 255) { size_index = 255;} //List of large pages
    
    if(free_page_lists[size_index]) { //First: try to pop from respective page list
        return popPages(size_index, page_size);
    } else {
        while(size_index <= 255) { //Second: Iterate through all larger page lists until a free page is found
            if(free_page_lists[size_index]) {
                return popPages(size_index, page_size);
            } else {
                size_index += 1;
            }
        }
    }

    //If the pageheap is empty
    if(!refillPageHeap(page_size)) {
        return nullptr;
    } else {
        return pageAlloc(page_size); //Recursion might give us some trouble
    }
     //Temporary -- //Third: Allocate a chunk of memory that can be sliced and diced for pages

}

void PageHeap::pageFree(Span* pages) {
    //Check
    //Set pages->state to FREE eventually
    if(!pages) {return;}
    else if(pages->status == SpanState::FREE) {return;}
    pages->status = SpanState::FREE;

    Span* current = pages;
    if(pages->starting_page_id > 0) { //Backwards merge
        size_t prev_page_id = pages->starting_page_id - 1;
        Span* prev_pages = pm->get(prev_page_id);

        //If prev_page exists and is ALSO free, merge pages
        if(prev_pages && prev_pages->status == SpanState::FREE) {
            unlinkPages(prev_pages);
            mergeSpans(prev_pages, pages);
            current = prev_pages;
        }
    }

    Span* next_pages = pm->get(current->starting_page_id + current->num_pages); 
    if(next_pages && next_pages->status == SpanState::FREE) {
        unlinkPages(next_pages);
        mergeSpans(current, next_pages);
    }
    pushPages(current->num_pages, current);
        //Do I need to call e
}