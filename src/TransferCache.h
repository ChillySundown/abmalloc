#include "MetaArena.h"
#include "constants.h"
#include "globals.h"
#include "PageMap.h"
class TransferCache {
    private:
        Span* CentralFreeList;
        FreeBlock* free_obj_lists;
    public:
        FreeBlock* alloc(size_t mem_size);
        void free(FreeBlock* ptr);
};