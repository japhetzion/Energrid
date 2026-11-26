#ifndef ACCOUNTS_H
#define ACCOUNTS_H
#include <string>
#include <stdexcept>
#include <cstddef>
#include "Settings.h"
using namespace std;
const string USERS_FILE = "energrid_users.txt";
const int MAX_USERS = 50;
struct User {
    string name;
    string salt;
    string hash;
    Role role;
    User() : role(ROLE_VIEWER) {}
};
class AccountManager {
    struct UNode {
        User user;
        UNode* next;
    };
    static const int SIZE = 11;
    UNode* table[SIZE];      
    int count;
    string path;
    int bucket(const string& name) const;
    UNode* findNode(const string& name) const;
    void insertNode(const User& user);
    void clearTable();
    void saveToFile() const;
    User makeUser(const string& name, const string& password, Role role) const;
    AccountManager(const AccountManager&);
    AccountManager& operator=(const AccountManager&);
public:
    AccountManager(const string& filePath);
    ~AccountManager();
    static string normalize(const string& name);     
    bool load();
    void createDefaults();
    bool check(const string& name, const string& password, Role& role) const;
    bool exists(const string& name) const;
    int userCount() const { return count; }
    int adminCount() const;
    void addUser(const string& name, const string& password, Role role);
    void removeUser(const string& name);
    void setPassword(const string& name, const string& newPassword);
    void setRole(const string& name, Role role);
    int listUsers(User out[], int maxCount) const;
};

#endif
