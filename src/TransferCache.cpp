#include "TransferCache.h"

void TransferCache::init_arena(MetaArena* m_arena) {
    arena = m_arena;
}

void TransferCache::init_pm(PageMap* map) {
    pm = map;
}

void TransferCache::init_ph(PageHeap* heap) {
    ph = heap;
}

FreeBlock* TransferCache::popFromList(size_t mem_size) {
    
}