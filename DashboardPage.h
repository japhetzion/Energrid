#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H
#include "Page.h"
class DashboardPage : public Page {
    void showOverview() const;
    void showLiveMetrics() const;
    void showEnergyFlow() const;
    void showTrends() const;
    void showAlerts() const;
    void show() const;
public:
    DashboardPage(CampusData& d, Settings& s) : Page(d, s) {}
    string name() const { return "Dashboard"; }
    void run();
};

#endif
