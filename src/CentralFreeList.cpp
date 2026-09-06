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

FreeBlock* CentralFreeList::popBlock(FreeBlock* list) {
    //Caller should check if list is null
    FreeBlock* b = list;
    list = list->next;
    return b;
}

FreeBlock* CentralFreeList::popFromList(size_t size) {
    Span* s = CentralFreeList;
    if(!s) {
        CentralFreeList = ph->pageAlloc(8);
        return popFromList(size);
    } else {
        while(s && !s->objects) {
            s = s->next;
        }
        if(!s->objects) { //Be careful of infinite recursion
            //Refill CentralFreeList
            CentralFreeList = ph->pageAlloc(8);
            return popFromList(size);
        }
    }
    return popBlock(s->objects);

}
