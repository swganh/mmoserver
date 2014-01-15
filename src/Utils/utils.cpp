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

#include "utils.h"
#include <string>
#include <cstring>
#include <cctype>

#if defined(_MSC_VER)
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

//==========================================================================

uint64 swap64(uint64 value)
{
    uint8 arr[8];

    memcpy(arr, &value, 8);

    for(int x = 0; x < 4; x++)
    {
        uint8 temp = arr[x];
        arr[x] = arr[7-x];
        arr[7-x] = temp;
    }

    memcpy(&value, arr, 8);
    return value;
}

//==========================================================================

std::string strRep(std::string a,std::string b,std::string c)
{
    std::string d;
    std::string::size_type p;
    bool found;

    do
    {
        if((found = (p = a.find(b)) != std::string::npos))
        {
            d += a.substr(0,p) + c;
            a = a.substr(p+b.size());
        }
    }
    while(found);

    d += a;

    return d;
}

//==========================================================================

int Anh_Utils::cmpistr(const char* s1, const char* s2)
{
    int    c1, c2;
    int    cmp = 0;

    if (s1 && s2)
        for (;;)
        {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 && c2)
            {
                c1 = tolower(c1)&0xFF; // 8 bits
                c2 = tolower(c2)&0xFF; // only
                if (c1 < c2)
                {
                    cmp = -1;
                    break;
                }
                else if (c1 > c2)
                {
                    cmp = 1;
                    break;
                }
            }
            else
            {
                if (c1)
                    cmp = 1;
                else if (c2)
                    cmp = -1;
                break;
            }
        }
    return cmp;
}

//==========================================================================

int Anh_Utils::cmpnistr(const char* s1, const char* s2, size_t length)
{
    int    c1, c2;
    int    cmp = 0;

    if (s1 && s2)
        for (size_t i = 0; i < length; ++i)
        {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 && c2)
            {
                c1 = tolower(c1)&0xFF; // 8 bits
                c2 = tolower(c2)&0xFF; // only
                if (c1 < c2)
                {
                    cmp = -1;
                    break;
                }
                else if (c1 > c2)
                {
                    cmp = 1;
                    break;
                }
            }
            else
            {
                if (c1)
                    cmp = 1;
                else if (c2)
                    cmp = -1;
                break;
            }
        }
    return cmp;
}

//==========================================================================

int Anh_Utils::kbhit(void)
{
#if defined(_MSC_VER)
    return _kbhit();
#else
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
#endif
}

//==========================================================================

