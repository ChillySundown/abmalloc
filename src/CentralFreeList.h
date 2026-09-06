#include "MetaArena.h"
#include "constants.h"
#include "globals.h"
#include "PageMap.h"
#include "PageHeap.h"
class CentralFreeList {
    private:
        Span* free_list {nullptr};
        size_t size_class;
        FreeBlock* popBlock(FreeBlock*& list);
        void init_span_objects(Span* s);
        void refillFreeList();

        MetaArena* mem_arena {nullptr};
        PageMap* pm {nullptr};
        PageHeap* ph {nullptr};
    public:
        void set_size_class(size_t size);
        void init_arena(MetaArena* arena);
        void init_pm(PageMap* map);
        void init_ph(PageHeap* heap);


        //Assume that requests for large objects skips free list entirely
        FreeBlock* popFromList();
        void returnToList(FreeBlock* blk);
};