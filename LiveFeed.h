#ifndef LIVEFEED_H
#define LIVEFEED_H

#include <string>
#include "CampusData.h"
#include "Settings.h"
using namespace std;
class DataSource {
public:
    virtual ~DataSource() {}
    virtual string name() const = 0;
    virtual void startTick(int hour) = 0;
    virtual bool getLoad(const Building* b, int hour, double& kw) = 0;
};
class SimulatedSource : public DataSource {
    unsigned int seed;
    string names[MAX_BUILDINGS];
    int spikeLeft[MAX_BUILDINGS];     
    int known;
    int nextRandom();
    int spikeIndex(const string& name);

public:
    SimulatedSource(unsigned int startSeed);
    string name() const { return "Simulated feed"; }
    void startTick(int hour);
    bool getLoad(const Building* b, int hour, double& kw);
};
class FileSource : public DataSource {
    string path;
    string names[MAX_BUILDINGS];
    double loads[MAX_BUILDINGS];
    int known;

    void store(const string& name, double kw);

public:
    FileSource(const string& filePath) : path(filePath), known(0) {}
    string name() const { return "File feed (" + path + ")"; }
    void startTick(int hour);
    bool getLoad(const Building* b, int hour, double& kw);
};
class LiveMonitor {
    CampusData& data;
    Settings& settings;
    DataSource* source;     
    int hour;               
    int day;                 

    LiveMonitor(const LiveMonitor&);
    LiveMonitor& operator=(const LiveMonitor&);

public:
    LiveMonitor(CampusData& d, Settings& s, DataSource* src);
    ~LiveMonitor();
    int getHour() const { return hour; }
    int getDay() const { return day; }
    string sourceName() const { return source->name(); }
    int tick(string events[], int maxEvents);
    void discardPartialDay();
};

#endif
