#pragma once

#ifndef ANH_BUFFDBITEM_H
#define ANH_BUFFDBITEM_H

#include "Utils/typedefs.h"

class BuffDBItem
{
public:
	BuffDBItem(void);
	~BuffDBItem(void);
	uint64 mBuffId;
	uint64 mTargetId;
	uint64 mInstigatorId;
	uint64 mMaxTicks;
	uint64 mTickLength;
	uint64 mCurrentTick;
	uint32 mIconCRC;
	uint64 mPausedGlobalTick;
	uint64 mStartGlobalTick;
	string mName;
};
class BuffAttributeDBItem
{
public:

	BuffAttributeDBItem(void){;}
	~BuffAttributeDBItem(void){;}
	int32 mType, mInitialValue, mTickValue, mFinalValue;
};


#endif // ANH_BUFFDBITEM_H
