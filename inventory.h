// inventory.h
#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include "product.h"
using namespace std;

class Inventory {
private:
    unordered_map<string, Product> table; // barcode : product
public:
    bool add_product(const Product& p) {
        if (table.count(p.barcode)) {
            return false;
        }
        else{
            table[p.barcode] = p;
        return true;
        }
    }

    Product* find(const string& barcode) {
        auto it = table.find(barcode);
        if (it == table.end()){
            return nullptr;
        } 
        else{
            return &(it->second);
        }
    }

    bool update_stock(const string& barcode, int delta) {
        auto p = find(barcode);
        if (!p){
            return false;
        }
        else{
            p->stock += delta;
            if (p->stock < 0) {
                p->stock -= delta; 
                return false;
            }
            else{
                return true;
            }
        }  
    }

    vector<Product> all_products() const {
        vector<Product> v; v.reserve(table.size());
        for(auto it = table.begin(); it != table.end(); ++it)
         {
            v.push_back(it->second);
         }
        return v;
    }

    void print_all() const {
        cout << "Inventory:\n";
        for (auto &kv : table) {
            const Product &p = kv.second;
            cout << p.barcode << " | " << p.name << " | " << p.price << " LE "
                 << " | stock: " << p.stock << " | expiry: " << p.expiry << " | " << p.category << '\n';
        }
    }
};
