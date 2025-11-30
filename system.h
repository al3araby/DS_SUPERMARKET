// system.h
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "queue.h"
#include "priority_queue.h"
#include <unordered_map>
#include <limits>
#include <string>
#include "inventory.h"
#include "bst.h"
#include "sales.h"
#include "customer.h"
using namespace std;

struct Cashier {
    string id;
    MyQueue<Customer*> q;
    MyQueue<SpecialCustomer*> specialNeedsQueue;
    MyStack<SaleRecord*> undoStack;
    Cashier() = default;
    Cashier(const string& i){
        id = i;
    }
};

struct OnlineOrder {
    OnlineCustomer* customer;
    OnlineOrder() { 
        customer = nullptr;
        placed_time = 0; 
        priority = 0;
    }
    time_t placed_time;
    int priority; // lower = higher priority
    OnlineOrder(OnlineCustomer* c, int p){
        customer = c;
        placed_time = time(nullptr);
        priority = p;
    }
};

struct OnlineOrderCompare {
    bool operator()(const OnlineOrder& a, const OnlineOrder& b) const {
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.placed_time > b.placed_time;
    }
};

class SupermarketSystem {
private:
    Inventory inventory;
    ProductBST bst;
    ProductBSTByCategory bstbycategory;
    SalesList sales;
    vector<unique_ptr<Cashier>> cashiers;
    unique_ptr<Cashier> specialNeedsCashier;
    unordered_map<string, unique_ptr<Customer>> customers;
    MyPriorityQueue<OnlineOrder, OnlineOrderCompare> onlineQueue;


    int nextSale = 1;

public:
    SupermarketSystem(int cashierCount = 3) {
        for (int i = 0; i < cashierCount; ++i){
            cashiers.push_back(make_unique<Cashier>("CASH" + to_string(i+1)));
        }
        specialNeedsCashier = make_unique<Cashier>("SPECIAL");
        seed_data();
    }

    void seed_data();
    void rebuild_bst();

    bool add_walkin_customer(const string& id, const string& name);
    bool add_online_customer(const string& id, const string& name, const string& addr, const string& pay, int priority = 5);
    Customer* get_customer(const string& id);
    void list_customers() const;

    bool customer_add_to_cart(const string& custId, const string& barcode, int qty);
    pair<bool,string> customer_undo(const string& custId);

    void show_customer_cart(const string& custId);

    void enqueue_walkin_to_cashier(const string& custId);
    void enqueue_specialneeds_to_cashier(SpecialCustomer* sc);
    void place_online_order(const string& custId);

    void process_checkout_at_cashier(int cashierIndex);
    void process_checkout_at_specialneedscashier();
    void process_next_online_order();
    void cashier_undo_last_bill(int cashierIndex);
    void cashier_undo_last_specialneedscashier_bill();
    

    void print_inventory() const;
    void print_products_sorted_price();
    void print_products_sorted_category();
    void print_sales_report();
    void print_cashiers_status() const;

    Inventory& get_inventory() { return inventory; }

    void interactive_console();
};

// Implementations (kept in header for simplicity)
inline void SupermarketSystem::seed_data() {
    //diary products
    inventory.add_product(Product("0001","Milk 1L",45.0,50,"2025-12-01","Dairy"));
    inventory.add_product(Product("0008","Cheese 200g",125.0,35,"2026-02-01","Dairy"));
    inventory.add_product(Product("0012","Butter 250g",170.5,30,"2026-04-01","Dairy"));
    inventory.add_product(Product("0009","Yogurt 150g",45.5,80,"2026-03-01","Dairy"));
    inventory.add_product(Product("0014","Cream 200ml",70.99,40,"2026-05-01","Dairy"));
    inventory.add_product(Product("0028","Ice Cream 500ml",172.5,25,"2026-06-01","Dairy"));
    //meet products
    inventory.add_product(Product("0006","Chicken Breast 1kg",185.0,25,"2025-12-15","Meat"));
    inventory.add_product(Product("0015","Ground Beef 500g",250.0,20,"2025-12-10","Meat"));
    inventory.add_product(Product("0016","tuna 1kg",285.0,15,"2025-12-20","Meat"));
    inventory.add_product(Product("0024","Salmon Fillet 500g",490.0,10,"2025-12-18","Meat"));
    inventory.add_product(Product("0025","Turkey Slices 300g",380.0,18,"2025-12-22","Meat"));
    inventory.add_product(Product("0026","Sausages 500g",230.0,22,"2025-12-25","Meat"));
    inventory.add_product(Product("0027","Lamb Chops 500g",280.0,8,"2025-12-30","Meat"));
    //school products
    inventory.add_product(Product("0017","Notebook A4 120pg",60.9,100,"2027-12-31","School"));
    inventory.add_product(Product("0018","Pen Blue Ink",10.0,200,"2027-12-31","School"));
    inventory.add_product(Product("0019","Eraser",5.0,150,"2027-12-31","School"));
    inventory.add_product(Product("0020","Ruler 30cm",25.0,80,"2027-12-31","School"));
    inventory.add_product(Product("0021","Backpack",250.0,40,"2027-12-31","School"));
    inventory.add_product(Product("0022","Calculator",1500.0,60,"2027-12-31","School"));
    inventory.add_product(Product("0023","Highlighter Set",30.5,70,"2027-12-31","School"));
    //produce products
    inventory.add_product(Product("0004","Apple 1kg",85.0,20,"2025-11-30","Produce"));
    inventory.add_product(Product("0012","Lettuce",15.0,50,"2025-11-28","Produce"));
    inventory.add_product(Product("0019","Carrots 1kg",15.0,60,"2025-12-05","Produce"));
    inventory.add_product(Product("0020","Potatoes 2kg",25.0,70,"2025-12-10","Produce"));
    inventory.add_product(Product("0029","Grapes 500g",28.0,40,"2025-11-29","Produce"));
    inventory.add_product(Product("0030","Strawberries 250g",32.0,30,"2025-11-27","Produce"));
    inventory.add_product(Product("0031","Cucumbers",12.0,55,"2025-12-03","Produce"));
    inventory.add_product(Product("0032","Bell Peppers 1kg",40.0,45,"2025-12-07","Produce"));
    //cleaning products
    inventory.add_product(Product("0033","Dish Soap 500ml",85.5,80,"2027-12-31","Cleaning"));
    inventory.add_product(Product("0034","Laundry Detergent 1L",50.0,60,"2027-12-31","Cleaning"));
    inventory.add_product(Product("0035","All-Purpose Cleaner 750ml",35.0,70,"2027-12-31","Cleaning"));
    inventory.add_product(Product("0036","Sponges 5pc",18.0,90,"2027-12-31","Cleaning"));
    inventory.add_product(Product("0037","Paper Towels 2rolls",22.0,50,"2027-12-31","Cleaning"));
    inventory.add_product(Product("0038","Trash Bags 30pc",40.0,40,"2027-12-31","Cleaning"));
    //beverages products
    inventory.add_product(Product("0039","Coffee 250g",250.0,30,"2026-12-31","Beverages"));
    inventory.add_product(Product("0040","Tea Bags 100pc",30.0,50,"2026-12-31","Beverages"));
    inventory.add_product(Product("0041","Soda 330ml",15.0,70,"2025-12-31","Beverages"));
    inventory.add_product(Product("0042","Bottled Water 500ml",8.0,100,"2025-12-31","Beverages"));
    inventory.add_product(Product("0043","Energy Drink 250ml",20.0,40,"2025-12-31","Beverages"));
    //snack products
    inventory.add_product(Product("0044","Chips 200g",25.0,60,"2026-06-30","Snacks"));
    inventory.add_product(Product("0045","Chocolate Bar 100g",30.0,80,"2026-05-31","Snacks"));
    inventory.add_product(Product("0046","Cookies 150g",20.0,70,"2026-07-15","Snacks"));
    inventory.add_product(Product("0047","Nuts chocolate Mix 250g",40.0,50,"2026-08-31","Snacks"));
    inventory.add_product(Product("0048","Granola Bars 6pc",50.0,90,"2026-09-30","Snacks"));
    inventory.add_product(Product("0049","Popcorn 100g",15.0,100,"2026-04-30","Snacks"));
    inventory.add_product(Product("0050","Dried Fruit 200g",75.0,40,"2026-10-31","Snacks"));
    inventory.add_product(Product("0051","Pretzels 150g",18.0,75,"2026-11-30","Snacks"));
    //self care products
    inventory.add_product(Product("0052","Shampoo 500ml",200.5,60,"2027-12-31","Self Care"));
    inventory.add_product(Product("0053","Conditioner 400ml",120.0,55,"2027-12-31","Self Care"));
    inventory.add_product(Product("0054","Body Wash 500ml",80.0,70,"2027-12-31","Self Care"));
    inventory.add_product(Product("0055","Toothpaste 150g",45.0,80,"2027-12-31","Self Care"));
    inventory.add_product(Product("0056","Deodorant 200ml",80.0,50,"2027-12-31","Self Care"));
    //household products
    inventory.add_product(Product("0057","Toilet Paper 12rolls",50.0,40,"2027-12-31","Household"));
    inventory.add_product(Product("0058","Facial Tissues 4packs",30.0,70,"2027-12-31","Household"));
    inventory.add_product(Product("0059","Hand Soap 300ml",30.0,90,"2027-12-31","Household"));
    inventory.add_product(Product("0060","Air Freshener 250ml",40.0,50,"2027-12-31","Household"));
    inventory.add_product(Product("0061","Light Bulbs 2pc",35.0,60,"2027-12-31","Household"));
    inventory.add_product(Product("0062","Batteries AA 4pc",40.0,80,"2027-12-31","Household"));
    inventory.add_product(Product("0063","Extension Cord 3m",7.0,30,"2027-12-31","Household"));
    //food staples products
    inventory.add_product(Product("0064","Pasta 500g",40.5,70,"2027-12-31","Food Staples"));
    inventory.add_product(Product("0065","Canned Beans 400g",26.0,80,"2027-12-31","Food Staples"));
    inventory.add_product(Product("0066","Canned Tuna 200g",50.0,60,"2027-12-31","Food Staples"));
    inventory.add_product(Product("0067","Olive Oil 1L",80.0,40,"2027-12-31","Food Staples"));
    inventory.add_product(Product("0068","Flour 1kg",25.0,50,"2027-12-31","Food Staples"));
    inventory.add_product(Product("0069","Sugar 1kg",20.0,60,"2027-12-31","Food Staples"));
    inventory.add_product(Product("0070","Salt 500g",10.0,90,"2027-12-31","Food Staples"));
    inventory.add_product(Product("0071","Baking Powder 200g",18.0,70,"2027-12-31","Food Staples"));
    inventory.add_product(Product("0072","Yeast 100g",15.0,80,"2027-12-31","Food Staples"));
    //toys products
    inventory.add_product(Product("0073","Action Figure",400.0,30,"2028-12-31","Toys"));
    inventory.add_product(Product("0074","Doll",380.0,25,"2028-12-31","Toys"));
    inventory.add_product(Product("0075","Puzzle 500pc",250.0,40,"2028-12-31","Toys"));
    inventory.add_product(Product("0076","Board Game",150.0,20,"2028-12-31","Toys"));
    inventory.add_product(Product("0077","Remote Control Car",250.0,15,"2028-12-31","Toys"));
    inventory.add_product(Product("0101","LEGO Building Blocks Set",300.0,18,"2028-12-31","Toys"));
    inventory.add_product(Product("0102","Stuffed Animal",120.0,50,"2028-12-31","Toys"));
    //electronics products
    inventory.add_product(Product("0078","Headphones",600.0,20,"2028-12-31","Electronics"));
    inventory.add_product(Product("0079","Portable Charger 65W",400.0,25,"2028-12-31","Electronics"));
    inventory.add_product(Product("0080","USB Flash Drive 128GB",600.0,30,"2028-12-31","Electronics"));
    inventory.add_product(Product("0081","Wireless Mouse",380.0,40,"2028-12-31","Electronics"));
    inventory.add_product(Product("0082","Keyboard",180.0,35,"2028-12-31","Electronics"));
    inventory.add_product(Product("0083","Webcam",400.0,15,"2028-12-31","Electronics"));
    inventory.add_product(Product("0084","Bluetooth Speaker",350.0,20,"2028-12-31","Electronics"));
    inventory.add_product(Product("0085","Smartwatch",1500.0,10,"2028-12-31","Electronics"));
    inventory.add_product(Product("0086","Fitness Tracker",500.0,15,"2028-12-31","Electronics"));
    inventory.add_product(Product("0087","E-reader",380.0,8,"2028-12-31","Electronics"));
    inventory.add_product(Product("0088","Tablet",8700.0,12,"2028-12-31","Electronics"));
    inventory.add_product(Product("0089","Iphone 13 pro max",39900.0,20,"2028-12-31","Electronics"));
    inventory.add_product(Product("0090","Laptop",69999.0,10,"2028-12-31","Electronics"));
    //sports products
    inventory.add_product(Product("0091","Football",380.0,25,"2027-12-31","Sports"));
    inventory.add_product(Product("0092","Basketball",220.0,30,"2027-12-31","Sports"));
    inventory.add_product(Product("0093","Tennis Racket",500.0,15,"2027-12-31","Sports"));
    inventory.add_product(Product("0094","Yoga Mat",180.0,40,"2027-12-31","Sports"));
    inventory.add_product(Product("0095","Dumbbell Set",720.0,10,"2027-12-31","Sports"));
    inventory.add_product(Product("0096","Jump Rope",80.0,50,"2027-12-31","Sports"));
    inventory.add_product(Product("0097","Cycling Helmet",450.0,20,"2027-12-31","Sports"));
    //bakery products
    inventory.add_product(Product("0098","Bagels 6pc",30.0,40,"2025-10-05","Bakery"));
    inventory.add_product(Product("0099","Muffins",40.0,35,"2025-10-03","Bakery"));
    inventory.add_product(Product("0100","Croissants 3pc",35.0,30,"2025-10-04","Bakery"));
    inventory.add_product(Product("0002","Bread",2.0,100,"2025-10-01","Bakery"));
    //other products
    inventory.add_product(Product("0003","Eggs 12pc",8.5,30,"2026-01-01","Eggs"));
    inventory.add_product(Product("0005","Rice 1kg",40.0,40,"2027-01-01","Grains"));
    inventory.add_product(Product("0007","Orange Juice 1L",60.0,60,"2025-12-20","Beverages"));
    inventory.add_product(Product("0010","Banana 1kg",45.0,45,"2025-11-25","Produce"));
    inventory.add_product(Product("0011","Cereal 500g",65.5,55,"2026-06-01","Breakfast"));
    inventory.add_product(Product("0013","Tomato Sauce 500g",16.0,70,"2027-05-01","Condiments"));

    rebuild_bst();
}

inline void SupermarketSystem::rebuild_bst() { 
    bst.build(inventory.all_products());
    bstbycategory.build(inventory.all_products());
}

inline bool SupermarketSystem::add_walkin_customer(const string& id, const string& name) {
    if (customers.count(id)) return false;
    customers[id] = make_unique<Customer>(id, name);
    return true;
}

inline bool SupermarketSystem::add_online_customer(const string& id, const string& name, const string& addr, const string& pay, int priority) {
    if (customers.count(id)) return false;
    customers[id] = make_unique<OnlineCustomer>(id, name, addr, pay, priority);
    return true;
}

inline Customer* SupermarketSystem::get_customer(const string& id) {
    auto it = customers.find(id);
    if (it == customers.end()) return nullptr;
    return it->second.get();
}

inline void SupermarketSystem::list_customers() const {
    cout << "Customers list:\n";
    for (auto it=customers.begin(); it != customers.end(); ++it) {
        const Customer* c = it->second.get();
        cout << c->get_id() << " | " << c->get_name() << " | Type: " << c->get_type() << '\n';
    }
}

inline bool SupermarketSystem::customer_add_to_cart(const string& custId, const string& barcode, int qty) {
    Customer* c = get_customer(custId);
    if (c==nullptr) { cout << "Customer not found\n"; return false; }
    Product* p = inventory.find(barcode);
    if (p==nullptr) { cout << "Product not found\n"; return false; }
    if (p->stock < qty) { cout << "Not enough stock. Available: " << p->stock << '\n'; return false; }
    p->stock -= qty;
    c->cart.add_item(*p, qty);
    cout << "Added " << qty << " x " << p->name << " to " << c->get_name() << " cart.\n";
    return true;
}

inline pair<bool,string> SupermarketSystem::customer_undo(const string& custId) {
    Customer* c = get_customer(custId);
    if (c==nullptr) return {false,"Customer not found"};
    auto res = c->cart.undo(inventory);
    return res;
}

inline void SupermarketSystem::show_customer_cart(const string& custId){
    Customer* c = get_customer(custId);
    if (c==nullptr) { cout << "Customer not found\n"; return; }
    cout << "Cart for customer " << c->get_name() << ":\n";
    c->cart.print_cart();
}

inline void SupermarketSystem::enqueue_specialneeds_to_cashier(SpecialCustomer* sc) {
    specialNeedsCashier->specialNeedsQueue.enqueue(sc);
    cout << "Enqueued special needs customer " << sc->get_name() << " to " << specialNeedsCashier->id << '\n';
}

inline void SupermarketSystem::enqueue_walkin_to_cashier(const string& custId) {
    Customer* c = get_customer(custId);
    if (c==nullptr) { cout << "Customer not found\n"; return; }

    SpecialCustomer* sc = dynamic_cast<SpecialCustomer*>(c);
    if (sc != nullptr) {
        specialNeedsCashier->specialNeedsQueue.enqueue(sc);
        cout << "Enqueued special needs customer " << c->get_name() << " to " << specialNeedsCashier->id << '\n';
        return;
    }

    size_t idx = 0; size_t minSz = numeric_limits<size_t>::max();
    for (size_t i=0;i<cashiers.size();++i) {
        if (cashiers[i]->q.size() < minSz) { minSz = cashiers[i]->q.size(); idx = i; }
    }
    cashiers[idx]->q.enqueue(c);
    cout << "Enqueued " << c->get_name() << " to " << cashiers[idx]->id << '\n';
}

inline void SupermarketSystem::place_online_order(const string& custId) {
    Customer* c = get_customer(custId);
    if (c==nullptr) { cout << "Customer not found\n"; return; }
    OnlineCustomer* oc = dynamic_cast<OnlineCustomer*>(c);
    if (oc == nullptr) { cout << "Not an online customer\n"; return; }
    onlineQueue.push(OnlineOrder(oc, oc->get_priority()));
    cout << "Placed online order for " << oc->get_name() << " (priority " << oc->get_priority() << ")\n";
}

inline void SupermarketSystem::process_checkout_at_cashier(int cashierIndex) {
    if (cashierIndex < 0 || cashierIndex >= (int)cashiers.size()) { cout << "Invalid cashier\n"; return; }
    Cashier* cs = cashiers[cashierIndex].get();
    if (cs->q.isEmpty()) { cout << "No customers in queue\n"; return; }
    Customer* c = cs->q.front(); cs->q.dequeue();
    if (c->cart.empty()) { cout << "Customer has empty cart\n"; return; }
    string coupon;
    cout << "Apply coupon code (or press Enter to skip): ";
    getline(cin, coupon);
    if (!coupon.empty()) {
        if(c->cart.apply_coupon(coupon)){
            cout << "Coupon applied successfully! New total: LE " << c->cart.total() << '\n';
        }else{
            cout << "Invalid coupon code. No discount applied.\n";
        }
    }
    double tot = c->cart.total();
    if(tot >= 1000.0){
        cout << "Applying special discount of 5% for bills over LE 1000\n";
        tot = tot * 0.95;
        cout << "New total after special discount: LE " << tot << '\n';
    }
    vector<pair<string,int>> items; // extracting not implemented here
    string sid = "S" + to_string(nextSale++);
    SaleRecord* s = new SaleRecord(sid, c->get_id(), false, items, tot);
    c->cart.print_cart();
    sales.add_sale(s);
    cs->undoStack.push(s);
    cout << "Checked out walk-in: " << c->get_name() << " | SaleID: " << sid << " | Total: LE " << tot << '\n';
}

inline void SupermarketSystem::process_checkout_at_specialneedscashier() {
    Cashier* cs = specialNeedsCashier.get();
    if (cs->specialNeedsQueue.isEmpty()) { cout << "No special needs customers in queue\n"; return; }
    SpecialCustomer* c = cs->specialNeedsQueue.front(); cs->specialNeedsQueue.dequeue();
    if (c->cart.empty()) { cout << "Customer has empty cart\n"; return; }
    string coupon;
    cout << "Apply coupon code (or press Enter to skip): ";
    getline(cin, coupon);
    if (!coupon.empty()) {
        if(c->cart.apply_coupon(coupon)){
            cout << "Coupon applied successfully! New total: LE " << c->cart.total() << '\n';
        }else{
            cout << "Invalid coupon code. No discount applied.\n";
        }
    }
    double tot = c->cart.total();
    double specialDiscount = tot * (c->get_discount_rate());
    tot = tot - specialDiscount;
    cout << "Applied special customer discount (10%): LE " << specialDiscount << '\n';
    
    if(tot >= 1000.0){
        cout << "Applying special discount of 5% for bills over LE 1000\n";
        tot = tot * 0.95;
        cout << "New total after special discount: LE " << tot << '\n';
    }
    vector<pair<string,int>> items; // extracting not implemented here
    string sid = "S" + to_string(nextSale++);
    SaleRecord* s = new SaleRecord(sid, c->get_id(), false, items, tot);
    c->cart.print_cart();
    sales.add_sale(s);
    cs->undoStack.push(s);
    cout << "Checked out special needs customer: " << c->get_name() << " | SaleID: " << sid << " | Total: LE " << tot << '\n';
}

inline void SupermarketSystem::cashier_undo_last_bill(int cashierIndex) {
    if (cashierIndex < 0 || cashierIndex >= (int)cashiers.size()) { cout << "Invalid cashier\n"; return; }
    Cashier* cs = cashiers[cashierIndex].get();
    if (cs->undoStack.isEmpty()) { cout << "No bills to undo\n"; return; }
    SaleRecord* s = cs->undoStack.top(); cs->undoStack.pop();
    string sid = s->saleId; // copy id before removal (avoid use-after-free)
    for (auto &it : s->items) inventory.update_stock(it.first, it.second);
    bool removed = sales.remove_by_id(sid);
    if (removed==false) cout << "Warning: sale not found in sales list for removal. (shouldn't happen)\n";
    cout << "Undid sale " << sid << " and restored stock\n";
}

inline void SupermarketSystem::cashier_undo_last_specialneedscashier_bill() {
    Cashier* cs = specialNeedsCashier.get();
    if (cs->undoStack.isEmpty()) { cout << "No bills to undo\n"; return; }
    SaleRecord* s = cs->undoStack.top(); cs->undoStack.pop();
    string sid = s->saleId; // copy id before removal (avoid use-after-free)
    for (auto &it : s->items) inventory.update_stock(it.first, it.second);
    bool removed = sales.remove_by_id(sid);
    if (removed==false) cout << "Warning: sale not found in sales list for removal. (shouldn't happen)\n";
    cout << "Undid sale " << sid << " and restored stock\n";
}

inline void SupermarketSystem::process_next_online_order() {
    if (onlineQueue.isEmpty()) { cout << "No online orders\n"; return; }
    OnlineOrder ord = onlineQueue.top(); onlineQueue.pop();
    OnlineCustomer* oc = ord.customer;
    if (oc->cart.empty()) { cout << "Online customer has empty cart\n"; return; }
    string coupon;
    cout << "Apply coupon code (or press Enter to skip): ";
    getline(cin, coupon);
    if (!coupon.empty()) {
        if(oc->cart.apply_coupon(coupon)){
            cout << "Coupon applied successfully! New total: LE " << oc->cart.total() << '\n';
        }else{
            cout << "Invalid coupon code. No discount applied.\n";
        }
    }
    double tot = oc->cart.total();
    if(tot >= 1000.0){
        cout << "Applying special discount of 5% for bills over LE 1000\n";
        tot = tot * 0.95;
        cout << "New total after special discount: LE " << tot << '\n';
    }
    vector<pair<string,int>> items; // extracting not implemented here
    string sid = "S" + to_string(nextSale++);
    SaleRecord* s = new SaleRecord(sid, oc->get_id(), true, items, tot);
    sales.add_sale(s);
    oc->cart.print_cart();
    if (!cashiers.empty()) cashiers[0]->undoStack.push(s);
    cout << "Processed online order: " << oc->get_name() << " | SaleID: " << sid << " | Total: LE " << tot << '\n';
}

inline void SupermarketSystem::print_inventory() const { inventory.print_all(); }

inline void SupermarketSystem::print_products_sorted_price() {
    const_cast<SupermarketSystem*>(this)->rebuild_bst();
    auto sorted = bst.sorted_by_price();
    cout << "Products sorted by price:\n";
    for (auto &p : sorted) cout << p.barcode << " | " << p.name << " | LE " << p.price << " | stock: " << p.stock << " | "<< p.category <<'\n';
}

inline void SupermarketSystem::print_products_sorted_category() {
    const_cast<SupermarketSystem*>(this)->rebuild_bst();
    auto sorted = bstbycategory.sorted_by_category();
    cout << "Products sorted by category:\n";
    for (auto &p : sorted) cout << p.category << " | " << p.barcode << " | " << p.name << " | LE " << p.price << " | stock: " << p.stock << '\n';
}

inline void SupermarketSystem::print_sales_report() {
    cout << "=== SALES REPORT ===\n";
    sales.print_sales();
    cout << "Top sold products:\n";
    auto top = sales.tally_products();
    for (size_t i=0;i<top.size() && i<10;++i) cout << i+1 << ". " << top[i].first << " x" << top[i].second << '\n';
}

// Minimal interactive console: simplified to avoid complex input interop across header-split code
inline void SupermarketSystem::interactive_console() {
    auto read_line = [](const string& prompt)->string{
        cout << prompt;
        string s;
        getline(cin, s);
        return s;
    };

    auto trim = [](string s)->string{
        auto is_space = [](char c){ return c==' '||c=='\t' || c=='\r' || c=='\n'; };
        size_t i=0; while (i<s.size() && is_space(s[i])) ++i;
        size_t j = s.size(); while (j>i && is_space(s[j-1])) --j;
        return s.substr(i, j-i);
    };

    auto read_int = [&](const string& prompt, int defaultVal = -1)->int{
        string s = read_line(prompt);
        if (s.empty()) return defaultVal;
        try { return stoi(s); } catch (...) { return defaultVal; }
    };

    while (true) {
        cout << "\n=== Supermarket Menu ===\n";
        cout << "1.  View Products\n";
        cout << "2.  Add product to inventory\n";
        cout << "3.  Add walk-in customer\n";
        cout << "4.  Add online customer\n";
        cout << "5.  Add item to customer's cart\n";
        cout << "6.  Remove item from customer's cart\n";
        cout << "7.  Undo customer's last cart action\n";
        cout << "8.  Enqueue walk-in to cashier\n";
        cout << "9.  Place online order (put in online queue)\n";
        cout << "10. Process Checkout\n";
        cout << "11. Undo last bill at cashier\n";
        cout << "12. Undo last bill at special needs cashier\n";
        cout << "13. Print customers\n";
        cout << "14. Print sales report\n";
        cout << "15. Show customer cart\n";
        cout << "16. Exit\n";

        int ch = read_int("Choose: ", -1);
        if (ch == -1) { cout << "Invalid input, try again.\n"; continue; }

        if (ch == 1) {
            // View Products Submenu
            while (true) {
                cout << "\n=== View Products ===\n";
                cout << "1. Print inventory\n";
                cout << "2. Print products sorted by price\n";
                cout << "3. Print products sorted by category\n";
                cout << "99. Back to main menu\n";
                int subch = read_int("Choose: ", -1);
                if (subch == -1) { cout << "Invalid input, try again.\n"; continue; }
                if (subch == 1) print_inventory();
                else if (subch == 2) print_products_sorted_price();
                else if (subch == 3) print_products_sorted_category();
                else if (subch == 99) break;
                else cout << "Unknown option\n";
            }
        }
        else if (ch == 2) {
            string bc = trim(read_line("Barcode: "));
            if (bc.empty()) { cout << "Invalid barcode (empty). Aborting add.\n"; continue; }
            string name = trim(read_line("Name: "));
            string price_s = trim(read_line("Price: "));
            string stock_s = trim(read_line("Stock: "));
            string exp = trim(read_line("Expiry (YYYY-MM-DD): "));
            string cat = trim(read_line("CATEGORY: "));
            if (cat.empty()) { cout << "Invalid category (empty). Aborting add.\n"; continue; }
            double price = 0.0; int stock = 0;
            try { price = stod(price_s); } catch(...) { cout << "Invalid price, using 0.\n"; }
            try { stock = stoi(stock_s); } catch(...) { cout << "Invalid stock, using 0.\n"; }
            if (price < 0.0) { cout << "Price cannot be negative. Aborting add.\n"; continue; }
            if (stock < 0) { cout << "Stock cannot be negative. Aborting add.\n"; continue; }
            if (inventory.add_product(Product(bc,name,price,stock,exp,cat))) cout << "Product added\n"; else cout << "Product exists\n";
            rebuild_bst();
        }
        else if (ch == 3) {
            string id = read_line("Customer ID: ");
            string name = read_line("Name: ");

            string type = read_line("is the Customer Type (Specialcustomer) (y/n) : ");
            if (type=="y" || type=="Y"){
                if (customers.count(id)) {
                    cout << "Customer exists\n"; 
                    continue;
                }
                customers[id] = make_unique<SpecialCustomer>(id, name);
                cout << "Special Customer added\n"; 
            }
            else {
                if (add_walkin_customer(id, name)) cout << "Walk-in added\n";
                else cout << "Customer exists\n";
            }
        }
        else if (ch == 4) {
            string id = read_line("Customer ID: ");
            string name = read_line("Name: ");
            string addr = read_line("Address: ");
            string pay = read_line("Payment method: ");
            int pr = -1; try { pr = stoi(read_line("Priority (1-10): ")); } catch(...) { pr = 5; }
            if (add_online_customer(id,name,addr,pay,pr)) cout << "Online customer added\n"; else cout << "Customer exists\n";
        }
        else if (ch == 5) {
            string cid = read_line("Customer ID: ");
            if (cid.empty()) { cout << "Invalid Customer ID\n"; continue; }
            while (true) {
                string bc = read_line("Product Barcode: ");
                if (bc.empty()) { cout << "Invalid barcode\n"; break; }
                int qty = -1; try { qty = stoi(read_line("Quantity: ")); } catch(...) { qty = 1; }
                customer_add_to_cart(cid, bc, qty);
                string more = read_line("Add another item for this customer? (y/n): ");
                if (more.empty() || (more[0] != 'y' && more[0] != 'Y')) break;
            }
        }
        else if (ch == 6) {
            string cid = read_line("Customer ID: ");
            string bc = read_line("Product Barcode: ");
            int qty = -1; try { qty = stoi(read_line("Quantity: ")); } catch(...) { qty = 1; }
            Customer* c = get_customer(cid);
            if (!c) { cout << "Customer not found\n"; continue; }
            int removed = c->cart.remove_item(bc, qty);
            if (removed > 0) { inventory.update_stock(bc, removed); cout << "Removed " << removed << " x " << bc << " from " << c->get_name() << " cart.\n"; }
            else cout << "No such item in cart\n";
        }
        else if (ch == 7) {
            string cid = read_line("Customer ID: "); auto res = customer_undo(cid); cout << res.second << '\n';
        }
        else if (ch == 8) { string cid = read_line("Customer ID: "); enqueue_walkin_to_cashier(cid); }
        else if (ch == 9) { string cid = read_line("Customer ID: "); place_online_order(cid); }
        else if (ch == 10) {
            while (true) {
                cout << "\n=== Process Checkout ===\n";
                cout << "1. Process checkout at regular cashier\n";
                cout << "2. Process checkout at special needs cashier\n";
                cout << "3. Process next online order\n";
                cout << "99. Back to main menu\n";
                int subch = read_int("Choose: ", -1);
                if (subch == -1) { cout << "Invalid input, try again.\n"; continue; }
                if (subch == 1) { int idx = -1; try { idx = stoi(read_line("Cashier index: ")); } catch(...) { idx = 0; } process_checkout_at_cashier(idx); }
                else if (subch == 2) process_checkout_at_specialneedscashier();
                else if (subch == 3) process_next_online_order();
                else if (subch == 99) break;
                else cout << "Unknown option\n";
            }
        }
        else if (ch == 11) { int idx = -1; try { idx = stoi(read_line("Cashier index: ")); } catch(...) { idx = 0; } cashier_undo_last_bill(idx); }
        else if (ch == 12) cashier_undo_last_specialneedscashier_bill();
        else if (ch == 13) list_customers();
        else if (ch == 14) print_sales_report();
        else if (ch == 15) { string cid = read_line("Customer ID: "); show_customer_cart(cid); }
        else if (ch == 16) { cout << "THANK YOU FOR USING OUR SYSTEM\n"; break; }
        else cout << "Unknown option\n";
    }
}

// Implement Customer checkout methods now that SupermarketSystem is defined
inline void Customer::checkout(SupermarketSystem& sys, int cashierIndex) {
    (void)cashierIndex;
    sys.enqueue_walkin_to_cashier(get_id());
}
inline void OnlineCustomer::checkout(SupermarketSystem& sys, int cashierIndex) {
    (void)cashierIndex;
    sys.place_online_order(get_id());
}
inline void SpecialCustomer::checkout(SupermarketSystem& sys, int cashierIndex) {
    (void)cashierIndex;
    sys.enqueue_specialneeds_to_cashier(this);
}

inline void Customer::print_info() const { cout << "Customer: " << id << " | " << name << " | Type: " << get_type() << '\n'; }
inline void OnlineCustomer::print_info() const { cout << "OnlineCustomer: " << id << " | " << name << " | addr: " << address << " | pay: " << paymentMethod << " | priority: " << priority << '\n'; }
inline void SpecialCustomer::print_info() const { cout << "SpecialCustomer: " << id << " | " << name << '\n'; }
