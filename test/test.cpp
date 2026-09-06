#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "globals.h"
#include "PageMap.h"
#include <cmath>
#include <random>
#include <algorithm>
/*
Metadata Malloc tests
-Alignment Test
-Memory Overlap Test
*/

TEST_CASE("Testing Meta Malloc byte alignment") {
    MetaArena meta_space;
    for(int i = 1; i < 1024; i += 3) {
        void* p = meta_space.allocate(i);
        REQUIRE(p != nullptr);
        CHECK(reinterpret_cast<uintptr_t>(p) % alignof(max_align_t) == 0);
    }
};

TEST_CASE("Testing Meta Mallocs no memory overlap") {
    MetaArena meta_space;
    std::vector<std::pair<uintptr_t, size_t>> allocs;
    for(int i = 0; i < 2000; i++) {
        size_t alloc_size = 1 + (rand() % 200); //Selects random size from 1 to 200 bytes
        void* ptr = meta_space.allocate(alloc_size);
        REQUIRE(ptr != nullptr);
        allocs.push_back({reinterpret_cast<uintptr_t>(ptr), alloc_size});
    }

    std::sort(allocs.begin(), allocs.end());

    for(int i = 1; i < allocs.size(); i++) {
        CHECK(allocs[i-1].first + allocs[i-1].second <= allocs[i].first);
    }
}

TEST_CASE("Testing if Meta Malloc memory is real") {
    MetaArena meta_space;
    std::vector<std::pair<unsigned char*, size_t>> allocs;
    for(int i = 0; i < 2000; ++i) {
        size_t n = 1 + (rand() % 200); //Randomly chooses n bytes
        unsigned char* ptr = static_cast<unsigned char*>(meta_space.allocate(n));
        REQUIRE(ptr != nullptr);
        std::memset(ptr, i & 0xFF, n);
        allocs.push_back({ptr, n});
    }

    for(size_t i = 0; i < allocs.size(); ++i) {
        for(size_t j = 0; j < allocs[i].second; ++j) { //Checks to see if every byte was initalized to the first byte of i
            REQUIRE(allocs[i].first[j] == static_cast<unsigned char>(i & 0xFF));
        }
    }
}

/*
PageMap Tests
- Idempotence Test
*/

TEST_CASE("Testing if PageMap operations are idempotent") {
    PageMap p_map;
    MetaArena meta_space;
    p_map.init_arena(&meta_space);

    SUBCASE("Overlapping page range should not allocate twice") {
        size_t start = 1000;
        size_t length = 40;
        //First time ensuring
        bool res1 = p_map.ensure(start, length);
        REQUIRE(res1 == true);
        size_t pre_second_call = p_map.getBytesAllocated(); 
        CHECK(pre_second_call != 0);
        //Second time ensuring
        start = 1020;
        bool res2 = p_map.ensure(start, length);
        REQUIRE(res2 == true);
        CHECK(p_map.getBytesAllocated() != 0);
        CHECK(p_map.getBytesAllocated() == pre_second_call);
    }

    SUBCASE("Overlapping page ranges should be able to allocate additional pages") {
        size_t start = 8180; 
        size_t length = 10;
        //First Call
        bool res1 = p_map.ensure(start, length);
        size_t prev_call = p_map.getBytesAllocated();
        //Second Call
        length = 20;
        bool res2 = p_map.ensure(start, length);
        CHECK(p_map.getBytesAllocated() != 0);
        CHECK(p_map.getBytesAllocated() == prev_call + sizeof(PageMapLeaf));

        size_t two_leaves = prev_call + sizeof(PageMapLeaf);
        start = 8200;
        length = 1000;
        bool res3 = p_map.ensure(start, length);
        CHECK(p_map.getBytesAllocated() == two_leaves);

    }

    SUBCASE("Testing if get and set are idempotent") {
        size_t start = 0;
        size_t length = 100;
        bool res1 = p_map.ensure(start, length);
        
        Span a;
        a.starting_page_id = 0;
        a.num_pages = 3;
        p_map.set(a.starting_page_id, &a);

        Span* b = p_map.get(a.starting_page_id);
        CHECK(&a == b);
        
        Span c;
        c.starting_page_id = 900;
        a.num_pages = 1;
        CHECK(p_map.get(c.starting_page_id) == nullptr);
        
    }
}

/*
PAGEHEAP TESTS
*/
TEST_CASE("Testing if PageHeap can allocate pages") {
    MetaArena arena;
    PageMap pm;
    PageHeap ph; 
    ph.init_arena(&arena);
    pm.init_arena(&arena);
    ph.init_pm(&pm);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 300);

    //Allocating random values
    size_t total_pages = 0;
    std::vector<size_t> alloc_sizes(50);
    for(auto i = 0; i < 50; i++) {
        alloc_sizes[i] = dis(gen);
    }
    std::vector<Span*> spans;
    for(auto n : alloc_sizes) {
        auto* s = ph.pageAlloc(n);
        if(n == 0) {    
            CHECK(!s);  
        }
        spans.push_back(s);
    }
    
    std::vector<int> idxs(50);
    for(int i = 0; i < 50; i++) {
        idxs[i] = i;
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(idxs.begin(), idxs.end(), std::default_random_engine(seed));
    while(!idxs.empty()) {
        int idx = idxs.back();
        ph.pageFree(spans[idx]);
        idxs.pop_back();
    }

    Span** free_lists = ph.getFreeLists();
    size_t total_allocated = 0;
    for(int i = 0; i <= MAX_PAGEHEAP_IDX; i++) {
        if(free_lists[i]) {
            Span* w = free_lists[i];
            while(w) {
                total_allocated += w->num_pages;
                w = w->next;
            }
        }
    }
    CHECK(total_allocated == ph.total_mapped_pages);

}
