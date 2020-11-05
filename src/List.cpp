#include "List.h"

template <typename T>
ListNode<T>* ListGetNode(List<T>* list) {
    auto node = list->allocator->Alloc<ListNode<T>>();
    return node;
}

template <typename T>
void ListFreeNode(List<T>* list, ListNode<T>* node) {
    list->allocator->Dealloc(node);
}

template <typename T>
T* List<T>::Add() {
    auto node = ListGetNode(this);
    if (node) {
        if (head) {
            head->prev = node;
        }

        node->next = head;
        head = node;
        count++;
    }
    return &node->value;
}

template <typename T>
void List<T>::Remove(T* elem) {
    assert(count > 0);
    auto node = (ListNode<T>*)((uptr)elem - offset_of(ListNode<T>, value));
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    if (head == node) {
        head = node->next;
    }
    ListFreeNode(this, node);
    count--;
}
