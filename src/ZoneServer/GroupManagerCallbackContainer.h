#ifndef GROUP_MANAGER_CALLBACKCONTAINER_H
#define GROUP_MANAGER_CALLBACKCONTAINER_H

#include "Utils/typedefs.h"

#ifndef GROUP_MANAGER_CALLBACK_H
	class GroupManagerCallback;
#endif

enum GroupManagerCallback_OperationTypes
{
	GROUPMANAGERCALLBACK_STARTBAND		= 0,
	GROUPMANAGERCALLBACK_STOPBAND		= 1,
	GROUPMANAGERCALLBACK_BANDFLOURISH	= 2
};

class GroupManagerCallbackContainer
{
public:
	uint32 operation;
	bool isLeader;
	string arg;
	uint32 flourishId;
	uint64 requestingPlayer;

	GroupManagerCallback* callback;
};

#endif