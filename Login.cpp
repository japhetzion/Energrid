#include <iostream>
#include "Login.h"
#include "Ui.h"
using namespace std;
bool showLogin(AccountManager& accounts, Settings& settings, const string& notice) {
    const int MAX_ATTEMPTS = 3;
    clearScreen();
    drawTop("ENERGRID  " + DOT + "  LOGIN");
    drawRow();
    drawCentered(WHITE + "Please log in to continue" + RESET);
    drawRow();
    drawRow(GREY + "Type 0 as the username to quit." + RESET);
    drawBottom();
    if (!notice.empty()) {
        cout << "\n";
        drawTop("NOTICE");
        drawWrapped(notice, AMBER);
        drawBottom();
    }
    for (int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
        string name = readText("Username");
        if (name == "0")
            return false;
        string password = readPassword("Password");
        Role role;
        if (accounts.check(name, password, role)) {
            settings.setRole(role);
            settings.setUser(AccountManager::normalize(name));
            showSuccess("Welcome, " + settings.getUser() + ". You are logged in as " +
                        settings.roleName() + ".");
            pauseScreen();
            return true;
        }
        showError("Wrong username or password (attempt " + to_string(attempt) + " of " +
                  to_string(MAX_ATTEMPTS) + ").");
    }
    showError("Too many failed attempts. The program will close.");
    pauseScreen();
    return false;
}
