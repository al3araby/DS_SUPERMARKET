// bst.h
#pragma once
#include <vector>
#include "product.h"
using namespace std;

struct BSTNode {
    Product p;
    BSTNode* left;
    BSTNode* right;
    BSTNode(const Product& prod){
        p = prod;
        left = nullptr;
        right = nullptr;
    }
};

class ProductBST {
private:
    BSTNode* root = nullptr;
    void free_tree(BSTNode* n) {
        if (n == nullptr) return;
        free_tree(n->left);
        free_tree(n->right);
        delete n;
    }

    BSTNode* insert_node(BSTNode* node, const Product& p) {
        if (node == nullptr){
           BSTNode* newNode = new BSTNode(p);
           return newNode; 
        } 

        if (p.price < node->p.price) {
            node->left = insert_node(node->left, p);
        }
        else {
            node->right = insert_node(node->right, p);
        }
        return node;
    }

    void inorder(BSTNode* node, vector<Product>& out) const {
        if (node == nullptr) return;
        inorder(node->left, out);
        out.push_back(node->p);
        inorder(node->right, out);
    }
public:
    ProductBST() = default;
    ~ProductBST() { free_tree(root); }

    void build(const vector<Product>& items) {
        free_tree(root);
        root = nullptr;
        for(int i=0;i<items.size();++i){
            root = insert_node(root, items[i]);
        }
    }

    vector<Product> sorted_by_price() const {
        vector<Product> out;
        inorder(root, out);
        return out;
    }
};

class ProductBSTByCategory {
    private:
        BSTNode* root = nullptr;
        void free_tree(BSTNode* n) {
            if (n == nullptr) return;
            free_tree(n->left);
            free_tree(n->right);
            delete n;
        }
        BSTNode* insert_node(BSTNode* node, const Product& p) {
            if (node == nullptr){
               BSTNode* newNode = new BSTNode(p);
               return newNode; 
            } 

            if (p.category < node->p.category) {
                node->left = insert_node(node->left, p);
            }
            else {
                node->right = insert_node(node->right, p);
            }
            return node;
        }
        void inorder(BSTNode* node, vector<Product>& out) const {
            if (node == nullptr) return;
            inorder(node->left, out);
            out.push_back(node->p);
            inorder(node->right, out);
        }
    public:
        ProductBSTByCategory() = default;
        ~ProductBSTByCategory() { free_tree(root); }
        void build(const vector<Product>& items) {
            free_tree(root);
            root = nullptr;
            for(int i=0;i<items.size();++i){
                root = insert_node(root, items[i]);
            }
        }
        vector<Product> sorted_by_category() const {
            vector<Product> out;
            inorder(root, out);
            return out;
        }
};