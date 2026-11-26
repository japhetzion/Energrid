#ifndef STORAGE_H
#define STORAGE_H
#include <string>
#include "CampusData.h"
#include "Settings.h"
#include "MaintenancePage.h"
using namespace std;
const string SAVE_FILE = "energrid_data.txt";
class Storage {
public:
    static void save(const string& path, const CampusData& data, const Settings& settings,
                     const MaintenancePage& maintenance);
    static bool load(const string& path, CampusData& data, Settings& settings,
                     MaintenancePage& maintenance);
};

#endif
