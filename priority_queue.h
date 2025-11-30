#include <iostream>
using namespace std;

//make a priority queue using linked list for my project
template<typename T, typename Compare>
class MyPriorityQueue {
    private:
        struct Node {
            T data;
            Node* next;
            
            Node(T val) {
                data = val;
                next = nullptr;
            }
        };
        Node* head;
        Compare comp;
    public:
        MyPriorityQueue() {
            head = nullptr;
        }
        ~MyPriorityQueue() {
            while (!isEmpty()) {
                pop();
            }
        }
        bool isEmpty() const {
            return head == nullptr;
        }
        void push(const T& val) {
            Node* newNode = new Node(val);
            if (isEmpty() || comp(val, head->data)) {
                newNode->next = head;
                head = newNode;
            } else {
                Node* current = head;
                while (current->next != nullptr && !comp(val, current->next->data)) {
                    current = current->next;
                }
                newNode->next = current->next;
                current->next = newNode;
            }
        }
        void pop() {
            if (isEmpty()) {
                cout << "Priority Queue is empty. Cannot pop.\n";
                return;
            }
            Node* temp = head;
            head = head->next;
            delete temp;
        }
        T& top() {
            if (isEmpty()) {
                throw runtime_error("Priority Queue is empty. No top element.");
            }
            return head->data;
        }
        size_t size() const {
            size_t count = 0;
            Node* current = head;
            while (current != nullptr) {
                count++;
                current = current->next;
            }
            return count;
        }
};