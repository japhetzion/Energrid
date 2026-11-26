#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H
#include <stdexcept>
using namespace std;
template <typename T, int CAPACITY>
class PriorityQueue {
    T items[CAPACITY + 1];
    int count;

public:
    PriorityQueue() : count(0) {}

    bool isEmpty() const { return count == 0; }
    int size() const { return count; }
    void push(const T& item) {
        if (count >= CAPACITY)
            throw runtime_error("The priority queue is full.");
        count++;
        int i = count;
        items[i] = item;
        while (i > 1 && items[i] > items[i / 2]) {
            T temp = items[i];
            items[i] = items[i / 2];
            items[i / 2] = temp;
            i = i / 2;
        }
    }
    const T& top() const {
        if (isEmpty())
            throw runtime_error("The priority queue is empty.");
        return items[1];
    }
    T pop() {
        if (isEmpty())
            throw runtime_error("The priority queue is empty.");
        T best = items[1];
        items[1] = items[count];
        count--;
        int i = 1;
        while (true) {
            int biggest = i;
            int left = 2 * i;
            int right = 2 * i + 1;
            if (left <= count && items[left] > items[biggest])
                biggest = left;
            if (right <= count && items[right] > items[biggest])
                biggest = right;
            if (biggest == i)
                break;
            T temp = items[i];
            items[i] = items[biggest];
            items[biggest] = temp;
            i = biggest;
        }
        return best;
    }
    int toSortedArray(T out[], int maxCount) const {
        PriorityQueue<T, CAPACITY> copy = *this;
        int n = 0;
        while (!copy.isEmpty() && n < maxCount)
            out[n++] = copy.pop();
        return n;
    }
};

#endif
