// sales.h
#pragma once
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include "utils.h"
using namespace std;

struct SaleRecord {
    string saleId;
    string customerId;
    bool online;
    vector<pair<string,int>> items;
    double total;
    string time;
    SaleRecord* next;
    SaleRecord(const string& sid, const string& cid, bool ol, const vector<pair<string,int>>& it, double tot){
        saleId = sid;
        customerId = cid;
        online = ol;
        items = it;
        total = tot;
        time = now_string();
        next = nullptr;
    }

};

class SalesList {
private:
    SaleRecord* head = nullptr;
public:
    SalesList() = default;
    ~SalesList() {
        while (head != nullptr) {
            SaleRecord* t = head;
             head = head->next;
              delete t; 
        } 
    }

    void add_sale(SaleRecord* s) {
         s->next = head; head = s; 
    }

    bool remove_by_id(const string& saleId) {
        SaleRecord* cur = head;
        SaleRecord* prev = nullptr;
        while (cur != nullptr) {
            if (cur->saleId == saleId) {
                if (prev != nullptr) {prev->next = cur->next; } 
                else { head = cur->next; }
                delete cur; 
                return true;
            }
            prev = cur; cur = cur->next;
        }
        return false;
    }

    void print_sales() const {
        cout << "Sales records:\n";
        SaleRecord* cur = head;
        while (cur) {
            cout << cur->saleId << " | Cust: " << cur->customerId
                 << " | " << (cur->online ? "Online" : "Walk-in")
                 << " | LE " << cur->total << " | " << cur->time << '\n';
            for (auto &it : cur->items) cout << "   - " << it.first << " x" << it.second << '\n';
            cur = cur->next;
        }
    }

    vector<pair<string,int>> tally_products() const {
        unordered_map<string,int> tally;
        SaleRecord* cur = head;
        while (cur != nullptr) {
            for(int i=0;i<cur->items.size();++i){
                tally[cur->items[i].first] += cur->items[i].second;
            }
            cur = cur->next;
        }

        vector<pair<string,int>> v(tally.begin(), tally.end());
        sort(v.begin(), v.end(), [](auto &a, auto &b){ return a.second > b.second; });
        return v;
    }
};
