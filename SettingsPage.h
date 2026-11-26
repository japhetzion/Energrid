#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H
#include "Page.h"
#include "Accounts.h"
class SettingsPage : public Page {
    AccountManager& accounts;
    void showMenu() const;
    void togglePrivacy();
    void accountsMenu();
    void notificationsMenu();
    void toggleAppearance();
    void chooseTheme();
    void changeMyPassword();
    void listUsers() const;
    void addUser();
    void removeUser();
    void resetPassword();
    void changeRole();

public:
    SettingsPage(CampusData& d, Settings& s, AccountManager& a) : Page(d, s), accounts(a) {}
    string name() const { return "Settings"; }
    void run();
};

#endif
