#include <iostream>
using namespace std;

template<typename T>
class MyStack {
    private:
        struct Node {
            T data;
            Node* next;
            Node(T val) {
                data = val;
                next = nullptr;
            }
        };
        Node* topNode;
    public:
        MyStack() {
            topNode = nullptr;
        }
        ~MyStack() {
            while (!isEmpty()) {
                pop();
            }
        }
        bool isEmpty() const {
            if (topNode == nullptr){
                return true;
            }
            else {
                return false;
            }
        }
        void push(const T& val) {
            Node* newNode = new Node(val);
            newNode->next = topNode;
            topNode = newNode;
         }
        
        void pop() {
            if (isEmpty()){
                cout << "Stack is empty. Cannot pop.\n";
                return;
            }
            Node* temp = topNode;
            topNode = topNode->next;
            delete temp;
        }
        T& top() {
            if (isEmpty()){
                throw runtime_error("Stack is empty. No top element.");
            }
            return topNode->data;
        }
        size_t size() const {
            size_t count = 0;
            Node* current = topNode;
            while (current != nullptr) {
                count++;
                current = current->next;
            }
            return count;
        }
};
