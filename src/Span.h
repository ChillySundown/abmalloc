#include <cstddef>
#include <cstdint>
#ifndef SPAN_H
#define SPAN_H

enum class SpanState : uint8_t {FREE, IN_USE, LARGE_OBJ};

struct FreeBlock { //Similar to block, but size is already known by size_class
    FreeBlock* next {nullptr};
};

struct Span {
    size_t starting_page_id;
    size_t num_pages;
    Span* next {nullptr};
    Span* prev {nullptr};
    SpanState status {SpanState::FREE};
    FreeBlock* objects {nullptr};
    size_t size_class;
    size_t current_count;
};

#endif