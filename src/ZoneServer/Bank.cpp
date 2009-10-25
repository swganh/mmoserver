/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "Bank.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "MathLib/Quaternion.h"

//=============================================================================

Bank::Bank() : TangibleObject(),
mCredits(0),
mPlanet(-1)
{
	mTanGroup	= TanGroup_PlayerInternal;
	mTanType	= TanType_Bank;
}

//=============================================================================

Bank::~Bank()
{
	ObjectList::iterator it = mObjects.begin();
	while(it != mObjects.end())
	{
		delete(*it);
		mObjects.erase(it);
		it = mObjects.begin();
	}
}

//=============================================================================

bool Bank::updateCredits(int32 amount)
{
	if(mCredits + amount < 0)
		return(false);

	mCredits += amount;

	if(mParent->getType() == ObjType_Player)
		gMessageLib->sendBankCreditsUpdate(dynamic_cast<PlayerObject*>(mParent));

	gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE banks set credits=credits+%i WHERE id=%"PRId64"",amount,mId);

	return(true);
}

//=============================================================================


