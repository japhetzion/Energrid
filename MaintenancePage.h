#ifndef MAINTENANCEPAGE_H
#define MAINTENANCEPAGE_H
#include <string>
#include <stdexcept>
#include "Page.h"
#include "PriorityQueue.h"
#include "Containers.h"
using namespace std;
class Troubleshooter {
public:
    static const int ISSUE_COUNT = 5;
    string issueName(int code) const;    
    string steps(int code) const;         
};
struct WorkOrder {
    int id;
    string building;
    int issueCode;
    int priority;       
    WorkOrder() : id(0), issueCode(0), priority(0) {}
    bool operator>(const WorkOrder& other) const {
        if (priority != other.priority)
            return priority > other.priority;
        return id < other.id;
    }
};

string priorityName(int p);
const int MAX_ORDERS = 100;
class MaintenancePage : public Page {
    Troubleshooter guide;
    PriorityQueue<WorkOrder, MAX_ORDERS> orders;   
    Stack<WorkOrder> history;
    int nextId;
    void reportIssue();
    void showGuide() const;
    void showSteps(int code, const string& title) const;
    void viewOrders() const;
    void respondToNext();
    void viewHistory() const;
    void reopenLast();
    void traceFault() const;
    void showMenu() const;

public:
    MaintenancePage(CampusData& d, Settings& s) : Page(d, s), nextId(1) {}
    string name() const { return "Maintenance & Troubleshooting"; }
    void run();
    int openOrders(WorkOrder out[], int maxCount) const { return orders.toSortedArray(out, maxCount); }
    int resolvedOrders(WorkOrder out[], int maxCount) const { return history.toArray(out, maxCount); }   
    int getNextId() const { return nextId; }
    void clearAll();
    void restore(const WorkOrder open[], int openCount, const WorkOrder done[], int doneCount, int next);
};

#endif
