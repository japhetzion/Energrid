#ifndef PAGE_H
#define PAGE_H
#include <string>
#include "CampusData.h"
#include "Settings.h"
#include "Ui.h"
using namespace std;

class Page {
protected:
    CampusData& data;
    Settings& settings;
    void header(const string& pageTitle, bool clear = true) const {
        if (clear)
            clearScreen();
        drawTop(pageTitle);
        string alertText = "Alerts: " + to_string(data.alerts.size());
        string who = settings.getUser().empty()
            ? "Role: " + settings.roleName()
            : "User: " + settings.getUser() + " (" + settings.roleName() + ")";
        drawRowSplit(paint(who, GREY),
                     paint(alertText, data.alerts.size() > 0 ? AMBER : GREY));
        drawMiddle();
    }
    void inputScreen(const string& pageTitle, const string& hint) const {
        header(pageTitle);
        drawWrapped(hint);
        drawBottom();
    }

public:
    Page(CampusData& d, Settings& s) : data(d), settings(s) {}
    virtual ~Page() {}

    virtual string name() const = 0;   
    virtual void run() = 0;             
};

#endif
