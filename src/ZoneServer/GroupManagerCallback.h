#ifndef GROUP_MANAGER_CALLBACK_H
#define GROUP_MANAGER_CALLBACK_H

#include "Utils/typedefs.h"

#ifndef GROUP_MANAGER_CALLBACKCONTAINER_H
	class GroupManagerCallbackContainer;
#endif

class GroupManagerCallback
{
public:
	virtual void handleGroupManagerCallback(uint64 playerId, GroupManagerCallbackContainer* container) = 0;
};

#endif