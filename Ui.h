#ifndef UI_H
#define UI_H
#include <string>
using namespace std;
const int BOX_WIDTH = 60;  
const string RESET = "\033[0m";
const string BOLD  = "\033[1m";
const string GREY  = "\033[90m";
const string RED   = "\033[91m";
const string GREEN = "\033[92m";
const string AMBER = "\033[93m";
const string WHITE = "\033[97m";
const string DOT = "\xC2\xB7";
void initConsole();               
void clearScreen();
void cursorHome();               
void clearBelow();               
bool keyPressed();               
void sleepMs(int milliseconds);  
void setUiTheme(int theme);      
string accent();                  
string paint(const string& text, const string& color);
int visibleLength(const string& s);
string padRight(const string& s, int width);
string padLeft(const string& s, int width);
string num1(double v);                         
string statusText(const string& status);        
string trendText(const string& label);          
string bar(double percent, int width, const string& color);
string sparkline(const double values[], int count, double maxValue, int width);   
string statusBar(double percent, int width);    
void drawTop(const string& title);
void drawMiddle(const string& title = "");
void drawBottom();
void drawRow(const string& text = "");
void drawRowSplit(const string& left, const string& right);
void drawCentered(const string& text);
void drawWrapped(const string& text, const string& color = "");
void drawMenuItem(const string& key, const string& label);
void showSuccess(const string& msg);
void showError(const string& msg);
void showAlert(const string& msg);
void pauseScreen(const string& msg = "Press any key to continue...");
void showWelcome();
int readInt(const string& prompt);
double readDouble(const string& prompt);
string readText(const string& prompt);
string readPassword(const string& prompt);  
#endif
