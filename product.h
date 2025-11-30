// product.h
#pragma once
#include <string>
#include <utility>
using namespace std;
//adding product category
struct Product {
    string barcode;
    string name;
    double price;
    int stock;
    string category;
    string expiry; // "YYYY-MM-DD"

    Product(){
        barcode = "";
        name = "";
        price = 0.0;
        stock = 0;
        expiry = "";
        category = "";
    }
    Product(string b, string n, double p, int s, string e, string c=""){
        barcode = move(b);
        name = move(n);
        price = p;
        stock = s;
        expiry = move(e);
        category = move(c);
    }
};
