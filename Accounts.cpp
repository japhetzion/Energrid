#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include "Accounts.h"
using namespace std;
static string toHex(unsigned long long value) {
    ostringstream s;
    s << hex << setw(16) << setfill('0') << value;
    return s.str();
}
static string hashPassword(const string& salt, const string& password) {
    unsigned long long h = 14695981039346656037ULL;
    string input = salt + ":" + password;
    for (int round = 0; round < 2000; round++) {
        for (size_t i = 0; i < input.length(); i++) {
            h ^= (unsigned char)input[i];
            h *= 1099511628211ULL;
        }
        input = toHex(h) + salt + password;     
    }
    return toHex(h);
}
static string makeSalt() {
    static unsigned long long counter = 0;
    counter++;
    unsigned long long seed = (unsigned long long)time(NULL) * 6364136223846793005ULL +
                              (unsigned long long)clock() * 1442695040888963407ULL +
                              counter * 2862933555777941757ULL;
    seed ^= seed >> 29;
    seed *= 0xBF58476D1CE4E5B9ULL;
    seed ^= seed >> 32;
    return toHex(seed);
}
static void checkName(const string& name) {
    if (name.length() < 3 || name.length() > 14)
        throw invalid_argument("The username must have 3 to 14 characters.");
    for (size_t i = 0; i < name.length(); i++) {
        char c = name[i];
        bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
        if (!ok)
            throw invalid_argument("The username may only contain letters, digits and _ .");
    }
}

static void checkPassword(const string& password) {
    if (password.length() < 6)
        throw invalid_argument("The password must have at least 6 characters.");
    if (password.length() > 30)
        throw invalid_argument("The password may have at most 30 characters.");
}
AccountManager::AccountManager(const string& filePath) : count(0), path(filePath) {
    for (int i = 0; i < SIZE; i++)
        table[i] = NULL;
}

AccountManager::~AccountManager() {
    clearTable();
}

void AccountManager::clearTable() {
    for (int i = 0; i < SIZE; i++) {
        while (table[i] != NULL) {
            UNode* temp = table[i];
            table[i] = table[i]->next;
            delete temp;
        }
    }
    count = 0;
}

int AccountManager::bucket(const string& name) const {
    int h = 0;
    for (size_t i = 0; i < name.length(); i++)
        h = (h * 31 + name[i]) % SIZE;
    if (h < 0)
        h += SIZE;
    return h;
}
AccountManager::UNode* AccountManager::findNode(const string& name) const {
    for (UNode* cur = table[bucket(name)]; cur != NULL; cur = cur->next)
        if (cur->user.name == name)
            return cur;
    return NULL;
}
void AccountManager::insertNode(const User& user) {
    UNode* node = new UNode;
    node->user = user;
    int b = bucket(user.name);
    node->next = table[b];
    table[b] = node;
    count++;
}
string AccountManager::normalize(const string& name) {
    size_t first = name.find_first_not_of(" \t\r");
    if (first == string::npos)
        return "";
    size_t last = name.find_last_not_of(" \t\r");
    string result = name.substr(first, last - first + 1);
    for (size_t i = 0; i < result.length(); i++)
        if (result[i] >= 'A' && result[i] <= 'Z')
            result[i] = (char)(result[i] - 'A' + 'a');
    return result;
}
User AccountManager::makeUser(const string& name, const string& password, Role role) const {
    User u;
    u.name = name;
    u.salt = makeSalt();
    u.hash = hashPassword(u.salt, password);
    u.role = role;
    return u;
}
void AccountManager::saveToFile() const {
    string tempPath = path + ".tmp";
    ofstream out(tempPath.c_str());
    if (!out)
        throw runtime_error("Could not write the file " + tempPath);
    out << "ENERGRID-USERS|1\n";
    for (int i = 0; i < SIZE; i++)
        for (UNode* cur = table[i]; cur != NULL; cur = cur->next)
            out << "USER|" << cur->user.name << "|" << cur->user.salt << "|"
                << cur->user.hash << "|" << (int)cur->user.role << "\n";
    out.close();
    if (out.fail())
        throw runtime_error("Writing the users file failed.");
    std::remove(path.c_str());
    if (std::rename(tempPath.c_str(), path.c_str()) != 0)
        throw runtime_error("Could not replace " + path);
}

bool AccountManager::load() {
    ifstream in(path.c_str());
    if (!in)
        return false;                                 

    clearTable();
    string line;
    int lineNo = 0;
    while (getline(in, line)) {
        lineNo++;
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            continue;
        if (lineNo == 1) {
            if (line != "ENERGRID-USERS|1") {
                clearTable();
                throw runtime_error("The users file is not valid (line 1).");
            }
            continue;
        }
        string f[5];
        int n = 0;
        size_t start = 0;
        while (n < 5) {
            size_t bar = line.find('|', start);
            if (bar == string::npos) {
                f[n++] = line.substr(start);
                break;
            }
            f[n++] = line.substr(start, bar - start);
            start = bar + 1;
        }
        int role = atoi(f[4].c_str());
        if (n < 5 || f[0] != "USER" || f[1].empty() || f[2].empty() || f[3].empty() ||
            role < 1 || role > 3 || findNode(f[1]) != NULL) {
            clearTable();
            throw runtime_error("The users file is damaged (line " + to_string(lineNo) + ").");
        }
        User u;
        u.name = f[1];
        u.salt = f[2];
        u.hash = f[3];
        u.role = (Role)role;
        insertNode(u);
    }
    if (adminCount() == 0) {
        clearTable();
        throw runtime_error("The users file has no Admin account.");
    }
    return true;
}
void AccountManager::createDefaults() {
    clearTable();
    insertNode(makeUser("admin", "admin123", ROLE_ADMIN));
    insertNode(makeUser("user", "user123", ROLE_TECHNICIAN));
    insertNode(makeUser("viewer", "viewer123", ROLE_VIEWER));
    saveToFile();
}
bool AccountManager::check(const string& name, const string& password, Role& role) const {
    UNode* node = findNode(normalize(name));
    if (node == NULL)
        return false;
    if (hashPassword(node->user.salt, password) != node->user.hash)
        return false;
    role = node->user.role;
    return true;
}

bool AccountManager::exists(const string& name) const {
    return findNode(normalize(name)) != NULL;
}

int AccountManager::adminCount() const {
    int admins = 0;
    for (int i = 0; i < SIZE; i++)
        for (UNode* cur = table[i]; cur != NULL; cur = cur->next)
            if (cur->user.role == ROLE_ADMIN)
                admins++;
    return admins;
}

void AccountManager::addUser(const string& name, const string& password, Role role) {
    string n = normalize(name);
    checkName(n);
    checkPassword(password);
    if (findNode(n) != NULL)
        throw invalid_argument("A user with this name already exists.");
    if (count >= MAX_USERS)
        throw runtime_error("The user limit is reached.");
    insertNode(makeUser(n, password, role));
    saveToFile();
}

void AccountManager::removeUser(const string& name) {
    string n = normalize(name);
    int b = bucket(n);
    UNode* prev = NULL;
    UNode* cur = table[b];
    while (cur != NULL && cur->user.name != n) {
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL)
        throw runtime_error("There is no user called '" + n + "'.");
    if (cur->user.role == ROLE_ADMIN && adminCount() <= 1)
        throw runtime_error("The last Admin cannot be removed.");
    if (prev == NULL)
        table[b] = cur->next;
    else
        prev->next = cur->next;
    delete cur;
    count--;
    saveToFile();
}

void AccountManager::setPassword(const string& name, const string& newPassword) {
    UNode* node = findNode(normalize(name));
    if (node == NULL)
        throw runtime_error("There is no user called '" + normalize(name) + "'.");
    checkPassword(newPassword);
    node->user.salt = makeSalt();                       
    node->user.hash = hashPassword(node->user.salt, newPassword);
    saveToFile();
}
void AccountManager::setRole(const string& name, Role role) {
    UNode* node = findNode(normalize(name));
    if (node == NULL)
        throw runtime_error("There is no user called '" + normalize(name) + "'.");
    if (node->user.role == ROLE_ADMIN && role != ROLE_ADMIN && adminCount() <= 1)
        throw runtime_error("The last Admin cannot lose the Admin role.");
    node->user.role = role;
    saveToFile();
}
int AccountManager::listUsers(User out[], int maxCount) const {
    int n = 0;
    for (int i = 0; i < SIZE; i++)
        for (UNode* cur = table[i]; cur != NULL && n < maxCount; cur = cur->next)
            out[n++] = cur->user;
    for (int i = 0; i < n - 1; i++) {
        int smallest = i;
        for (int j = i + 1; j < n; j++)
            if (out[j].name < out[smallest].name)
                smallest = j;
        User temp = out[i];
        out[i] = out[smallest];
        out[smallest] = temp;
    }
    return n;
}
