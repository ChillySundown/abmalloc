#include "PageMap.h"
#include "Span.h"
#include "globals.h"

int getPageRootIdx(size_t page_id) {
    assert(page_id >> PAGEMAP_TOTAL_BITS);
    return page_id >> (PAGEMAP_BRANCH_BITS + PAGEMAP_LEAF_BITS) & (PAGEMAP_ROOT_SIZE-1);
}

int getPageBranchIdx(size_t page_id) {
    return page_id >> (PAGEMAP_LEAF_BITS) & (PAGEMAP_BRANCH_SIZE-1);
}

int getPageLeafIdx(size_t page_id) {
    return page_id & (PAGEMAP_LEAF_SIZE-1);
}

size_t PageMap::getBytesAllocated() {
    if(mem_arena) {
        return mem_arena->byte_count;
    } else {
        return 0;
    }
}

void PageMap::init_arena(MetaArena* arena) {
    mem_arena = arena;
}

Span* PageMap::get(size_t page_id) {
    size_t root_idx = getPageRootIdx(page_id);
    size_t branch_idx = getPageBranchIdx(page_id);
    int leaf_idx = getPageLeafIdx(page_id);

    auto* branch_entry = map_root.arr[root_idx];
    if(!branch_entry) {return nullptr;}
    auto* leaf_entry = branch_entry->arr[branch_idx];
    if(!leaf_entry) {return nullptr;}
    return leaf_entry->arr[leaf_idx];

}

void PageMap::set(size_t page_id, Span* span) {
    int root_idx = getPageRootIdx(page_id);
    int branch_idx = getPageBranchIdx(page_id);
    int leaf_idx = getPageLeafIdx(page_id);

    auto* branch_entry = map_root.arr[root_idx];
    assert(branch_entry); //Checks to make sure branch_entry is good
    auto* leaf_entry  = branch_entry->arr[branch_idx];
    assert(leaf_entry);
    leaf_entry->arr[leaf_idx] = span;
}

bool PageMap::ensure(size_t start_page, size_t length) {
    assert(mem_arena != nullptr);
    size_t idx = start_page;
    size_t end_idx = idx + length;
    while(idx < end_idx) {
        size_t root_idx = getPageRootIdx(idx);
        size_t branch_idx = getPageBranchIdx(idx);
        size_t leaf_idx = getPageLeafIdx(idx);

        auto* branch = map_root.arr[root_idx];
        if(!branch) {
            branch = static_cast<PageMapBranch*>(mem_arena->allocate(sizeof(PageMapBranch))); 
            if(branch) {
                map_root.arr[root_idx] = branch;
            } else {
                return false;
            }
        }
        auto* leaf = branch->arr[branch_idx];
        if(!leaf) {
            leaf = static_cast<PageMapLeaf*>(mem_arena->allocate(sizeof(PageMapLeaf)));
            if(leaf) {
                branch->arr[branch_idx] = leaf;
            } else {
                return false;
            }
        }
        idx += std::min((PAGEMAP_LEAF_SIZE - leaf_idx), (end_idx - idx));

    }
    return true;
}

size_t PageMap::getSizeClass(size_t page_id) {
    Span* s = get(page_id);
    if(!s) {
        return 0;
    } else {
        return s->size_class;
    }
}