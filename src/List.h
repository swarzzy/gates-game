#pragma once

#define ListForEach(list, iter) do {\
    auto concat(iter, __current) = (list)->head;    \
    while (concat(iter, __current)) { \
    auto iter = &concat(iter, __current)->value;

#define ListEndEach(iter) concat(iter, __current) = concat(iter, __current)->next; }} while(false)

template <typename T>
struct ListNode {
    ListNode* next;
    ListNode* prev;
    T value;
};

template <typename T>
struct List {
    ListNode<T>* head = nullptr;
    Allocator* allocator = nullptr;
    u32 count = 0;

    List() = default;
    List(Allocator* alloc) : allocator(alloc) {}

    T* Add();
    void Remove(T* elem);
};
