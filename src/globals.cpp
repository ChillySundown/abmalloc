#include "globals.h"
//static unsigned char* heap_ptr = static_cast<unsigned char*>(mmap(nullptr, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0)); //pointer to our heap

static MetaArena g_meta_arena;
static PageMap g_page_map;
static PageHeap g_page_heap;
static CentralFreeList g_free_lists[8]; //One Free List for each size class
static TransferCache g_transfer_caches[8]; //One transfer cache for each size class


//Block* heap_head {nullptr};

MetaArena& meta_arena() {
    return g_meta_arena;
}

PageMap& page_map() {
    if(!g_page_map.mem_arena) {
        g_page_map.init_arena(&g_meta_arena);
    }
    return g_page_map;
}

PageHeap& page_heap() {
    if(!g_page_heap.getMetaArena()) {
        g_page_heap.init_arena(&meta_arena());
    }
    if(!g_page_heap.getPageMap()) {
        g_page_heap.init_pm(&page_map());
    }
    return g_page_heap;
}

CentralFreeList& cfl(size_t size_class) {
    size_t idx = (std::find(size_classes, size_classes + 8, size_class) - size_classes);
    CentralFreeList& free_list = g_free_lists[idx];
    return free_list;
}

size_t align_up(size_t bytes, size_t alignment) {
    return (bytes + (alignment-1)) & ~(alignment - 1);
}

// void init_heap() {
//    heap_head = reinterpret_cast<Block*>(heap_ptr); //Intializes our block linked list with ptr from our heap array
//    heap_head->free = true;
//    heap_head->next = nullptr;
//    heap_head->size = HEAP_SIZE - sizeof(Block);
// }


//Memory allocator for metadata
// void* meta_malloc(size_t bytes) {
//     return meta_space.allocate(bytes);
// }

//Aligns bytes upward by given power

// void split_free_block(Block* ptr, size_t split_size) {
//     //Steps into memory payload, moves by split_size bytes, and creates a new header
//     auto* new_block {reinterpret_cast<Block*>(reinterpret_cast<unsigned char*>(ptr + 1) + split_size)};

//     new_block->size = ptr->size - split_size - sizeof(Block);
//     new_block->free = true;
//     new_block->next = ptr->next;

//     ptr->next = new_block;
//     ptr->size = split_size;
// }

// void merge_free_blocks() {
//     auto* current {heap_head};

//     while(current && current->next) {
//         if(current->free && current->next->free) {
//             // Block* new_block {current};
//             // new_block->size = current->size + current->next->size;
//             // new_block->free = true;
//             current->size += current->next->size + sizeof(Block);
//             current->next = current->next->next;
//         }
//         current = current->next;
//     }
// }

// void* mallocate(size_t bytes) {
//     if(bytes == 0) { return nullptr; }
//     if(!heap_head) { init_heap(); }
//     bytes = align_up(bytes, alignof(double));

//     auto* w = heap_head;
//     while(w) {
//         if(w->free && w->size >= bytes) {
//             split_free_block(w, bytes);
//             w->free = false;
//             return w + 1; //Moves past header to payload
//         }

//         w = w->next;
//     }
//     return nullptr;
// }
// void deallocate(void* ptr) {
//     if(!ptr) {
//         return;
//     }
//     auto* new_header {reinterpret_cast<Block*>(ptr) - 1}; //Removes back by sizeof(Block) bytes and creates a new header
//     new_header->free = true;
//     merge_free_blocks();
// }

// int main() {
//     std::cout << "Size of pages on Apple Silicon: " << page_size << std::endl;
//     auto* a {mallocate(10)};
//     auto* b {mallocate(20)};
//     print_heap();
//     deallocate(a);
//     deallocate(b);

//     print_heap();
//     auto* c {mallocate(30)};

//     return 0;
// }