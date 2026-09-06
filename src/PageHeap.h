#ifndef PAGEHEAP_H
#define PAGEHEAP_H

#include "Span.h"
#include "PageMap.h"


class PageHeap {
    private:
        //Something something mutex
        Span* free_page_lists[256] {nullptr}; // idx = 0 - 1 page, idx = 254 - 255 pages, idx = 255 - 256<= pages
        Span* free_spans {nullptr};
        MetaArena* mem_arena {nullptr};
        PageMap* pm {nullptr};

        void unlinkPages(Span* s);
        bool refillPageHeap(size_t page_size);
        Span* popFreeSpan();
        void pushFreeSpan(Span* s);
        Span* popPages(size_t index, size_t page_length);
        void retireSpan(Span* s);
        void pushPages(size_t index, Span* s);
        void mergeSpans(Span* s, Span* r);
    public: 
        
        Span** getFreeLists();
        size_t total_mapped_pages {0};
        void init_arena(MetaArena* arena);
        void init_pm(PageMap* map);
        MetaArena* getMetaArena();
        PageMap* getPageMap();

        
        Span* pageAlloc(size_t num_pages);
        void pageFree(Span* pages);
    
};

#endif