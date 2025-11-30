#include <iostream>
using namespace std;

//creating a queue using linked list to my project
template<typename T>
class MyQueue {
    private:
        struct Node {
            T data;
            Node* next;
            Node(T val) {
                data = val;
                next = nullptr;
            }
        };
        Node* frontNode;
        Node* rearNode;
    public:
        MyQueue() {
            frontNode = nullptr;
            rearNode = nullptr;
        }
        ~MyQueue() {
            while (!isEmpty()) {
                dequeue();
            }
        }
        bool isEmpty() const {
            return frontNode == nullptr;
        }
        void enqueue(const T& val) {
            Node* newNode = new Node(val);
            if (isEmpty()) {
                frontNode = newNode;
                rearNode = newNode;
            } else {
                rearNode->next = newNode;
                rearNode = newNode;
            }
        }
        void dequeue() {
            if (isEmpty()) {
                cout << "Queue is empty. Cannot dequeue.\n";
                return;
            }
            Node* temp = frontNode;
            frontNode = frontNode->next;
            if (frontNode == nullptr) {
                rearNode = nullptr;
            }
            delete temp;
        }
        T& front() {
            if (isEmpty()) {
                throw runtime_error("Queue is empty. No front element.");
            }
            return frontNode->data;
        }
        size_t size() const {
            size_t count = 0;
            Node* current = frontNode;
            while (current != nullptr) {
                count++;
                current = current->next;
            }
            return count;
        }
        T& back() {
            if (isEmpty()) {
                throw runtime_error("Queue is empty. No back element.");
            }
            return rearNode->data;
        }
};