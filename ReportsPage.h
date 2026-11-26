#ifndef REPORTSPAGE_H
#define REPORTSPAGE_H
#include <string>
#include "Page.h"
using namespace std;
const int MAX_REPORT_ROWS = MAX_BUILDINGS + 5;
struct ReportTable {
    string header[4];
    string cell[MAX_REPORT_ROWS][4];
    int rows;
    ReportTable() : rows(0) {}
};

class ReportsPage : public Page {
    int reportType;     
    int fromDay;
    int toDay;
    string reportName() const;
    string fileName() const;
    ReportTable build() const;
    void selectType();
    void setDateRange();
    void generateReport() const;
    void exportReport() const;
    void showMenu() const;

public:
    ReportsPage(CampusData& d, Settings& s) : Page(d, s), reportType(0), fromDay(1), toDay(7) {}
    string name() const { return "Reports"; }
    void run();
};

#endif
