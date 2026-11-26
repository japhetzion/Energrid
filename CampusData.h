#ifndef CAMPUSDATA_H
#define CAMPUSDATA_H
#include <string>
#include <stdexcept>
#include "Building.h"
#include "Containers.h"
#include "DataStructures.h"
using namespace std;
struct CampusData {
    BuildingList list;         
    BuildingTable table;        
    Queue<string> alerts;       
    Stack<string> undoStack;    
    PowerGrid grid;            
    void addBuilding(Building* b) {
        try {
            if (list.getSize() >= MAX_BUILDINGS)
                throw runtime_error("Building limit reached.");
            if (b->getName().find('|') != string::npos)     
                throw invalid_argument("The character | is not allowed in names.");
            table.insert(b);   
        } catch (...) {
            delete b;
            throw;
        }
        list.add(b);
    }
    void removeBuilding(const string& name) {
        table.remove(name);
        list.remove(name);
    }
    void clearAll() {
        while (list.getHead() != NULL)
            removeBuilding(list.getHead()->data->getName());
        alerts.clear();
        while (!undoStack.isEmpty())
            undoStack.pop();
        grid.clear();
    }
    bool raiseAlertIfHigh(Building* b, int day, double kwh, int percent, bool queueIt) {
        double trigger = b->getLimit() * percent / 100.0;
        if (kwh <= trigger)
            return false;
        if (queueIt)
            alerts.enqueue(b->getName() + ": Day " + to_string(day) + " used " +
                           to_string((int)kwh) + " kWh (limit " +
                           to_string((int)b->getLimit()) + ")");
        return true;
    }
};

#endif
