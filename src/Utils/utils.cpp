/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "utils.h"
#include <string>
#include <cstring>

#if defined(_MSC_VER)
#include <conio.h>
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
		if(found = (p = a.find(b)) != std::string::npos)
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

