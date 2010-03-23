#include "StreamColors.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

std::ostream& blue(std::ostream &s)
{
#ifdef _MSC_VER
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
#else
		s << "\033[22;34m";
#endif
    return s;
}

std::ostream& aqua(std::ostream &s)
{
#ifdef _MSC_VER
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE
              |FOREGROUND_GREEN|FOREGROUND_INTENSITY);
#else
		s << "\033[01;34m";
#endif
    return s;
}

std::ostream& red(std::ostream &s)
{
#ifdef _MSC_VER
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, 
                FOREGROUND_RED|FOREGROUND_INTENSITY);
#else
		s << "\033[22;31m";
#endif
    return s;
}

std::ostream& green(std::ostream &s)
{
#ifdef _MSC_VER
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, 
              FOREGROUND_GREEN|FOREGROUND_INTENSITY);
#else
		s << "\033[22;32m";
#endif
    return s;
}

std::ostream& yellow(std::ostream &s)
{
#ifdef _MSC_VER
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, 
         FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
#else
		s << "\033[01;33m";
#endif
    return s;
}

std::ostream& white(std::ostream &s)
{
#ifdef _MSC_VER
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, 
       FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
#else
		s << "\033[01;37m";
#endif
    return s;
}

