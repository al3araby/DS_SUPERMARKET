// customer.h
#pragma once
#include <string>
#include "cart.h"
using namespace std;

class SupermarketSystem; // forward

class Customer {
protected:
    string id;
    string name;
public:
    ShoppingCart cart;
    Customer() = default;
    Customer(string i, string n){
        id = move(i);
        name = move(n);
    }
    virtual ~Customer() = default;

    virtual string get_type() const {
         return "WalkIn"; 
    }
    string get_id() const {
         return id; 
        }
    string get_name() const { return name; }

    virtual void checkout(SupermarketSystem& sys, int cashierIndex);

    virtual void print_info() const;
};

class OnlineCustomer : public Customer {
private:
    string address;
    string paymentMethod;
    int priority = 5;
public:
    OnlineCustomer() = default;
    OnlineCustomer(string i, string n, string addr, string pay, int pr = 5) {
        id=move(i);
        name=move(n);
        address=move(addr);
        paymentMethod=move(pay);
        priority = pr;
    }

    string get_type() const override { return "Online"; }
    int get_priority() const { return priority; }
    void set_priority(int p) { priority = p; }
    void print_info() const override;
    void checkout(SupermarketSystem& sys, int cashierIndex) override;
};

//add new class for spicialcruchturs customers if needed 
class SpecialCustomer : public Customer {
private:
    double discountRate = 0.10; // 10% discount        
public:
    SpecialCustomer(string i, string n) {
        id=move(i);
        name=move(n);
    }

    string get_type() const override { return "Special"; }
    double get_discount_rate() const { return discountRate; } 
    void print_info() const override ;
    void checkout(SupermarketSystem& sys, int cashierIndex) override ;
};