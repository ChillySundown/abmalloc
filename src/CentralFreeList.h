#include "MetaArena.h"
#include "constants.h"
#include "globals.h"
#include "PageMap.h"
#include "PageHeap.h"
class CentralFreeList {
    private:
        Span* free_list {nullptr};
        size_t size_class {0};
        FreeBlock* popBlock(FreeBlock*& list);
        void pushBlock(FreeBlock*& head, FreeBlock* obj);

        void unlinkSpan(Span* s);
        void init_span_objects(Span* s);
        bool refillFreeList();

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