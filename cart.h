// cart.h
#pragma once
#include <string>
#include "stack.h"
#include <iostream>
#include <vector>
#include "product.h"
#include "inventory.h"
using namespace std;

struct CartItem {
    string barcode;
    string name;
    string category;
    double unitPrice;
    int qty;
    CartItem* next;
    CartItem() {
        barcode = "";
        name = "";
        category = "";
        unitPrice = 0.0;
        qty = 0;
        next = nullptr;
    }
    CartItem(const string& b, const string& n, const string& c, double up, int q){
        barcode = b;
        name = n;
        category = c;
        unitPrice = up;
        qty = q;
        next = nullptr;
    }
};

enum class CartActionType { ADD, REMOVE };

struct CartAction {
    CartActionType type;
    string barcode;
    int qty;
    CartAction() : type(CartActionType::ADD), barcode(""), qty(0) {}
    CartAction(CartActionType t, const string& b, int q) {
        type = t;
        barcode = b;
        qty = q;
    }
};

struct Coupon {
    string code;
    double discountRate; // e.g., 0.10 for 10%
    Coupon() {
        code="";
        discountRate=0.0;
    }
    Coupon(const string& c, double dr) {
        code = c;
        discountRate = dr;
    }
};

class ShoppingCart {
private:
    CartItem* head = nullptr;
    MyStack<CartAction> actions;

    void add_item_noaction_internal(const Product& p, int qty) {
        if (qty <= 0){
            cout << "Quantity must be positive.\n";
            return;
        }
        CartItem* node = find_node(p.barcode);
        if (node) node->qty += qty;
        else {
            CartItem* n = new CartItem(p.barcode, p.name, p.category, p.price, qty);
            n->next = head; head = n;
        }
    }

    int remove_item_noaction_internal(const string& barcode, int qty) {
        if (qty <= 0){
            cout << "Quantity must be positive.\n";
            return 0;
        }
        CartItem* prev = nullptr;
        CartItem* cur = head;
        while (cur != nullptr) {
            if (cur->barcode == barcode) {
                if (qty >= cur->qty) {
                    int removed = cur->qty;
                    if (prev != nullptr) {
                        prev->next = cur->next;
                    }
                    else {
                        head = cur->next;
                    }
                    delete cur;
                    return removed;
                } 
                else {
                    cur->qty -= qty;
                    return qty;
                }
            }
            prev = cur; cur = cur->next;
        }
        return 0;
    }

    vector<Coupon> coupons = {
        {"LOVEEGYPT", 10.0},
        {"SAVE5", 5.0},
        {"OFFER20", 20.0},
        {"BLACKFRIDAY", 15.0}
    };
    Coupon appliedCoupon;
    bool couponApplied = false;

public:
    ShoppingCart() = default;
    ~ShoppingCart() { clear(); }

    void clear() {
        while (head != nullptr) { 
            CartItem* t = head; 
            head = head->next; 
            delete t; 
        }
        while (!actions.isEmpty()) actions.pop();
    }

    CartItem* find_node(const string& barcode) {
        CartItem* cur = head;
        while (cur != nullptr) {
            if (cur->barcode == barcode){
                return cur; 
            }
            else { 
             cur = cur->next;  
            }
        }
        return nullptr;
    }

    void add_item(const Product& p, int qty) {
        if (qty <= 0){
            cout << "Quantity must be positive.\n";
            return;
        }
        add_item_noaction_internal(p, qty);
        actions.push(CartAction(CartActionType::ADD, p.barcode, qty));
    }

    // removes up to qty items; returns number of items actually removed (0 if none)
    int remove_item(const string& barcode, int qty) {
        if (qty <= 0){
            cout << "Quantity must be positive.\n";
            return 0;
        }
        int removed = remove_item_noaction_internal(barcode, qty);
        if (removed == 0) return 0;
        actions.push(CartAction(CartActionType::REMOVE, barcode, removed));
        return removed;
    }

    bool apply_coupon(const string& code) {
        if (couponApplied) {
            cout << "A coupon has already been applied.\n";
            return false;
        }
        for (const auto& c : coupons) {
            if (c.code == code) {
                appliedCoupon = c;
                couponApplied = true;
                return true;
            }
        }
        cout << "Invalid coupon code.\n";
        return false;
    }

    double total() const {
        double t = 0;
        CartItem* cur = head;
        while (cur != nullptr) {
             t += cur->unitPrice * cur->qty;
            cur = cur->next; 
        } 
        if(couponApplied){
            t = t * (1 - appliedCoupon.discountRate / 100.0);
        }
        return t;
    }

    bool empty() const {
         if (head == nullptr){
             return true;
         }
         else {
             return false;
         }
    }

    void print_cart() {
        cout << "Cart contents:\n";
        CartItem* cur = head;
        while (cur!= nullptr) {
            cout << cur->barcode << " | " << cur->name << " | " << cur->category << " | qty: " << cur->qty
                 << " | unit: LE" << cur->unitPrice << '\n';
            cur = cur->next;
        }
        cout << "Total before discount: LE " << total() << '\n';
    }

    // The undo functions require Inventory type; forward declare or define in caller.
    friend class Inventory; // allow Inventory access in undo if needed by design

    // Undo the last cart action and update inventory accordingly.
    // Returns pair(success, message).
    pair<bool,string> undo(Inventory& inv) {
        if (actions.isEmpty()) { 
            return {false, "No actions to undo"};
        } 
        CartAction act = actions.top(); actions.pop();
        if (act.type == CartActionType::ADD) {
            // Undo adding to cart: remove from cart, restore inventory
            int removed = remove_item_noaction_internal(act.barcode, act.qty);
            if (removed <= 0){
                return {false, "Nothing to remove from cart"};
            }
            inv.update_stock(act.barcode, removed); // restore stock
            return {true, "Undid add: restored " + to_string(removed) + " items to inventory"};
        } 
        else {
            // Undo removing from cart: add back to cart, decrease inventory
            Product* p = inv.find(act.barcode);
            if (p == nullptr) return {false, "Product not found in inventory to restore into cart"};
            add_item_noaction_internal(*p, act.qty);
            inv.update_stock(act.barcode, -act.qty);
            return {true, "Undid remove: returned " + to_string(act.qty) + " items to cart"};
        }
    }
};
