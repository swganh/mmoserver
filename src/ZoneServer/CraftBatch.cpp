/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "CraftBatch.h"


//=============================================================================

CraftBatch::CraftBatch()
{
}

//=============================================================================

CraftBatch::~CraftBatch()
{
}

//=============================================================================


uint32 CraftBatch::getId()
{ 
    return mId; 
}


uint32 CraftBatch::getListId()
{ 
    return mListId; 
}


uint32 CraftBatch::getExpGroup()
{ 
    return mExpGroup; 
}


CraftWeights* CraftBatch::getCraftWeights()
{ 
    return &mWeights; 
}


CraftAttributes* CraftBatch::getCraftAttributes()
{ 
    return &mAttributes; 
}


CraftAttributeWeights* CraftBatch::getCraftAttributeWeights()
{ 
    return &mAttributeWeights; 
}
