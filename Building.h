#ifndef BUILDING_H
#define BUILDING_H

#include <string>
#include <stdexcept>
#include <cstddef>
using namespace std;

const int MAX_DAYS = 30;        
const int LIVE_HISTORY = 16;   
struct LiveData {
    double kw;                       
    double todayKwh;                 
    double history[LIVE_HISTORY];    
    int historyCount;
    int highStreak;                  
    bool alerted;                   
    LiveData() { reset(); }
    void reset() {
        kw = 0;
        todayKwh = 0;
        historyCount = 0;
        highStreak = 0;
        alerted = false;
        for (int i = 0; i < LIVE_HISTORY; i++)
            history[i] = 0;
    }
};

class Building {
protected:
    string name;
    double usage[MAX_DAYS];   
    int count;               
    double dailyLimit;        

public:
    Building(string n, double limit) : name(n), count(0), dailyLimit(limit) {
        for (int i = 0; i < MAX_DAYS; i++)
            usage[i] = 0;
    }
    virtual ~Building() {}

    string getName() const { return name; }
    double getLimit() const { return dailyLimit; }
    int getCount() const { return count; }

    double getReading(int day) const {
        if (day < 1 || day > count)
            throw out_of_range("No reading recorded for that day.");
        return usage[day - 1];
    }

    double latest() const {
        if (count == 0)
            throw runtime_error("No readings entered yet.");
        return usage[count - 1];
    }

   
    LiveData live;
    double peakLimit() const { return dailyLimit / 24.0 * 1.5; }
    void recordLive(double kw) {
        live.kw = kw;
        live.todayKwh += kw;
        if (live.historyCount < LIVE_HISTORY) {
            live.history[live.historyCount++] = kw;
        } else {
            for (int i = 1; i < LIVE_HISTORY; i++)
                live.history[i - 1] = live.history[i];
            live.history[LIVE_HISTORY - 1] = kw;
        }
    }
    double endLiveDay() {
        double total = live.todayKwh;
        live.todayKwh = 0;
        return total;
    }

   
    void addReading(double kwh) {
        if (kwh < 0)
            throw invalid_argument("Reading cannot be negative.");
        if (count >= MAX_DAYS)
            throw out_of_range("Month is full (30 readings already).");
        usage[count++] = kwh;
    }
    void addReading(int day, double kwh) {
        if (day < 1 || day > count)
            throw out_of_range("That day has no reading to correct.");
        if (kwh < 0)
            throw invalid_argument("Reading cannot be negative.");
        usage[day - 1] = kwh;
    }
    double removeLastReading() {
        if (count == 0)
            throw runtime_error("No readings to undo.");
        count--;
        double value = usage[count];
        usage[count] = 0;
        return value;
    }
    double total() const {
        double sum = 0;
        for (int i = 0; i < count; i++)
            sum += usage[i];
        return sum;
    }
    double total(int fromDay, int toDay) const {
        if (fromDay < 1 || toDay < fromDay)
            throw invalid_argument("Invalid date range.");
        double sum = 0;
        for (int d = fromDay; d <= toDay && d <= count; d++)
            sum += usage[d - 1];
        return sum;
    }
    double average() const {
        if (count == 0)
            throw runtime_error("No readings entered yet.");
        return total() / count;
    }
    double usageRatio() const {
        if (count == 0)
            return 0;
        return average() / dailyLimit;
    }
    string priority() const {
        if (count == 0) return "NO DATA";
        double r = usageRatio();
        if (r > 1.0) return "HIGH";
        if (r > 0.8) return "MEDIUM";
        return "LOW";
    }
    bool operator>(const Building& other) const {
        return total() > other.total();
    }
    double operator+(const Building& other) const {
        return total() + other.total();
    }
    virtual string type() const = 0;
    virtual string tips() const = 0;
};
class AcademicBlock : public Building {
public:
    AcademicBlock(string n) : Building(n, 400) {}
    string type() const { return "Academic Block"; }
    string tips() const {
        return "- Switch off lights and fans in empty classrooms.\n"
               "- Keep AC at 24 C and close doors while it runs.\n"
               "- Turn off projectors and computers after the last class.";
    }
};
class Hostel : public Building {
public:
    Hostel(string n) : Building(n, 250) {}
    string type() const { return "Hostel"; }
    string tips() const {
        return "- Use timers for geysers and water heaters.\n"
               "- Replace old bulbs with LED lights.\n"
               "- Unplug chargers and switch off room appliances at night.";
    }
};
class Lab : public Building {
public:
    Lab(string n) : Building(n, 500) {}
    string type() const { return "Lab"; }
    string tips() const {
        return "- Shut down idle computers and instruments.\n"
               "- Run heavy equipment during off-peak hours.\n"
               "- Check that equipment is not left on after lab hours.";
    }
};
struct BuildingRank {
    Building* building;
    double ratio;

    BuildingRank() : building(NULL), ratio(0) {}
    BuildingRank(Building* b) : building(b), ratio(b->usageRatio()) {}

    bool operator>(const BuildingRank& other) const {
        return ratio > other.ratio;
    }
};
#endif
