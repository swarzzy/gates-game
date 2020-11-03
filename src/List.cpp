#include "List.h"

template <typename T>
List<T> CreateList(Allocator allocator) {
    List<T> list = {};
    list.allocator = allocator;
    return list;
}

template <typename T>
ListNode<T>* ListGetNode(List<T>* list) {
    auto node = list->allocator.Alloc<ListNode<T>>();
    return node;
}

template <typename T>
void ListFreeNode(List<T>* list, ListNode<T>* node) {
    list->allocator.Dealloc(node);
}

template <typename T>
T* ListAdd(List<T>* list) {
    auto node = ListGetNode(list);
    if (node) {
        if (list->head) {
            list->head->prev = node;
        }

        node->next = list->head;
        list->head = node;
        list->count++;
    }
    return &node->value;
}

template <typename T>
void ListRemove(List<T>* list, T* elem) {
    assert(list->count > 0);
    auto node = (ListNode<T>*)((uptr)elem - offset_of(ListNode<T>, value));
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    if (list->head == node) {
        list->head = node->next;
    }
    ListFreeNode(list, node);
    list->count--;
}
