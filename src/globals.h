#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstddef>
#include "CentralFreeList.h"
#include "PageHeap.h"
#include "PageMap.h"
#include "MetaArena.h"
#include "TransferCache.h"
//Given a size in bytes, returns the size rounded up to the nearest power of align_up

MetaArena& meta_arena();
PageMap& page_map();
PageHeap& page_heap();
CentralFreeList& cfl(size_t size_class);
size_t align_up(size_t bytes, size_t align_up);

#endif