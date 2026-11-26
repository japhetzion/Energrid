#include <iostream>
#include "MaintenancePage.h"
using namespace std;
const int NAME_WIDTH = 14;
string Troubleshooter::issueName(int code) const {
    switch (code) {
    case 1: return "Sudden spike in usage";
    case 2: return "Lights/fans on in empty rooms";
    case 3: return "AC consuming too much power";
    case 4: return "Wrong or zero meter readings";
    case 5: return "Breaker tripping repeatedly";
    default: throw invalid_argument("Unknown issue number.");
    }
}
string Troubleshooter::steps(int code) const {
    switch (code) {
    case 1:
        return "1. Check if heavy equipment or AC was left running.\n"
               "2. Inspect for faulty or short-circuited appliances.\n"
               "3. Compare with the same day last week.\n"
               "4. Call the electrician if the spike continues.";
    case 2:
        return "1. Do a round of the block after working hours.\n"
               "2. Put up 'Switch off when leaving' notices.\n"
               "3. Consider motion-sensor switches.";
    case 3:
        return "1. Set the temperature to 24 C or above.\n"
               "2. Clean or replace the filters.\n"
               "3. Check doors and windows for air leaks.\n"
               "4. Service the unit if it has not been serviced this year.";
    case 4:
        return "1. Check the meter connections and display.\n"
               "2. Re-enter the reading carefully (Energy Control, option 8).\n"
               "3. Report to the maintenance office to test the meter.";
    case 5:
        return "1. Switch off non-essential loads on that circuit.\n"
               "2. Reset the breaker once; do not keep resetting it.\n"
               "3. Look for overloaded sockets or damaged wiring.\n"
               "4. Call a licensed electrician if it trips again.";
    default:
        throw invalid_argument("Unknown issue number.");
    }
}
string priorityName(int p) {
    switch (p) {
    case 1: return "Low";
    case 2: return "Normal";
    case 3: return "Medium";
    case 4: return "High";
    case 5: return "Critical";
    default: return "Unknown";
    }
}
void MaintenancePage::showSteps(int code, const string& title) const {
    header(title);
    drawRow(BOLD + guide.issueName(code) + RESET);
    drawMiddle();
    drawWrapped(guide.steps(code), GREEN);
    drawBottom();
}
void MaintenancePage::reportIssue() {
    inputScreen("REPORT AN ISSUE",
                "1 Sudden spike   2 Lights on in empty rooms   3 AC using too much power   "
                "4 Wrong meter reading   5 Breaker tripping");
    Building* b = data.table.find(readText("Building name"));
    int code = readInt("Issue (1 to 5)");
    guide.issueName(code);                     
    int priority = readInt("Priority (1 Low, 2 Normal, 3 Medium, 4 High, 5 Critical)");
    if (priority < 1 || priority > 5)
        throw out_of_range("Priority must be between 1 and 5.");

    WorkOrder w;
    w.id = nextId;
    w.building = b->getName();
    w.issueCode = code;
    w.priority = priority;
    orders.push(w);                            
    nextId++;
    showSteps(code, "WORK ORDER #" + to_string(w.id) + " CREATED");
    showSuccess("Work order #" + to_string(w.id) + " for " + w.building + " [" +
                priorityName(priority) + "]. Start with the steps above.");
}
void MaintenancePage::showGuide() const {
    inputScreen("TROUBLESHOOTING GUIDE",
                "1 Sudden spike   2 Lights on in empty rooms   3 AC using too much power   "
                "4 Wrong meter reading   5 Breaker tripping");
    int code = readInt("Issue (1 to 5)");
    showSteps(code, "TROUBLESHOOTING");
}
void MaintenancePage::viewOrders() const {
    WorkOrder list[MAX_ORDERS];
    int n = orders.toSortedArray(list, MAX_ORDERS);
    if (n == 0)
        throw runtime_error("There are no open work orders.");
    header("OPEN WORK ORDERS (most urgent first)");
    drawRow(GREY + padRight("#", 4) + padRight("Priority", 11) + padRight("Building", NAME_WIDTH) +
            "Issue" + RESET);
    for (int i = 0; i < n; i++) {
        string p = priorityName(list[i].priority);
        drawRow(padRight("#" + to_string(list[i].id), 4) + padRight(statusText(p), 11) +
                padRight(list[i].building, NAME_WIDTH) + guide.issueName(list[i].issueCode));
    }
    drawBottom();
}
void MaintenancePage::respondToNext() {
    settings.requireStaff();
    WorkOrder w = orders.pop();                
    history.push(w);                           
    showSteps(w.issueCode, "RESPONDING TO WORK ORDER #" + to_string(w.id));
    showSuccess("Work order #" + to_string(w.id) + " (" + w.building + ", " +
                priorityName(w.priority) + ") is marked as resolved.");
}

void MaintenancePage::viewHistory() const {
    WorkOrder list[MAX_ORDERS];
    int n = history.toArray(list, MAX_ORDERS);
    if (n == 0)
        throw runtime_error("No work orders have been resolved yet.");
    header("RESPONSE HISTORY (latest first)");
    drawRow(GREY + padRight("#", 4) + padRight("Priority", 11) + padRight("Building", NAME_WIDTH) +
            "Issue" + RESET);
    for (int i = 0; i < n; i++) {
        string p = priorityName(list[i].priority);
        drawRow(padRight("#" + to_string(list[i].id), 4) + padRight(statusText(p), 11) +
                padRight(list[i].building, NAME_WIDTH) + guide.issueName(list[i].issueCode));
    }
    drawBottom();
}
void MaintenancePage::reopenLast() {
    settings.requireStaff();
    WorkOrder w = history.pop();              
    orders.push(w);
    showSuccess("Work order #" + to_string(w.id) + " (" + w.building + ") was reopened.");
}
void MaintenancePage::traceFault() const {
    inputScreen("TRACE A FAULT",
                "Type the point of the power network that failed, for example  Feeder 2.  "
                "The search follows the network and lists everything that loses power.");
    string point = readText("Failed point");

    string affected[100];
    int n = data.grid.affectedBy(point, affected, 100);   

    header("FAULT AT: " + point);
    if (n == 0) {
        drawRow(paint("Nothing is fed by this point, so nothing else is affected.", GREEN));
        drawBottom();
        return;
    }
    drawRow(RED + to_string(n) + " point(s) lose power:" + RESET);
    drawMiddle();
    for (int i = 0; i < n; i++) {
        if (data.table.contains(affected[i]))
            drawRow(RED + "! " + RESET + padRight(affected[i], NAME_WIDTH) + GREY + "building" + RESET);
        else
            drawRow(AMBER + "- " + RESET + affected[i]);
    }
    drawBottom();
}
void MaintenancePage::showMenu() const {
    header("MAINTENANCE & TROUBLESHOOTING");
    drawRow(GREY + "ISSUES" + RESET);
    drawMenuItem("1", "Report an issue (creates a work order)");
    drawMenuItem("2", "Troubleshooting guide");
    drawRow();
    drawRow(GREY + "WORK ORDERS" + RESET);
    drawMenuItem("3", "View open work orders");
    drawMenuItem("4", "Respond to the most urgent work order");
    drawMenuItem("5", "Response history");
    drawMenuItem("6", "Reopen the last resolved work order");
    drawRow();
    drawRow(GREY + "POWER NETWORK" + RESET);
    drawMenuItem("7", "Trace a fault");
    drawRow();
    drawMenuItem("0", "Back");
    drawBottom();
}
void MaintenancePage::run() {
    int choice;
    do {
        showMenu();
        choice = readInt("Choose an option");
        if (choice == 0)
            break;
        try {
            switch (choice) {
            case 1: reportIssue(); break;
            case 2: showGuide(); break;
            case 3: viewOrders(); break;
            case 4: respondToNext(); break;
            case 5: viewHistory(); break;
            case 6: reopenLast(); break;
            case 7: traceFault(); break;
            default: showError("Please choose a number from the menu.");
            }
        } catch (exception& e) {
            showError(e.what());
        }
        pauseScreen();
    } while (true);
}
void MaintenancePage::clearAll() {
    while (!orders.isEmpty())
        orders.pop();
    while (!history.isEmpty())
        history.pop();
    nextId = 1;
}
void MaintenancePage::restore(const WorkOrder open[], int openCount,
                              const WorkOrder done[], int doneCount, int next) {
    clearAll();
    for (int i = 0; i < openCount; i++)
        orders.push(open[i]);
    for (int i = doneCount - 1; i >= 0; i--)
        history.push(done[i]);
    nextId = next;
}
