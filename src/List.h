#pragma once

template <typename T> struct ListIterator;

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

    inline ListIterator<T> begin() { return { head }; }
    inline ListIterator<T> end() { return { nullptr}; }
};

template <typename T>
List<T> CreateList(Allocator allocator);

template <typename T>
T* ListAdd(List<T>* list);

template <typename T>
void ListRemove(List<T>* list, T* elem);

template <typename T>
struct ListIterator {
    ListNode<T>* current;

    inline ListIterator& operator++() { current = current->next; return *this; }
    inline bool operator==(ListIterator other) { return current == other.current; }
    inline bool operator!=(ListIterator other) { return current != other.current; }
    inline T& operator*() { return current->value; }
};
