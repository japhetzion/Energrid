#include <iostream>
#include "DashboardPage.h"
using namespace std;
const int NAME_WIDTH = 14;
void DashboardPage::showOverview() const {
    double total = 0;
    Building* top = data.list.getHead()->data;
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
        total += cur->data->total();
        if (*(cur->data) > *top)          
            top = cur->data;
    }
    drawRow(GREY + "Buildings: " + RESET + BOLD + to_string(data.list.getSize()) + RESET +
            GREY + "     Total: " + RESET + BOLD + num1(total) + " kWh" + RESET +
            GREY + "     Highest: " + RESET + BOLD + top->getName() + RESET);
    drawRow(GREY + "Power network: " + RESET + to_string(data.grid.getNodeCount()) +
            GREY + " points, " + RESET + to_string(data.grid.getLinkCount()) +
            GREY + " links" + RESET);
}
void DashboardPage::showLiveMetrics() const {
    drawMiddle("LIVE METRICS");
    drawRow(GREY + padRight("Building", NAME_WIDTH) + padLeft("Latest", 8) +
            padLeft("Limit", 9) + "   Status" + RESET);
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
        Building* b = cur->data;
        string latest = "-";
        string status = "NO DATA";
        if (b->getCount() > 0) {
            latest = num1(b->latest());
            status = "OK";
            if (b->latest() > b->getLimit()) status = "OVER LIMIT";
            else if (b->latest() > 0.8 * b->getLimit()) status = "NEAR LIMIT";
        }
        drawRow(padRight(b->getName(), NAME_WIDTH) + padLeft(latest, 8) +
                padLeft(num1(b->getLimit()), 9) + "   " + statusText(status));
    }
}
void DashboardPage::showEnergyFlow() const {
    double academic = 0, hostel = 0, lab = 0;
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
        string t = cur->data->type();
        if (t == "Academic Block") academic += cur->data->total();
        else if (t == "Hostel") hostel += cur->data->total();
        else lab += cur->data->total();
    }
    double total = academic + hostel + lab;
        drawMiddle("ENERGY FLOW");
    if (total == 0) {
        drawRow(paint("No consumption recorded yet.", GREY));
        return;
    }
    string names[3] = {"Academic", "Hostels", "Labs"};
    double values[3] = {academic, hostel, lab};
    for (int i = 0; i < 3; i++) {
        double percent = values[i] / total * 100;
        drawRow(padRight(names[i], 10) + bar(percent, 20, accent()) + "  " +
                padLeft(num1(percent) + "%", 6) + "  " + padLeft(num1(values[i]), 8) + " kWh");
    }
}
void DashboardPage::showTrends() const {
    drawMiddle("TRENDS (last 3 days vs the 3 days before)");
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
        Building* b = cur->data;
        int n = b->getCount();
        if (n < 6) {
            drawRow(padRight(b->getName(), NAME_WIDTH) + paint("needs 6 days of readings", GREY));
            continue;
        }
        double recent = 0, previous = 0;
        for (int d = n - 2; d <= n; d++)
            recent += b->getReading(d);
        for (int d = n - 5; d <= n - 3; d++)
            previous += b->getReading(d);
        recent /= 3;
        previous /= 3;
        double change = 0;
        if (previous > 0)
            change = (recent - previous) / previous * 100;

        string label = "STABLE";
        if (change > 5) label = "RISING";
        else if (change < -5) label = "FALLING";
        string sign = change >= 0 ? "+" : "";
        drawRow(padRight(b->getName(), NAME_WIDTH) + padRight(trendText(label), 12) +
                padLeft(sign + num1(change) + "%", 9) + GREY + "   now " + RESET +
                num1(recent) + " kWh/day");
    }
}
void DashboardPage::showAlerts() const {
    drawMiddle("ALERTS (latest)");
    int n = data.alerts.size();
    if (n == 0) {
        drawRow(paint("No active alerts.", GREEN));
        return;
    }
    int start = n > 3 ? n - 3 : 0;
    for (int i = start; i < n; i++)
        drawWrapped("! " + data.alerts.at(i), AMBER);
    if (start > 0)
        drawRow(paint("(" + to_string(start) + " earlier alerts not shown)", GREY));
}
void DashboardPage::show() const {
    header("DASHBOARD");
    if (data.list.getSize() == 0) {
        drawWrapped("No buildings yet. Add one in Energy Control, or choose "
                    "'Load sample data' in the main menu.", GREY);
        drawBottom();
        return;
    }
    showOverview();
    showLiveMetrics();
    if (!settings.isCompact()) {         
        showEnergyFlow();
        showTrends();
    }
    showAlerts();
    drawBottom();
}

void DashboardPage::run() {
    int choice;
    do {
        show();
        cout << "\n  " << GREY << "[1] Mark alerts as read     [0] Back" << RESET << "\n";
        choice = readInt("Choose an option");
        if (choice == 1) {
            try {
                settings.requireStaff();
                data.alerts.clear();
                showSuccess("All alerts marked as read.");
            } catch (exception& e) {
                showError(e.what());
            }
            pauseScreen();
        }
    } while (choice != 0);
}
