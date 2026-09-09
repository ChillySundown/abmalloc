#ifndef TRANSFER_CACHE_H
#define TRANSFER_CACHE_H

#include "CentralFreeList.h"
#include <mutex>
class TransferCache {
    private:
        std::mutex lock;
        FreeBlock* list[32] {nullptr};
        size_t count {0};

        MetaArena* arena {nullptr};
        PageHeap* ph {nullptr};
        PageMap* pm {nullptr};  
        CentralFreeList* free_list {nullptr};
    public:
        void init_arena(MetaArena* m_arena);
        void init_ph(PageHeap* heap);
        void init_pm(PageMap* map);

        FreeBlock* popFromList(size_t mem_size);
        void returnToList(FreeBlock* block, size_t n_blocks);

};

#endif