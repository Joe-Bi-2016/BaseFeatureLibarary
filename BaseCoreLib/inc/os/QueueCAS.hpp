/*****************************************************************************
* FileName    : LockFreeQueue.hpp
* Description : Lock free queues definition, implemented in c++11
* Author      : Joe.Bi
* Date        : 2024-04
* Version     : v1.0
* Copyright (c)  xxx . All rights reserved.
******************************************************************************/
#ifndef __LockFreeQueue_h__
#define __LockFreeQueue_h__
#include <atomic>
#include <iostream>

template<typename ElemType>
class LockFreeQueue {
public:
    LockFreeQueue(void);
    ~LockFreeQueue(void);

    void push(ElemType elem);
    ElemType pop(void);
    void dump(void);

private:
    typedef struct qNode {
        qNode(void) : next(nullptr) { }
        qNode(ElemType elem) : elem(elem), next(nullptr) { }
        ElemType       elem;
        std::atomic<struct qNode*> next;
    }Node;
    
private:
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
};

template<typename ElemType>
LockFreeQueue<ElemType>::LockFreeQueue(void) {
    head = tail = new Node();
}

template<typename ElemType>
LockFreeQueue<ElemType>::~LockFreeQueue(void) {
    while (head != nullptr)
    {
        Node* tempNode = head;
        head = head->next;
        delete tempNode;
    }
}

template<typename ElemType>
void LockFreeQueue<ElemType>::push(ElemType elem) {
    Node* newNode = new Node(elem);
    Node* oldtail;
    Node* next;

    do {
        oldtail = tail.load();
        next = oldtail->next.load();
        if (oldtail != tail.load())
            continue;

        if (next != nullptr) {
            atomic_compare_exchange_weak(&tail, (std::atomic<Node*>*) (&oldtail), next);
            continue;
        }
    } while (atomic_compare_exchange_weak(&(oldtail->next), (std::atomic<Node*>*) (&next), newNode) != true);

    atomic_compare_exchange_weak((std::atomic<Node*>*)(&tail), (Node**)(&oldtail), newNode);
}

template<typename ElemType>
ElemType LockFreeQueue<ElemType>::pop(void) {
    Node* oldHead;
    Node* next;
    do {
        oldHead = head.load();
        Node* oldTail = tail.load();
        next = oldHead->next.load();
        if (oldHead != head.load())
            continue;

        if (oldHead == oldTail && next == nullptr)
            return  ElemType(0);

    } while (atomic_compare_exchange_weak(&head, (std::atomic<Node*>*)(&oldHead), next) != true);

    ElemType val = oldHead->elem;
    oldHead->next = nullptr;
    delete oldHead;

    return val; 
}

template<typename ElemType>
void LockFreeQueue<ElemType>::dump(void) {
    Node* tempNode = head->next;

    if (tempNode == nullptr) {
        std::cout << "Empty" << std::endl;
        return;
    }

    while (tempNode != nullptr)
    {
        std::cout << tempNode->elem << " ";
        tempNode = tempNode->next;
    }
    std::cout << std::endl;
}

#endif // __LockFreeQueue_h__
