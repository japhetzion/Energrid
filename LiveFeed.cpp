#include <fstream>
#include <cstdlib>
#include "LiveFeed.h"
#include "Ui.h"
using namespace std;
SimulatedSource::SimulatedSource(unsigned int startSeed) : seed(startSeed), known(0) {
    for (int i = 0; i < MAX_BUILDINGS; i++)
        spikeLeft[i] = 0;
}
int SimulatedSource::nextRandom() {
    seed = seed * 1103515245u + 12345u;
    return (int)((seed >> 16) & 0x7FFF);
}
int SimulatedSource::spikeIndex(const string& name) {
    for (int i = 0; i < known; i++)
        if (names[i] == name)
            return i;
    if (known >= MAX_BUILDINGS)
        return 0;
    names[known] = name;
    spikeLeft[known] = 0;
    return known++;
}

void SimulatedSource::startTick(int) {
}
bool SimulatedSource::getLoad(const Building* b, int hour, double& kw) {
    int letters = 0;
    for (size_t i = 0; i < b->getName().length(); i++)
        letters += b->getName()[i];
    double bias = 0.95 + (letters % 20) / 100.0;
    double base = b->peakLimit() / 1.5 * 0.85 * bias;
    double factor;
    string type = b->type();
    if (type == "Academic Block") {
        if (hour >= 8 && hour < 18) factor = 1.35;
        else if (hour >= 18 && hour < 22) factor = 0.9;
        else factor = 0.45;
    } else if (type == "Hostel") {
        if (hour >= 18) factor = 1.5;
        else if (hour >= 9) factor = 0.6;
        else if (hour >= 6) factor = 1.1;
        else factor = 0.5;
    } else {                                  
        factor = (hour >= 9 && hour < 19) ? 1.25 : 0.8;
    }
    int slot = spikeIndex(b->getName());
    if (spikeLeft[slot] > 0) {
        factor *= 1.8;
        spikeLeft[slot]--;
    } else if (nextRandom() % 100 < 4) {
        spikeLeft[slot] = 2;
        factor *= 1.8;
    }

    double noise = 1.0 + ((nextRandom() % 17) - 8) / 100.0;  
    kw = base * factor * noise;
    return true;
}
static string trimText(const string& s) {
    size_t first = s.find_first_not_of(" \t\r");
    if (first == string::npos)
        return "";
    size_t last = s.find_last_not_of(" \t\r");
    return s.substr(first, last - first + 1);
}
void FileSource::store(const string& name, double kw) {
    for (int i = 0; i < known; i++) {
        if (names[i] == name) {
            loads[i] = kw;
            return;
        }
    }
    if (known < MAX_BUILDINGS) {
        names[known] = name;
        loads[known] = kw;
        known++;
    }
}
void FileSource::startTick(int) {
    ifstream in(path.c_str());
    if (!in)
        throw runtime_error("Could not open " + path + ". Put it in the project folder, "
                            "with one line per building, like:  Block A,18.5");
    known = 0;
    string line;
    while (getline(in, line)) {
        size_t comma = line.rfind(',');
        if (comma == string::npos)
            continue;
        string name = trimText(line.substr(0, comma));
        string value = trimText(line.substr(comma + 1));
        char* end;
        double kw = strtod(value.c_str(), &end);
        if (end == value.c_str() || *end != '\0' || kw < 0)
            continue;                          
        store(name, kw);
    }
}
bool FileSource::getLoad(const Building* b, int, double& kw) {
    for (int i = 0; i < known; i++) {
        if (names[i] == b->getName()) {
            kw = loads[i];
            return true;
        }
    }
    return false;                              
}
LiveMonitor::LiveMonitor(CampusData& d, Settings& s, DataSource* src)
    : data(d), settings(s), source(src), hour(0), day(1) {
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
        cur->data->live.reset();
        if (cur->data->getCount() + 1 > day)
            day = cur->data->getCount() + 1;   
    }
}

LiveMonitor::~LiveMonitor() {
    delete source;
}

int LiveMonitor::tick(string events[], int maxEvents) {
    int n = 0;
    source->startTick(hour);

    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
        Building* b = cur->data;
        double kw;
        if (!source->getLoad(b, hour, kw))
            continue;                          
        b->recordLive(kw);
        double trigger = b->peakLimit() * settings.getAlertPercent() / 100.0;
        if (kw > trigger) {
            b->live.highStreak++;
        } else {
            b->live.highStreak = 0;
            b->live.alerted = false;
        }
        if (b->live.highStreak >= 3 && !b->live.alerted) {
            b->live.alerted = true;
            string message = b->getName() + ": " + num1(kw) + " kW for 3 hours (limit " +
                             num1(b->peakLimit()) + ")";
            if (settings.notificationsEnabled())
                data.alerts.enqueue(message);           
            if (n < maxEvents)
                events[n++] = "ALERT  " + message;
        }
    }
    hour++;
    if (hour == 24) {                                   
        hour = 0;
        for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
            Building* b = cur->data;
            if (b->live.historyCount == 0)
                continue;                               
            double kwh = b->endLiveDay();
            if (b->getCount() >= MAX_DAYS) {
                if (n < maxEvents)
                    events[n++] = b->getName() + ": month is full, day not saved";
                continue;
            }
            b->addReading(kwh);                        
            bool high = data.raiseAlertIfHigh(b, b->getCount(), kwh,
                                              settings.getAlertPercent(),
                                              settings.notificationsEnabled());
            if (n < maxEvents)
                events[n++] = "Day " + to_string(day) + " closed: " + b->getName() + " " +
                              num1(kwh) + " kWh" + (high ? "  (over the daily limit)" : "");
        }
        day++;
    }
    return n;
}
void LiveMonitor::discardPartialDay() {
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next)
        cur->data->live.todayKwh = 0;
}
