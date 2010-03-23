/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "DraftWeight.h"


//=============================================================================

DraftWeight::DraftWeight()
{
}

//=============================================================================

DraftWeight::~DraftWeight()
{
}

//=============================================================================

void DraftWeight::prepareData()
{
	mData = (mDataType << 4) | mDistribution;
}

//=============================================================================


