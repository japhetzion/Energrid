#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <conio.h>
#else
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#endif
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include "Ui.h"
using namespace std;
static const string TL = "\xE2\x95\x94";      
static const string TR = "\xE2\x95\x97";     
static const string BL = "\xE2\x95\x9A";     
static const string BR = "\xE2\x95\x9D";      
static const string ML = "\xE2\x95\xA0";      
static const string MR = "\xE2\x95\xA3";      
static const string HZ = "\xE2\x95\x90";      
static const string VT = "\xE2\x95\x91";     
static const string BLOCK = "\xE2\x96\x88";   
static const string LIGHT = "\xE2\x96\x91";   
static const string CIRCLE = "\xE2\x97\x8F";  
static const string UP = "\xE2\x96\xB2";      
static const string DOWN = "\xE2\x96\xBC";    
static const string SQUARE = "\xE2\x96\xA0";  
static const string CHEVRON = "\xE2\x80\xBA"; 
static const string SPARK[8] = {"\xE2\x96\x81", "\xE2\x96\x82", "\xE2\x96\x83", "\xE2\x96\x84",
                                "\xE2\x96\x85", "\xE2\x96\x86", "\xE2\x96\x87", "\xE2\x96\x88"};
static int currentTheme = 1;
void initConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(out, &mode))
        SetConsoleMode(out, mode | 0x0004);           
#endif
}
void clearScreen() {
    cout << "\033[2J\033[3J\033[H";
}
void cursorHome() {
    cout << "\033[H";
}
void clearBelow() {
    cout << "\033[J" << flush;
}
bool keyPressed() {
#if defined(ENERGRID_NO_PAUSE)
    return false;                      
#elif defined(_WIN32)
    if (_kbhit()) {
        _getch();                     
        return true;
    }
    return false;
#else
    fd_set input;
    FD_ZERO(&input);
    FD_SET(0, &input);
    struct timeval wait = {0, 0};
    if (select(1, &input, NULL, NULL, &wait) > 0) {
        char c;
        if (read(0, &c, 1) < 0)
            return false;
        return true;
    }
    return false;
#endif
}
void sleepMs(int milliseconds) {
    if (milliseconds > 0)
        this_thread::sleep_for(chrono::milliseconds(milliseconds));
}
void setUiTheme(int theme) {
    if (theme >= 1 && theme <= 3)
        currentTheme = theme;
}
string accent() {
    if (currentTheme == 2) return "\033[92m";  
    if (currentTheme == 3) return "\033[95m";  
    return "\033[96m";                          
}
static string repeat(const string& s, int times) {
    string result;
    for (int i = 0; i < times; i++)
        result += s;
    return result;
}
string paint(const string& text, const string& color) {
    return color + text + RESET;
}
int visibleLength(const string& s) {
    int len = 0;
    for (size_t i = 0; i < s.length(); i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == 0x1B) {
            while (i < s.length() && s[i] != 'm')
                i++;
        } else if ((c & 0xC0) != 0x80) {
            len++;
        }
    }
    return len;
}
string padRight(const string& s, int width) {
    int n = width - visibleLength(s);
    if (n > 0)
        return s + string(n, ' ');
    return s;
}
string padLeft(const string& s, int width) {
    int n = width - visibleLength(s);
    if (n > 0)
        return string(n, ' ') + s;
    return s;
}
string num1(double v) {
    ostringstream s;
    s << fixed << setprecision(1) << v;
    return s.str();
}
string statusText(const string& status) {
    string color = GREY;
    if (status == "OVER LIMIT" || status == "HIGH" || status == "High" ||
        status == "Critical" || status == "ACTION NEEDED")
        color = RED;
    else if (status == "NEAR LIMIT" || status == "MEDIUM" || status == "Medium")
        color = AMBER;
    else if (status == "OK" || status == "LOW" || status == "Low" || status == "Normal")
        color = GREEN;
    return color + CIRCLE + " " + status + RESET;
}
string trendText(const string& label) {
    if (label == "RISING") return RED + UP + " RISING" + RESET;
    if (label == "FALLING") return GREEN + DOWN + " FALLING" + RESET;
    return GREY + SQUARE + " STABLE" + RESET;
}
string bar(double percent, int width, const string& color) {
    int filled = (int)(percent / 100 * width + 0.5);
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;
    return color + repeat(BLOCK, filled) + GREY + repeat(LIGHT, width - filled) + RESET;
}
string sparkline(const double values[], int count, double maxValue, int width) {
    int start = count > width ? count - width : 0;
    string result = string(width - (count - start), ' ');
    for (int i = start; i < count; i++) {
        int level = maxValue > 0 ? (int)(values[i] / maxValue * 8) : 0;
        if (level < 0) level = 0;
        if (level > 7) level = 7;
        result += SPARK[level];
    }
    return result;
}

string statusBar(double percent, int width) {
    string color = GREEN;
    if (percent > 100) color = RED;
    else if (percent > 80) color = AMBER;
    return bar(percent, width, color);
}
static void drawEdge(const string& left, const string& right, const string& title) {
    cout << accent() << left;
    if (title.empty()) {
        cout << repeat(HZ, BOX_WIDTH);
    } else {
        cout << HZ << " " << RESET << BOLD << WHITE << title << RESET << accent() << " ";
        cout << repeat(HZ, BOX_WIDTH - visibleLength(title) - 3);
    }
    cout << right << RESET << "\n";
}
void drawTop(const string& title) { drawEdge(TL, TR, title); }
void drawMiddle(const string& title) { drawEdge(ML, MR, title); }
void drawBottom() { drawEdge(BL, BR, ""); }

void drawRow(const string& text) {
    int pad = BOX_WIDTH - 1 - visibleLength(text);
    if (pad < 0) pad = 0;
    cout << accent() << VT << RESET << " " << text << string(pad, ' ')
         << accent() << VT << RESET << "\n";
}
void drawRowSplit(const string& left, const string& right) {
    int gap = BOX_WIDTH - 2 - visibleLength(left) - visibleLength(right);
    if (gap < 1) gap = 1;
    drawRow(left + string(gap, ' ') + right);
}
void drawCentered(const string& text) {
    int total = BOX_WIDTH - visibleLength(text);
    if (total < 0) total = 0;
    int left = total / 2;
    cout << accent() << VT << RESET << string(left, ' ') << text
         << string(total - left, ' ') << accent() << VT << RESET << "\n";
}
void drawWrapped(const string& text, const string& color) {
    const int MAX_WIDTH = 56;
    istringstream lines(text);
    string line;
    while (getline(lines, line)) {
        string current;
        string word;
        istringstream words(line);
        while (words >> word) {
            string attempt = current.empty() ? word : current + " " + word;
            if ((int)attempt.length() > MAX_WIDTH && !current.empty()) {
                drawRow(color.empty() ? current : paint(current, color));
                current = "   " + word;       
            } else {
                current = attempt;
            }
        }
        drawRow(color.empty() ? current : paint(current, color));
    }
}
void drawMenuItem(const string& key, const string& label) {
    drawRow(accent() + "[" + key + "]" + RESET + string(key.length() == 1 ? 2 : 1, ' ') + label);
}
void showSuccess(const string& msg) {
    cout << "\n  " << GREEN << "[ OK ]" << RESET << "  " << msg << "\n";
}
void showError(const string& msg) {
    cout << "\n  " << RED << "[ ERROR ]" << RESET << "  " << msg << "\n";
}

void showAlert(const string& msg) {
    cout << "\n  " << AMBER << "[ ALERT ]" << RESET << "  " << msg << "\n";
}
void pauseScreen(const string& msg) {
    cout << "\n  " << GREY << msg << RESET << flush;
#if defined(ENERGRID_NO_PAUSE)
    cout << "\n";                     
#elif defined(_WIN32)
    _getch();                          
    cout << "\n";
#else
    string dummy;
    getline(cin, dummy);
#endif
}
static const string FONT[6][5] = {
    {"XXXXX", "X....", "XXXX.", "X....", "XXXXX"},  
    {"X...X", "XX..X", "X.X.X", "X..XX", "X...X"},  
    {"XXXX.", "X...X", "XXXX.", "X..X.", "X...X"},   
    {".XXXX", "X....", "X.XXX", "X...X", ".XXXX"},  
    {"XXXXX", "..X..", "..X..", "..X..", "XXXXX"},   
    {"XXXX.", "X...X", "X...X", "X...X", "XXXX."}    
};

void showWelcome() {
    int word[8] = {0, 1, 0, 2, 3, 2, 4, 5};         
    clearScreen();
    cout << "\n";
    drawTop("");
    drawCentered("");
    drawCentered(GREY + "W E L C O M E   T O" + RESET);
    drawCentered("");
    for (int row = 0; row < 5; row++) {
        string line;
        for (int i = 0; i < 8; i++) {
            const string& pattern = FONT[word[i]][row];
            for (size_t c = 0; c < pattern.length(); c++)
                line += (pattern[c] == 'X') ? BLOCK : " ";
            if (i < 7)
                line += " ";
        }
        drawCentered(accent() + BOLD + line + RESET);
    }
    drawCentered("");
    drawCentered(WHITE + "Campus Energy Analyser" + RESET);
    drawCentered(GREY + "Analyse  " + DOT + "  Alert  " + DOT + "  Troubleshoot" + RESET);
    drawCentered("");
    drawBottom();
    pauseScreen("Press any key to continue...");
}
static void showPrompt(const string& prompt) {
    cout << "\n  " << accent() << prompt << " " << CHEVRON << RESET << " " << flush;
}
int readInt(const string& prompt) {
    while (true) {
        showPrompt(prompt);
        int x;
        if (cin >> x) {
            cin.ignore(1000, '\n');
            return x;
        }
        if (cin.eof())
            exit(0);
        cin.clear();
        cin.ignore(1000, '\n');
        showError("Please type a whole number.");
    }
}
double readDouble(const string& prompt) {
    while (true) {
        showPrompt(prompt);
        double x;
        if (cin >> x) {
            cin.ignore(1000, '\n');
            return x;
        }
        if (cin.eof())
            exit(0);
        cin.clear();
        cin.ignore(1000, '\n');
        showError("Please type a number.");
    }
}
string readText(const string& prompt) {
    showPrompt(prompt);
    string text;
    if (!getline(cin, text))
        exit(0);
    size_t first = text.find_first_not_of(" \t\r");
    if (first == string::npos)
        return "";
    size_t last = text.find_last_not_of(" \t\r");
    return text.substr(first, last - first + 1);
}
string readPassword(const string& prompt) {
    showPrompt(prompt);
    string password;
#ifdef _WIN32
    while (true) {
        int c = _getch();
        if (c == '\r' || c == '\n')
            break;
        if (c == 0 || c == 224) {          
            _getch();
            continue;
        }
        if (c == '\b') {
            if (!password.empty()) {
                password.erase(password.size() - 1);
                cout << "\b \b" << flush;
            }
        } else if (c >= 32 && c < 127) {
            password += (char)c;
            cout << '*' << flush;
        }
    }
    cout << "\n";
#else
    termios saved;
    bool hidden = isatty(0) && tcgetattr(0, &saved) == 0;
    if (hidden) {
        termios quiet = saved;
        quiet.c_lflag &= ~ECHO;
        tcsetattr(0, TCSANOW, &quiet);
    }
    if (!getline(cin, password))
        exit(0);
    if (hidden) {
        tcsetattr(0, TCSANOW, &saved);
        cout << "\n";
    }
    if (!password.empty() && password[password.size() - 1] == '\r')
        password.erase(password.size() - 1);
#endif
    return password;
}
