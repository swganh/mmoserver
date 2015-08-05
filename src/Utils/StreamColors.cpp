/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

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

