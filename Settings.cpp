#include "Settings.h"
using namespace std;
Settings::Settings()
    : role(ROLE_ADMIN), notificationsOn(true), alertPercent(100),
      privacyMode(false), compactView(false), theme(1) {}
string Settings::roleName() const {
    if (role == ROLE_VIEWER) return "Viewer";
    if (role == ROLE_TECHNICIAN) return "Technician";
    return "Admin";
}
string Settings::themeName() const {
    if (theme == 1) return "Ocean (cyan)";
    if (theme == 2) return "Forest (green)";
    return "Violet";
}
void Settings::requireAdmin() const {
    if (role != ROLE_ADMIN)
        throw runtime_error("Permission denied: this needs the Admin role "
                            "(change it in Settings > Permissions).");
}
void Settings::requireStaff() const {
    if (role == ROLE_VIEWER)
        throw runtime_error("Permission denied: Viewers cannot do this. "
                            "Switch to Technician or Admin in Settings > Permissions.");
}
void Settings::setRole(Role r) {
    role = r;
}
void Settings::setNotifications(bool on) {
    if (on == notificationsOn)
        return;
    requireAdmin();
    notificationsOn = on;
}
void Settings::setAlertPercent(int percent) {
    if (percent == alertPercent)
        return;
    requireAdmin();
    if (percent < 50 || percent > 150)
        throw out_of_range("Alert trigger must be between 50 and 150 percent.");
    alertPercent = percent;
}
void Settings::setPrivacyMode(bool on) {
    if (on == privacyMode)
        return;
    requireAdmin();
    privacyMode = on;
}
void Settings::setCompact(bool on) {
    compactView = on;
}
void Settings::setTheme(int t) {
    if (t < 1 || t > 3)
        throw invalid_argument("Unknown theme.");
    theme = t;
}
