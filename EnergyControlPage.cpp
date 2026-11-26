#include <iostream>
#include <ctime>
#include "EnergyControlPage.h"
using namespace std;
const int NAME_WIDTH = 14;
void EnergyControlPage::addBuilding() {
    settings.requireAdmin();
    inputScreen("ADD BUILDING", "Building types:  1 = Academic Block,  2 = Hostel,  3 = Lab");
    int type = readInt("Type");
    if (type < 1 || type > 3)
        throw invalid_argument("Unknown building type.");
    string name = readText("Name (up to 14 characters)");
    if (name.empty())
        throw invalid_argument("Please enter a building name.");
    if ((int)name.length() > NAME_WIDTH)
        throw invalid_argument("The name must be 14 characters or fewer.");
    Building* b;
    if (type == 1) b = new AcademicBlock(name);
    else if (type == 2) b = new Hostel(name);
    else b = new Lab(name);
    data.addBuilding(b);      
    showSuccess(name + " was added.");
}

void EnergyControlPage::removeBuilding() {
    settings.requireAdmin();
    inputScreen("REMOVE BUILDING", "Type the exact name of the building to remove.");
    string name = readText("Name");
    data.removeBuilding(name);
    showSuccess(name + " was removed.");
}
void EnergyControlPage::listBuildings() const {
    if (data.list.getSize() == 0)
        throw runtime_error("No buildings added yet.");
    BuildingTree tree;
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next)
        tree.insert(cur->data);
    Building* sorted[MAX_BUILDINGS];
    int n = tree.sortedList(sorted, MAX_BUILDINGS);

    header("BUILDINGS (A to Z)");
    drawRow(GREY + padRight("Building", NAME_WIDTH) + padRight("Type", 16) +
            padLeft("Limit", 7) + padLeft("Days", 6) + "  Priority" + RESET);
    for (int i = 0; i < n; i++) {
        Building* b = sorted[i];
        drawRow(padRight(b->getName(), NAME_WIDTH) + padRight(b->type(), 16) +
                padLeft(num1(b->getLimit()), 7) + padLeft(to_string(b->getCount()), 6) +
                "  " + statusText(b->priority()));
    }
    drawBottom();
}

void EnergyControlPage::compareBuildings() const {
    inputScreen("COMPARE BUILDINGS", "Type the names of the two buildings to compare.");
    Building* a = data.table.find(readText("First building"));
    Building* b = data.table.find(readText("Second building"));

    header("COMPARISON");
    drawRow(padRight(a->getName(), NAME_WIDTH) + padLeft(num1(a->total()), 10) + " kWh");
    drawRow(padRight(b->getName(), NAME_WIDTH) + padLeft(num1(b->total()), 10) + " kWh");
    drawMiddle();
    if (*a > *b)                             
        drawRow(WHITE + a->getName() + RESET + " uses more energy.");
    else
        drawRow(WHITE + b->getName() + RESET + " uses more energy (or they are equal).");
    drawRow(GREY + "Combined: " + RESET + num1(*a + *b) + " kWh");  
    drawBottom();
}

void EnergyControlPage::showTips() const {
    inputScreen("SAVING TIPS", "Type the name of the building.");
    Building* b = data.table.find(readText("Building"));
    header("SAVING TIPS");
    drawRow(BOLD + b->getName() + RESET + GREY + "  (" + b->type() + ")" + RESET);
    drawMiddle();
    drawWrapped(b->tips(), GREEN);
    drawBottom();
}
void EnergyControlPage::enterReading() {
    settings.requireStaff();
    inputScreen("LIVE READING", "Type the building name, then today's consumption in kWh.");
    Building* b = data.table.find(readText("Building"));
    double kwh = readDouble("Consumption (kWh) for day " + to_string(b->getCount() + 1));

    b->addReading(kwh);
    data.undoStack.push(b->getName());            

    bool high = data.raiseAlertIfHigh(b, b->getCount(), kwh,
                                      settings.getAlertPercent(),
                                      settings.notificationsEnabled());
    showSuccess("Reading saved for " + b->getName() + ".");
    if (high && settings.notificationsEnabled()) {
        showAlert(b->getName() + " used " + num1(kwh) + " kWh, above its trigger of " +
                  num1(b->getLimit() * settings.getAlertPercent() / 100.0) + " kWh.");
        cout << "\n";
        drawTop("SUGGESTIONS");
        drawWrapped(b->tips(), GREEN);
        drawBottom();
    } else if (high) {
        cout << "\n  " << GREY << "Above the limit, but notifications are OFF, so no alert was raised."
             << RESET << "\n";
    } else {
        cout << "\n  " << GREEN << "Usage is within the limit." << RESET << "\n";
    }
}
void EnergyControlPage::undoReading() {
    settings.requireStaff();
    if (data.undoStack.isEmpty())
        throw runtime_error("There is no reading to undo.");
    string name = data.undoStack.pop();
    Building* b = data.table.find(name);
    double removed = b->removeLastReading();
    showSuccess("Undone: the reading of " + num1(removed) + " kWh for " + name + " was removed.");
}

void EnergyControlPage::correctReading() {
    settings.requireStaff();
    inputScreen("CORRECT A READING", "Type the building, the day number and the right value.");
    Building* b = data.table.find(readText("Building"));
    int day = readInt("Day number (1 to " + to_string(b->getCount()) + ")");
    double kwh = readDouble("Correct value (kWh)");
    b->addReading(day, kwh);                      
    showSuccess("Day " + to_string(day) + " of " + b->getName() + " now reads " + num1(kwh) + " kWh.");
}
const int LOG_SIZE = 5;
void EnergyControlPage::drawLive(const LiveMonitor& monitor, const string log[], int logSize,
                                 const string& footer) const {
    cursorHome();
    header("LIVE MONITOR", false);
    string hh = (monitor.getHour() < 10 ? "0" : "") + to_string(monitor.getHour()) + ":00";
    drawRowSplit(GREY + "Source: " + RESET + monitor.sourceName(),
                 GREY + "Day " + to_string(monitor.getDay()) + "  " + DOT + "  " + RESET + BOLD + hh + RESET);
    drawMiddle("LOAD NOW (kW)");
    drawRow(GREY + padRight("Building", NAME_WIDTH) + padLeft("Now", 7) + padLeft("Peak", 7) +
            padLeft("Today", 8) + "  " + padRight("Trend", 10) + "Status" + RESET);
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next) {
        Building* b = cur->data;
        if (b->live.historyCount == 0) {
            drawRow(padRight(b->getName(), NAME_WIDTH) + paint("waiting for data...", GREY));
            continue;
        }
        double ratio = b->live.kw / b->peakLimit();
        string status = "OK";
        if (ratio > 1.0) status = "HIGH";
        else if (ratio > 0.8) status = "MEDIUM";
        drawRow(padRight(b->getName(), NAME_WIDTH) + padLeft(num1(b->live.kw), 7) +
                padLeft(num1(b->peakLimit()), 7) + padLeft(num1(b->live.todayKwh), 8) + "  " +
                accent() + sparkline(b->live.history, b->live.historyCount, b->peakLimit() * 1.2, 9) +
                RESET + " " + statusText(status));
    }
    drawMiddle("EVENTS (latest)");
    for (int i = 0; i < logSize; i++) {
        string e = log[i];
        if (e.length() > 56)
            e = e.substr(0, 53) + "...";
        if (e.compare(0, 5, "ALERT") == 0)
            drawRow(paint(e, AMBER));
        else
            drawRow(e);
    }
    drawBottom();
    cout << "\n  " << GREY << footer << RESET << "\n";
    clearBelow();
}
void EnergyControlPage::runLiveMonitor() {
    settings.requireStaff();
    if (data.list.getSize() == 0)
        throw runtime_error("No buildings added yet.");

    inputScreen("LIVE DATA FEED",
                "Where do the live readings come from?\n"
                "1 = Simulated feed (made up by the program, good for demos)\n"
                "2 = File feed: reads live_feed.csv from the project folder, one line per "
                "building, like  Block A,18.5   (save the file while it runs to change the data)");
    int choice = readInt("Source");
    if (choice < 1 || choice > 2)
        throw invalid_argument("Unknown source.");
    double seconds = readDouble("Seconds per simulated hour (1 is a good speed, 0 = fastest)");
    if (seconds < 0 || seconds > 10)
        throw out_of_range("Choose between 0 and 10 seconds.");
    int hours = readInt("Hours to run (0 = until you press a key)");
    if (hours < 0)
        throw out_of_range("The number of hours cannot be negative.");

    DataSource* source;
    if (choice == 1)
        source = new SimulatedSource((unsigned int)time(NULL));
    else
        source = new FileSource("live_feed.csv");
    LiveMonitor monitor(data, settings, source);      
    string log[LOG_SIZE];
    clearScreen();
    int ticks = 0;
    while (true) {
        string events[MAX_BUILDINGS * 2];
        int n = monitor.tick(events, MAX_BUILDINGS * 2);
        for (int i = 0; i < n; i++) {                
            for (int k = 1; k < LOG_SIZE; k++)
                log[k - 1] = log[k];
            log[LOG_SIZE - 1] = events[i];
        }
        ticks++;
        string footer = hours > 0
            ? "Running: hour " + to_string(ticks) + " of " + to_string(hours)
            : "Press any key to stop the live monitor...";
        drawLive(monitor, log, LOG_SIZE, footer);

        if (hours > 0 && ticks >= hours)
            break;
        sleepMs((int)(seconds * 1000));
        if (keyPressed())
            break;
    }
    monitor.discardPartialDay();
    showSuccess("Live monitor stopped after " + to_string(ticks) + " simulated hour(s). "
                "Completed days were saved as daily readings.");
}
void EnergyControlPage::showPriorityRanking() const {
    if (data.list.getSize() == 0)
        throw runtime_error("No buildings added yet.");
    PriorityQueue<BuildingRank, MAX_BUILDINGS> queue;
    for (BNode* cur = data.list.getHead(); cur != NULL; cur = cur->next)
        queue.push(BuildingRank(cur->data));
    BuildingRank ranked[MAX_BUILDINGS];
    int n = queue.toSortedArray(ranked, MAX_BUILDINGS);
    header("PRIORITY RANKING (most urgent first)");
    drawRow(GREY + padRight("#", 4) + padRight("Building", NAME_WIDTH) + padRight("% of limit", 24) +
            "Priority" + RESET);
    for (int i = 0; i < n; i++) {
        Building* b = ranked[i].building;
        string percent = b->getCount() > 0 ? num1(ranked[i].ratio * 100) + "%" : "-";
        drawRow(padRight(to_string(i + 1), 4) + padRight(b->getName(), NAME_WIDTH) +
                statusBar(ranked[i].ratio * 100, 10) + " " + padLeft(percent, 8) + "  " +
                statusText(b->priority()));
    }
    if (n > 0 && ranked[0].building->priority() == "HIGH") {
        drawMiddle("TOP PRIORITY: " + ranked[0].building->getName());
        drawWrapped(ranked[0].building->tips(), GREEN);
    }
    drawBottom();
}
void EnergyControlPage::showNetwork() const {
    string lines[100];
    int n = data.grid.describe(lines, 100);
    if (n == 0)
        throw runtime_error("The power network is empty. Add links with option 12.");
    header("POWER NETWORK");
    drawRow(GREY + to_string(data.grid.getNodeCount()) + " points, " +
            to_string(data.grid.getLinkCount()) + " links.  An arrow shows what a point feeds." + RESET);
    drawMiddle();
    for (int i = 0; i < n; i++)
        drawWrapped(lines[i]);
    drawBottom();
}

void EnergyControlPage::addNetworkLink() {
    settings.requireAdmin();
    inputScreen("ADD POWER LINK", "Type what feeds what, for example  Main Substation  feeds  Feeder 1.");
    string from = readText("Power comes from");
    string to = readText("and goes to");
    data.grid.addLink(from, to);
    showSuccess(from + " now feeds " + to + ".");
}
void EnergyControlPage::showMenu() const {
    header("ENERGY CONTROL");
    drawRow(GREY + "BUILDING MANAGEMENT" + RESET);
    drawMenuItem("1", "Add building");
    drawMenuItem("2", "Remove building");
    drawMenuItem("3", "List buildings (A to Z)");
    drawMenuItem("4", "Compare two buildings");
    drawMenuItem("5", "Energy-saving tips");
    drawRow();
    drawRow(GREY + "LIVE MONITORING" + RESET);
    drawMenuItem("6", "Enter a live reading");
    drawMenuItem("7", "Undo the last reading");
    drawMenuItem("8", "Correct a reading");
    drawMenuItem("9", "Start the live data feed");
    drawRow();
    drawRow(GREY + "PRIORITY RANKING" + RESET);
    drawMenuItem("10", "Show priority ranking");
    drawRow();
    drawRow(GREY + "POWER NETWORK" + RESET);
    drawMenuItem("11", "View power network");
    drawMenuItem("12", "Add a power link");
    drawRow();
    drawMenuItem("0", "Back");
    drawBottom();
}

void EnergyControlPage::run() {
    int choice;
    do {
        showMenu();
        choice = readInt("Choose an option");
        if (choice == 0)
            break;
        try {
            switch (choice) {
            case 1: addBuilding(); break;
            case 2: removeBuilding(); break;
            case 3: listBuildings(); break;
            case 4: compareBuildings(); break;
            case 5: showTips(); break;
            case 6: enterReading(); break;
            case 7: undoReading(); break;
            case 8: correctReading(); break;
            case 9: runLiveMonitor(); break;
            case 10: showPriorityRanking(); break;
            case 11: showNetwork(); break;
            case 12: addNetworkLink(); break;
            default: showError("Please choose a number from the menu.");
            }
        } catch (exception& e) {
            showError(e.what());
        }
        pauseScreen();
    } while (true);
}
