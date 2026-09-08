cmake_minimum_required(VERSION 3.10)

project(abmalloc)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

set(LIB_SOURCES 
    src/globals.cpp 
    src/MetaArena.cpp 
    src/PageHeap.cpp 
    src/PageMap.cpp 
    src/CentralFreeList.cpp
    src/TransferCache.cpp
)

add_library(abmalloc_lib STATIC ${LIB_SOURCES})
target_include_directories(abmalloc_lib PUBLIC src)

add_executable(abmalloc_test test/test.cpp)
target_link_libraries(abmalloc_test PRIVATE abmalloc_lib)
target_compile_options(abmalloc_test PRIVATE -fsanitize=address,undefined -g)
target_link_options(abmalloc_test PRIVATE -fsanitize=address,undefined)

#add_executable(abmalloc_bench benchmark.cpp) Add benchmark for testing
