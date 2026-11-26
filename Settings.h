#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <stdexcept>
using namespace std;
enum Role { ROLE_VIEWER = 1, ROLE_TECHNICIAN = 2, ROLE_ADMIN = 3 };
class Settings {
    Role role;
    string user;         
    bool notificationsOn;
    int alertPercent;     
    bool privacyMode;    
    bool compactView;    
    int theme;           
public:
    Settings();
    Role getRole() const { return role; }
    string getUser() const { return user; }
    void setUser(const string& name) { user = name; }
    bool notificationsEnabled() const { return notificationsOn; }
    int getAlertPercent() const { return alertPercent; }
    bool isPrivacyMode() const { return privacyMode; }
    bool isCompact() const { return compactView; }
    int getTheme() const { return theme; }
    string roleName() const;
    string themeName() const;
    void requireAdmin() const;   
    void requireStaff() const;   
    void setRole(Role r);                 
    void setNotifications(bool on);       
    void setAlertPercent(int percent);    
    void setPrivacyMode(bool on);          
    void setCompact(bool on);             
    void setTheme(int t);                 
};

#endif
