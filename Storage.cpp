#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include "Storage.h"
using namespace std;
const int MAX_SAVED_LINKS = 500;
static void fail(int lineNo, const string& message) {
    throw runtime_error("Saved data, line " + to_string(lineNo) + ": " + message);
}
static int splitFields(const string& line, string out[], int maxFields) {
    int n = 0;
    size_t start = 0;
    while (n < maxFields) {
        size_t bar = line.find('|', start);
        if (bar == string::npos) {
            out[n++] = line.substr(start);
            break;
        }
        out[n++] = line.substr(start, bar - start);
        start = bar + 1;
    }
    return n;
}
static int toInt(const string& text, int lineNo) {
    char* end;
    long value = strtol(text.c_str(), &end, 10);
    if (text.empty() || *end != '\0')
        fail(lineNo, "expected a whole number but found '" + text + "'");
    return (int)value;
}
static double toDouble(const string& text, int lineNo) {
    char* end;
    double value = strtod(text.c_str(), &end);
    if (text.empty() || *end != '\0')
        fail(lineNo, "expected a number but found '" + text + "'");
    return value;
}
void Storage::save(const string& path, const CampusData& data, const Settings& settings,
                   const MaintenancePage& maintenance) {
    string tempPath = path + ".tmp";
    ofstream out(tempPath.c_str());
    if (!out)
        throw runtime_error("Could not write the file " + tempPath);
    out << setprecision(10);

    out << "ENERGRID|1\n";
    out << "SETTINGS|" << settings.notificationsEnabled() << "|" << settings.getAlertPercent() << "|"
        << settings.isPrivacyMode() << "|" << settings.isCompact() << "|" << settings.getTheme() << "\n";
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
        const Building* b = cur->data;
        int type = 3;
        if (b->type() == "Academic Block") type = 1;
        else if (b->type() == "Hostel") type = 2;
        out << "BUILDING|" << type << "|" << b->getName() << "|" << b->getCount() << "|";
        for (int day = 1; day <= b->getCount(); day++) {
            if (day > 1)
                out << ";";
            out << b->getReading(day);
        }
        out << "\n";
    }
    if (data.grid.getLinkCount() > MAX_SAVED_LINKS)
        throw runtime_error("The power network has too many links to save.");
    string from[MAX_SAVED_LINKS], to[MAX_SAVED_LINKS];
    int links = data.grid.linkList(from, to, MAX_SAVED_LINKS);
    for (int i = 0; i < links; i++)
        out << "LINK|" << from[i] << "|" << to[i] << "\n";
    for (int i = 0; i < data.alerts.size(); i++)
        out << "ALERT|" << data.alerts.at(i) << "\n";
    WorkOrder open[MAX_ORDERS], done[MAX_ORDERS];
    int openCount = maintenance.openOrders(open, MAX_ORDERS);
    int doneCount = maintenance.resolvedOrders(done, MAX_ORDERS);
    for (int i = 0; i < openCount; i++)
        out << "ORDER|O|" << open[i].id << "|" << open[i].building << "|"
            << open[i].issueCode << "|" << open[i].priority << "\n";
    for (int i = 0; i < doneCount; i++)
        out << "ORDER|R|" << done[i].id << "|" << done[i].building << "|"
            << done[i].issueCode << "|" << done[i].priority << "\n";
    out << "NEXTID|" << maintenance.getNextId() << "\n";
    out.close();
    if (out.fail())
        throw runtime_error("Writing to " + tempPath + " failed (disk full?).");
    std::remove(path.c_str());
    if (std::rename(tempPath.c_str(), path.c_str()) != 0)
        throw runtime_error("Could not replace " + path + ". Your data is in " + tempPath + ".");
}
bool Storage::load(const string& path, CampusData& data, Settings& settings,
                   MaintenancePage& maintenance) {
    ifstream in(path.c_str());
    if (!in) {
        in.clear();
        in.open((path + ".tmp").c_str());      
        if (!in)
            return false;                     
    }
    string line;
    int lineNo = 1;
    if (!getline(in, line))
        throw runtime_error("The saved data file is empty.");
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
    if (line != "ENERGRID|1")
        fail(1, "this is not an Energrid data file");

    WorkOrder open[MAX_ORDERS], done[MAX_ORDERS];
    int openCount = 0, doneCount = 0;
    int nextId = 1;
    while (getline(in, line)) {
        lineNo++;
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            continue;
        string f[8];
        int n = splitFields(line, f, 8);
        const string& tag = f[0];

        if (tag == "SETTINGS") {
            if (n < 6) fail(lineNo, "the settings line is incomplete");
            try {
                settings.setNotifications(toInt(f[1], lineNo) != 0);
                settings.setAlertPercent(toInt(f[2], lineNo));
                settings.setPrivacyMode(toInt(f[3], lineNo) != 0);
                settings.setCompact(toInt(f[4], lineNo) != 0);
                settings.setTheme(toInt(f[5], lineNo));
            } catch (runtime_error&) {
                throw;                         
            } catch (exception& e) {
                fail(lineNo, e.what());
            }
        } else if (tag == "BUILDING") {
            if (n < 5) fail(lineNo, "the building line is incomplete");
            int type = toInt(f[1], lineNo);
            int count = toInt(f[3], lineNo);
            if (type < 1 || type > 3) fail(lineNo, "unknown building type");
            if (count < 0 || count > MAX_DAYS) fail(lineNo, "the number of days is wrong");

            Building* b;
            if (type == 1) b = new AcademicBlock(f[2]);
            else if (type == 2) b = new Hostel(f[2]);
            else b = new Lab(f[2]);

            try {
                istringstream values(f[4]);
                string item;
                int got = 0;
                while (getline(values, item, ';')) {
                    b->addReading(toDouble(item, lineNo));
                    got++;
                }
                if (got != count)
                    fail(lineNo, "the number of readings does not match the number of days");
            } catch (exception& e) {
                delete b;
                throw runtime_error(e.what());
            }
            try {
                data.addBuilding(b);          
            } catch (exception& e) {
                fail(lineNo, e.what());
            }

        } else if (tag == "LINK") {
            if (n < 3) fail(lineNo, "the link line is incomplete");
            try {
                data.grid.addLink(f[1], f[2]);
            } catch (exception& e) {
                fail(lineNo, e.what());
            }
        } else if (tag == "ALERT") {
            if (n < 2) fail(lineNo, "the alert line is incomplete");
            data.alerts.enqueue(f[1]);

        } else if (tag == "ORDER") {
            if (n < 6) fail(lineNo, "the work order line is incomplete");
            WorkOrder w;
            w.id = toInt(f[2], lineNo);
            w.building = f[3];
            w.issueCode = toInt(f[4], lineNo);
            w.priority = toInt(f[5], lineNo);
            if (w.priority < 1 || w.priority > 5) fail(lineNo, "the priority must be 1 to 5");
            if (f[1] == "O") {
                if (openCount >= MAX_ORDERS) fail(lineNo, "too many open work orders");
                open[openCount++] = w;
            } else if (f[1] == "R") {
                if (doneCount >= MAX_ORDERS) fail(lineNo, "too many resolved work orders");
                done[doneCount++] = w;
            } else {
                fail(lineNo, "a work order must be marked O or R");
            }
        } else if (tag == "NEXTID") {
            if (n < 2) fail(lineNo, "the line is incomplete");
            nextId = toInt(f[1], lineNo);
        } else {
            fail(lineNo, "unknown entry '" + tag + "'");
        }
    }
    maintenance.restore(open, openCount, done, doneCount, nextId);
    return true;
}
