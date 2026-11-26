#include <iostream>
#include <cstdio>
#include "Ui.h"
#include "CampusData.h"
#include "Settings.h"
#include "Page.h"
#include "DashboardPage.h"
#include "EnergyControlPage.h"
#include "MaintenancePage.h"
#include "ReportsPage.h"
#include "SettingsPage.h"
#include "Storage.h"
#include "Accounts.h"
#include "Login.h"
using namespace std;
void addSample(CampusData& data, Settings& settings,
               Building* b, const double values[], int n) {
    data.addBuilding(b);   
    for (int i = 0; i < n; i++) {
        b->addReading(values[i]);
        data.raiseAlertIfHigh(b, i + 1, values[i],
                              settings.getAlertPercent(),
                              settings.notificationsEnabled());
    }
}
void loadSampleData(CampusData& data, Settings& settings) {
    settings.requireAdmin();
    const double blockA[]  = {330, 345, 360, 372, 395, 410, 425, 440};
    const double hostelB[] = {220, 215, 230, 240, 255, 260, 270, 280};
    const double labC[]    = {480, 470, 465, 450, 455, 440, 430, 425};
    const double library[] = {200, 205, 198, 202, 200, 204, 199, 201};
    addSample(data, settings, new AcademicBlock("Block A"), blockA, 8);
    addSample(data, settings, new Hostel("Hostel B"), hostelB, 8);
    addSample(data, settings, new Lab("Lab C"), labC, 8);
    addSample(data, settings, new AcademicBlock("Library"), library, 8);
    data.grid.addLink("Main Substation", "Feeder 1");
    data.grid.addLink("Main Substation", "Feeder 2");
    data.grid.addLink("Feeder 1", "Block A");
    data.grid.addLink("Feeder 1", "Library");
    data.grid.addLink("Feeder 2", "Hostel B");
    data.grid.addLink("Feeder 2", "Lab C");
}
bool saveAll(const CampusData& data, const Settings& settings, const MaintenancePage& maintenance) {
    try {
        Storage::save(SAVE_FILE, data, settings, maintenance);
        return true;
    } catch (exception& e) {
        showError(string("Could not save: ") + e.what());
        pauseScreen();
        return false;
    }
}
static void keepDamagedCopy(const string& file) {
    string badFile = file + ".bad";
    std::remove(badFile.c_str());
    std::rename(file.c_str(), badFile.c_str());
}

void showMainMenu(const CampusData& data, const Settings& settings, Page* pages[], int pageCount) {
    clearScreen();
    drawTop("ENERGRID  " + DOT + "  Campus Energy Analyser");
    string alertText = "Alerts: " + to_string(data.alerts.size());
    drawRowSplit(paint("User: " + settings.getUser() + " (" + settings.roleName() + ")", GREY),
                 paint(alertText, data.alerts.size() > 0 ? AMBER : GREY));
    drawMiddle();
    drawRow();
    for (int i = 0; i < pageCount; i++)
        drawMenuItem(to_string(i + 1), pages[i]->name());  
    drawMenuItem("6", "Load sample data");
    drawMenuItem("7", "Save data now");
    drawMenuItem("8", "Erase all data");
    drawMenuItem("9", "Log out");
    drawMenuItem("0", "Exit");
    drawRow();
    drawBottom();
}

int main() {
    initConsole();
    showWelcome();                           
    CampusData data;
    Settings settings;
    AccountManager accounts(USERS_FILE);
    setUiTheme(settings.getTheme());
    const int PAGE_COUNT = 5;
    Page* pages[PAGE_COUNT];
    pages[0] = new DashboardPage(data, settings);
    pages[1] = new EnergyControlPage(data, settings);
    MaintenancePage* maintenance = new MaintenancePage(data, settings);
    pages[2] = maintenance;
    pages[3] = new ReportsPage(data, settings);
    pages[4] = new SettingsPage(data, settings, accounts);
    try {
        if (Storage::load(SAVE_FILE, data, settings, *maintenance)) {
            setUiTheme(settings.getTheme());
            showSuccess("Saved data loaded: " + to_string(data.list.getSize()) + " building(s), " +
                        to_string(data.alerts.size()) + " alert(s).");
            pauseScreen();
        }
    } catch (exception& e) {
        data.clearAll();                       
        maintenance->clearAll();
        keepDamagedCopy(SAVE_FILE);
        showError(string(e.what()) + "  Starting with empty data. The damaged file was kept as " +
                  SAVE_FILE + ".bad.");
        pauseScreen();
    }
    string notice;
    try {
        if (!accounts.load()) {
            accounts.createDefaults();
            notice = "First start: accounts were created.  admin / admin123 (Admin),  "
                     "user / user123 (Technician),  viewer / viewer123 (Viewer).  "
                     "Change these passwords in Settings > Permissions.";
        }
    } catch (exception& e) {
        keepDamagedCopy(USERS_FILE);
        accounts.createDefaults();
        notice = string(e.what()) + "  The damaged file was kept as " + USERS_FILE + ".bad.  "
                 "New starting accounts were created: admin / admin123, user / user123, "
                 "viewer / viewer123.";
    }
    bool quit = false;
    while (!quit) {
        if (!showLogin(accounts, settings, notice))
            break;                            
        notice = "";
        int choice;
        do {
            showMainMenu(data, settings, pages, PAGE_COUNT);
            choice = readInt("Choose an option");
            if (choice >= 1 && choice <= PAGE_COUNT) {
                pages[choice - 1]->run();      
                saveAll(data, settings, *maintenance);   
            } else if (choice == 6) {
                try {
                    loadSampleData(data, settings);
                    showSuccess("Sample data loaded: Block A, Hostel B, Lab C, Library and the power network.");
                    saveAll(data, settings, *maintenance);
                } catch (exception& e) {
                    showError(e.what());
                }
                pauseScreen();
            } else if (choice == 7) {
                if (saveAll(data, settings, *maintenance)) {
                    showSuccess("Data saved to " + SAVE_FILE + ".");
                    pauseScreen();
                }
            } else if (choice == 8) {
                try {
                    settings.requireAdmin();
                    string answer = readText("Type YES to erase all buildings, readings and work orders");
                    if (answer == "YES") {
                        data.clearAll();
                        maintenance->clearAll();
                        saveAll(data, settings, *maintenance);
                        showSuccess("All data was erased.");
                    } else {
                        showError("Cancelled. Nothing was erased.");
                    }
                } catch (exception& e) {
                    showError(e.what());
                }
                pauseScreen();
            } else if (choice != 0 && choice != 9) {
                showError("Please choose a number from the menu.");
                pauseScreen();
            }
        } while (choice != 0 && choice != 9);

        saveAll(data, settings, *maintenance);
        if (choice == 0)
            quit = true;                       
    }
    for (int i = 0; i < PAGE_COUNT; i++)
        delete pages[i];

    clearScreen();
    cout << accent() << "Thank you for using Energrid." << RESET << "\n";
    return 0;
}
