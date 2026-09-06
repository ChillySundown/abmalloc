#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>
#include <print>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

constexpr int K_PAGE_SHIFT {14};
constexpr size_t K_PAGE_SIZE {1 << K_PAGE_SHIFT};

constexpr int PAGEMAP_ROOT_BITS {12};
constexpr int PAGEMAP_BRANCH_BITS {9};
constexpr int PAGEMAP_LEAF_BITS {13};

static_assert(PAGEMAP_ROOT_BITS + PAGEMAP_BRANCH_BITS + PAGEMAP_LEAF_BITS == 34);

constexpr size_t PAGEMAP_ROOT_SIZE = 1ull << PAGEMAP_ROOT_BITS;
constexpr size_t PAGEMAP_BRANCH_SIZE = 1ull << PAGEMAP_BRANCH_BITS;
constexpr size_t PAGEMAP_LEAF_SIZE = 1ull << PAGEMAP_LEAF_BITS;

constexpr size_t size_classes[] = {8, 16, 32, 64, 128, 256, 512, 1024};
static const std::unordered_map<size_t, size_t> free_list_refill_sizes = {{8, 1}, {16, 1}, {32, 1}, {64, 1}, {128, 1}, {256, 2}, {512, 2}, {1024, 2}};
constexpr size_t HEAP_SIZE {1024 * 1024}; //Represents the size of our heap in bytes (128 to be exact)
//alignas(8) static unsigned char heap[HEAP_SIZE]; //Our actual pool of memory
constexpr size_t PAGEHEAP_REFILL_SIZE {2048 * K_PAGE_SIZE};
constexpr size_t MAX_PAGEHEAP_IDX = 255;

static const size_t page_size = sysconf(_SC_PAGESIZE);
constexpr size_t K_MAX_SIZE = static_cast<size_t>(size_classes[7]);

// struct Block {
//     size_t size {};
//     bool free {};
//     Block* prev {nullptr};
//     Block* next {nullptr};
// }; 

// void* mallocate(size_t bytes);
// void deallocate(void* ptr);

#endif
