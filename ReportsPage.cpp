#include <iostream>
#include <fstream>
#include "ReportsPage.h"
using namespace std;
string ReportsPage::reportName() const {
    if (reportType == 1) return "Building usage report";
    if (reportType == 2) return "Campus summary report";
    if (reportType == 3) return "Alert report";
    return "none selected";
}

string ReportsPage::fileName() const {
    if (reportType == 1) return "usage_report.csv";
    if (reportType == 2) return "campus_report.csv";
    return "alert_report.csv";
}
ReportTable ReportsPage::build() const {
    if (reportType == 0)
        throw runtime_error("Select a report type first (option 1).");
    if (data.list.getSize() == 0)
        throw runtime_error("No buildings added yet.");

    ReportTable t;
    bool hide = settings.isPrivacyMode();     

    if (reportType == 1) {
        t.header[0] = "Building"; t.header[1] = "Type";
        t.header[2] = "Total kWh"; t.header[3] = "Avg kWh/day";
        int index = 1;
        for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next, index++) {
            Building* b = cur->data;
            double sum = b->total(fromDay, toDay);            
            int days = b->getCount() < toDay ? b->getCount() : toDay;
            days = days - fromDay + 1;
            t.cell[t.rows][0] = hide ? "Building " + to_string(index) : b->getName();
            t.cell[t.rows][1] = b->type();
            t.cell[t.rows][2] = num1(sum);
            t.cell[t.rows][3] = days > 0 ? num1(sum / days) : "-";
            t.rows++;
        }
    } else if (reportType == 2) {
        double academic = 0, hostel = 0, lab = 0;
        int na = 0, nh = 0, nl = 0;
        for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
            string ty = cur->data->type();
            double sum = cur->data->total(fromDay, toDay);
            if (ty == "Academic Block") { academic += sum; na++; }
            else if (ty == "Hostel") { hostel += sum; nh++; }
            else { lab += sum; nl++; }
        }
        double campus = academic + hostel + lab;
        t.header[0] = "Category"; t.header[1] = "Buildings";
        t.header[2] = "Total kWh"; t.header[3] = "Share %";
        string names[3] = {"Academic Block", "Hostel", "Lab"};
        int counts[3] = {na, nh, nl};
        double totals[3] = {academic, hostel, lab};
        for (int i = 0; i < 3; i++) {
            t.cell[t.rows][0] = names[i];
            t.cell[t.rows][1] = to_string(counts[i]);
            t.cell[t.rows][2] = num1(totals[i]);
            t.cell[t.rows][3] = campus > 0 ? num1(totals[i] / campus * 100) : "0.0";
            t.rows++;
        }
        t.cell[t.rows][0] = "CAMPUS TOTAL";
        t.cell[t.rows][1] = to_string(data.list.getSize());
        t.cell[t.rows][2] = num1(campus);
        t.cell[t.rows][3] = campus > 0 ? "100.0" : "0.0";
        t.rows++;
    } else {
        t.header[0] = "Building"; t.header[1] = "Days over limit";
        t.header[2] = "Excess kWh"; t.header[3] = "Status";
        int index = 1;
        for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next, index++) {
            Building* b = cur->data;
            int over = 0;
            double excess = 0;
            for (int d = fromDay; d <= toDay && d <= b->getCount(); d++) {
                double r = b->getReading(d);
                if (r > b->getLimit()) {
                    over++;
                    excess += r - b->getLimit();
                }
            }
            t.cell[t.rows][0] = hide ? "Building " + to_string(index) : b->getName();
            t.cell[t.rows][1] = to_string(over);
            t.cell[t.rows][2] = num1(excess);
            t.cell[t.rows][3] = over == 0 ? "OK" : "ACTION NEEDED";
            t.rows++;
        }
    }
    return t;
}
void ReportsPage::selectType() {
    inputScreen("SELECT REPORT TYPE",
                "1 = Building usage report (per building)   2 = Campus summary report "
                "(by category)   3 = Alert report (days over the limit)");
    int t = readInt("Report type");
    if (t < 1 || t > 3)
        throw invalid_argument("Unknown report type.");
    reportType = t;
    showSuccess("Selected: " + reportName());
}
void ReportsPage::setDateRange() {
    inputScreen("DATE RANGE", "Reports cover day numbers 1 to 30. Type the first and last day.");
    int from = readInt("From day");
    int to = readInt("To day");
    if (from < 1 || to < from || to > MAX_DAYS)
        throw out_of_range("Invalid date range.");
    fromDay = from;
    toDay = to;
    showSuccess("Date range set: day " + to_string(fromDay) + " to day " + to_string(toDay) + ".");
}
void ReportsPage::generateReport() const {
    ReportTable t = build();
    header(reportName() + "  |  days " + to_string(fromDay) + " to " + to_string(toDay));
    if (settings.isPrivacyMode())
        drawRow(paint("Privacy mode is ON: building names are hidden.", AMBER));
    int w[4] = {15, 16, 11, 16};
    if (reportType == 1) { w[0] = 15; w[1] = 16; w[2] = 11; w[3] = 14; }
    if (reportType == 2) { w[0] = 16; w[1] = 11; w[2] = 14; w[3] = 12; }

    drawRow(GREY + padRight(t.header[0], w[0]) + padRight(t.header[1], w[1]) +
            padLeft(t.header[2], w[2]) + padLeft(t.header[3], w[3]) + RESET);
    for (int r = 0; r < t.rows; r++) {
        string last = t.cell[r][3];
        if (last == "ACTION NEEDED" || last == "OK")
            last = statusText(last);                
        drawRow(padRight(t.cell[r][0], w[0]) + padRight(t.cell[r][1], w[1]) +
                padLeft(t.cell[r][2], w[2]) + padLeft(last, w[3]));
    }
    drawBottom();
}
void ReportsPage::exportReport() const {
    ReportTable t = build();
    string file = fileName();
    ofstream out(file.c_str());
    if (!out)
        throw runtime_error("Could not create the file " + file);
    out << t.header[0] << "," << t.header[1] << "," << t.header[2] << "," << t.header[3] << "\n";
    for (int r = 0; r < t.rows; r++)
        out << t.cell[r][0] << "," << t.cell[r][1] << "," << t.cell[r][2] << "," << t.cell[r][3] << "\n";
    out.close();
    showSuccess("Exported to " + file + " (saved in the project folder).");
}
void ReportsPage::showMenu() const {
    header("REPORTS");
    drawRow(GREY + "Report: " + RESET + reportName());
    drawRow(GREY + "Days:   " + RESET + to_string(fromDay) + " to " + to_string(toDay));
    drawRow();
    drawMenuItem("1", "Select report type");
    drawMenuItem("2", "Set date range");
    drawMenuItem("3", "Generate report");
    drawMenuItem("4", "Export report (CSV file)");
    drawRow();
    drawMenuItem("0", "Back");
    drawBottom();
}
void ReportsPage::run() {
    int choice;
    do {
        showMenu();
        choice = readInt("Choose an option");
        if (choice == 0)
            break;
        try {
            switch (choice) {
            case 1: selectType(); break;
            case 2: setDateRange(); break;
            case 3: generateReport(); break;
            case 4: exportReport(); break;
            default: showError("Please choose a number from the menu.");
            }
        } catch (exception& e) {
            showError(e.what());
        }
        pauseScreen();
    } while (true);
}
