#pragma once

#define ListForEach(list, iter) {\
    auto __current = (list)->head;              \
    while (__current) { \
    auto iter = &__current->value;

#define ListEndEach \
    __current = __current->next; }}


template <typename T>
struct ListNode {
    ListNode* next;
    ListNode* prev;
    T value;
};

template <typename T>
struct List {
    ListNode<T>* head;
    u32 count;
    Allocator allocator;
};

template <typename T>
List<T> CreateList(Allocator allocator);

template <typename T>
T* ListAdd(List<T>* list);

template <typename T>
void ListRemove(List<T>* list, T* elem);
