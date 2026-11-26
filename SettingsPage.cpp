#include <iostream>
#include "SettingsPage.h"
using namespace std;
static string onOff(bool on) {
    return on ? paint("ON", GREEN) : paint("OFF", GREY);
}
void SettingsPage::showMenu() const {
    header("SETTINGS");
    drawRow(padRight("Logged in as", 22) + WHITE + settings.getUser() + RESET + GREY + "  (" +
            settings.roleName() + ")" + RESET);
    drawRow(padRight("Privacy mode", 22) + onOff(settings.isPrivacyMode()));
    drawRow(padRight("Notifications", 22) + onOff(settings.notificationsEnabled()));
    drawRow(padRight("Alert trigger", 22) + to_string(settings.getAlertPercent()) + "% of the daily limit");
    drawRow(padRight("Compact dashboard", 22) + onOff(settings.isCompact()));
    drawRow(padRight("Theme", 22) + accent() + settings.themeName() + RESET);
    drawMiddle();
    drawMenuItem("1", "Privacy (hide names in reports)");
    drawMenuItem("2", "Permissions (accounts and roles)");
    drawMenuItem("3", "Notifications");
    drawMenuItem("4", "Appearance (compact dashboard)");
    drawMenuItem("5", "Themes");
    drawRow();
    drawMenuItem("0", "Back");
    drawBottom();
}
void SettingsPage::togglePrivacy() {
    settings.setPrivacyMode(!settings.isPrivacyMode());      
    showSuccess("Privacy mode is now " + string(settings.isPrivacyMode() ? "ON." : "OFF."));
}
static Role roleFromNumber(int r) {
    if (r < 1 || r > 3)
        throw invalid_argument("Unknown role. Use 1 (Viewer), 2 (Technician) or 3 (Admin).");
    return (Role)r;
}
static string roleText(Role r) {
    if (r == ROLE_ADMIN) return paint("Admin", AMBER);
    if (r == ROLE_TECHNICIAN) return paint("Technician", GREEN);
    return paint("Viewer", GREY);
}
void SettingsPage::changeMyPassword() {
    inputScreen("CHANGE MY PASSWORD", "Type your current password, then the new one twice.");
    string old = readPassword("Current password");
    Role role;
    if (!accounts.check(settings.getUser(), old, role))
        throw runtime_error("The current password is wrong.");
    string first = readPassword("New password (at least 6 characters)");
    string second = readPassword("Repeat the new password");
    if (first != second)
        throw runtime_error("The two passwords are not the same. Nothing was changed.");
    accounts.setPassword(settings.getUser(), first);       
    showSuccess("Your password was changed.");
}
void SettingsPage::listUsers() const {
    settings.requireAdmin();
    User users[MAX_USERS];
    int n = accounts.listUsers(users, MAX_USERS);
    header("USERS");
    drawRow(GREY + padRight("Username", 18) + "Role" + RESET);
    for (int i = 0; i < n; i++)
        drawRow(padRight(users[i].name, 18) + roleText(users[i].role) +
                (users[i].name == settings.getUser() ? GREY + "   (you)" + RESET : string("")));
    drawBottom();
}
void SettingsPage::addUser() {
    settings.requireAdmin();
    inputScreen("ADD USER", "Username: 3 to 14 letters, digits or _.   Password: at least 6 characters.\n"
                            "Roles: 1 = Viewer, 2 = Technician, 3 = Admin");
    string name = readText("Username");
    string password = readPassword("Password");
    Role role = roleFromNumber(readInt("Role"));
    accounts.addUser(name, password, role);
    showSuccess("User " + AccountManager::normalize(name) + " was added.");
}
void SettingsPage::removeUser() {
    settings.requireAdmin();
    inputScreen("REMOVE USER", "Type the username to remove.");
    string name = AccountManager::normalize(readText("Username"));
    if (name == settings.getUser())
        throw runtime_error("You cannot remove the account you are logged in with.");
    accounts.removeUser(name);
    showSuccess("User " + name + " was removed.");
}
void SettingsPage::resetPassword() {
    settings.requireAdmin();
    inputScreen("RESET A PASSWORD", "Type the username and a new password for that user.");
    string name = readText("Username");
    if (!accounts.exists(name))
        throw runtime_error("There is no user called '" + AccountManager::normalize(name) + "'.");
    string password = readPassword("New password (at least 6 characters)");
    accounts.setPassword(name, password);
    showSuccess("The password of " + AccountManager::normalize(name) + " was changed.");
}
void SettingsPage::changeRole() {
    settings.requireAdmin();
    inputScreen("CHANGE A ROLE", "Roles: 1 = Viewer, 2 = Technician, 3 = Admin");
    string name = AccountManager::normalize(readText("Username"));
    if (name == settings.getUser())
        throw runtime_error("You cannot change your own role.");
    Role role = roleFromNumber(readInt("New role"));
    accounts.setRole(name, role);
    showSuccess(name + " is now " + (role == ROLE_ADMIN ? "an Admin" :
                                     role == ROLE_TECHNICIAN ? "a Technician" : "a Viewer") + ".");
}
void SettingsPage::accountsMenu() {
    int choice;
    do {
        header("PERMISSIONS: ACCOUNTS AND ROLES");
        drawRow(GREY + "Logged in as " + RESET + WHITE + settings.getUser() + RESET + GREY + " (" +
                settings.roleName() + ")" + RESET);
        drawMiddle("WHAT EACH ROLE CAN DO");
        drawWrapped("Viewer: look at the dashboard and reports, report issues.");
        drawWrapped("Technician: Viewer + enter readings, run the live feed, respond to work orders.");
        drawWrapped("Admin: Technician + manage buildings, the power network, users and settings.");
        drawMiddle();
        drawMenuItem("1", "Change my password");
        drawMenuItem("2", "List users            (Admin)");
        drawMenuItem("3", "Add a user            (Admin)");
        drawMenuItem("4", "Remove a user         (Admin)");
        drawMenuItem("5", "Reset a password      (Admin)");
        drawMenuItem("6", "Change a user's role  (Admin)");
        drawRow();
        drawMenuItem("0", "Back");
        drawBottom();
        choice = readInt("Choose an option");
        if (choice == 0)
            break;
        try {
            switch (choice) {
            case 1: changeMyPassword(); break;
            case 2: listUsers(); break;
            case 3: addUser(); break;
            case 4: removeUser(); break;
            case 5: resetPassword(); break;
            case 6: changeRole(); break;
            default: showError("Please choose a number from the menu.");
            }
        } catch (exception& e) {
            showError(e.what());
        }
        pauseScreen();
    } while (true);
}

void SettingsPage::notificationsMenu() {
    inputScreen("NOTIFICATIONS", "1 = turn alerts on or off     2 = change the alert trigger percentage");
    int c = readInt("Choice (0 = back)");
    if (c == 0)
        return;
    if (c == 1) {
        settings.setNotifications(!settings.notificationsEnabled());   
        showSuccess("Notifications are now " + string(settings.notificationsEnabled() ? "ON." : "OFF."));
    } else if (c == 2) {
        int p = readInt("New trigger in percent (50 to 150)");
        settings.setAlertPercent(p);                                  
        showSuccess("Alerts now trigger above " + to_string(p) + "% of the daily limit.");
    } else {
        throw invalid_argument("Invalid choice.");
    }
}
void SettingsPage::toggleAppearance() {
    settings.setCompact(!settings.isCompact());
    showSuccess("Compact dashboard is now " + string(settings.isCompact() ? "ON." : "OFF."));
}
void SettingsPage::chooseTheme() {
    inputScreen("THEMES", "1 = Ocean (cyan)     2 = Forest (green)     3 = Violet");
    int t = readInt("Theme (0 = keep)");
    if (t == 0)
        return;
    settings.setTheme(t);
    setUiTheme(settings.getTheme());      
    showSuccess("Theme changed to " + settings.themeName() + ".");
}
void SettingsPage::run() {
    int choice;
    do {
        showMenu();
        choice = readInt("Choose an option");
        if (choice == 0)
            break;
        try {
            switch (choice) {
            case 1: togglePrivacy(); break;
            case 2: accountsMenu(); break;
            case 3: notificationsMenu(); break;
            case 4: toggleAppearance(); break;
            case 5: chooseTheme(); break;
            default: showError("Please choose a number from the menu.");
            }
        } catch (exception& e) {
            showError(e.what());
        }
        pauseScreen();
    } while (true);
}
