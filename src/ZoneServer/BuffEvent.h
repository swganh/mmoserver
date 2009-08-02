#ifndef ANH_BUFFEVENT_H
#define ANH_BUFFEVENT_H

#include "Utils/EventHandler.h"

#include "Utils\typedefs.h"

class Buff;

class BuffEvent
{
	public:
		~BuffEvent(void);
		BuffEvent(Buff* buff){ mBuff = &buff; }
		
		Buff*		getBuff() const { return *mBuff; }
		bool		Tick(uint64 CurrentTime, void* ref);

	private:

		Buff** mBuff;
};

#endif ANH_BUFFEVENT_H