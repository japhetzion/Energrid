#ifndef ENERGYCONTROLPAGE_H
#define ENERGYCONTROLPAGE_H

#include "Page.h"
#include "PriorityQueue.h"
#include "LiveFeed.h"

class EnergyControlPage : public Page {
    void addBuilding();
    void removeBuilding();
    void listBuildings() const;
    void compareBuildings() const;
    void showTips() const;
    void enterReading();
    void undoReading();
    void correctReading();
    void runLiveMonitor();                 
    void drawLive(const LiveMonitor& monitor, const string log[], int logSize,
                  const string& footer) const;
    void showPriorityRanking() const;
    void showNetwork() const;
    void addNetworkLink();
    void showMenu() const;
public:
    EnergyControlPage(CampusData& d, Settings& s) : Page(d, s) {}
    string name() const { return "Energy Control"; }
    void run();
};

#endif
