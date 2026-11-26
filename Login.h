#ifndef LOGIN_H
#define LOGIN_H
#include <string>
#include "Accounts.h"
#include "Settings.h"
using namespace std;
bool showLogin(AccountManager& accounts, Settings& settings, const string& notice = "");

#endif
