#pragma once

#define ListForEach(list, iter) do {\
    auto __current = (list)->head;\
    while (__current) { \
    auto iter = &__current->value;

#define ListEndEach __current = __current->next; }} while(false)

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
