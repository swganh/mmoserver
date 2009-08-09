/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include <iostream>
#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
	std::cout << "Running MMOServer Unit Tests\n";

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

