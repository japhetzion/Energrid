#ifndef CONTAINERS_H
#define CONTAINERS_H
#include <stdexcept>
#include <cstddef>
using namespace std;
template <typename T>
class Queue {
    struct Node {
        T data;
        Node* next;
    };
    Node* front;
    Node* rear;
    int count;
    Queue(const Queue&);
    Queue& operator=(const Queue&);

public:
    Queue() : front(NULL), rear(NULL), count(0) {}
    ~Queue() { clear(); }

    bool isEmpty() const { return front == NULL; }
    int size() const { return count; }
    void enqueue(const T& item) {
        Node* node = new Node;
        node->data = item;
        node->next = NULL;
        if (rear == NULL)
            front = rear = node;
        else {
            rear->next = node;
            rear = node;
        }
        count++;
    }
    T dequeue() {
        if (front == NULL)
            throw runtime_error("The queue is empty.");
        Node* temp = front;
        T item = temp->data;
        front = front->next;
        if (front == NULL)
            rear = NULL;
        delete temp;
        count--;
        return item;
    }

    void clear() {
        while (!isEmpty())
            dequeue();
    }
    T at(int index) const {
        if (index < 0 || index >= count)
            throw out_of_range("No item at that position.");
        Node* cur = front;
        for (int i = 0; i < index; i++)
            cur = cur->next;
        return cur->data;
    }
};
template <typename T>
class Stack {
    struct Node {
        T data;
        Node* next;
    };
    Node* topNode;
    int count;

    Stack(const Stack&);
    Stack& operator=(const Stack&);

public:
    Stack() : topNode(NULL), count(0) {}
    ~Stack() {
        while (!isEmpty())
            pop();
    }

    bool isEmpty() const { return topNode == NULL; }
    int size() const { return count; }
    void push(const T& item) {
        Node* node = new Node;
        node->data = item;
        node->next = topNode;
        topNode = node;
        count++;
    }
    T pop() {
        if (topNode == NULL)
            throw runtime_error("The stack is empty.");
        Node* temp = topNode;
        T item = temp->data;
        topNode = topNode->next;
        delete temp;
        count--;
        return item;
    }
    int toArray(T out[], int maxCount) const {
        int n = 0;
        for (Node* cur = topNode; cur != NULL && n < maxCount; cur = cur->next)
            out[n++] = cur->data;
        return n;
    }
};

#endif
