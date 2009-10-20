#ifndef ANH_BUFFEVENT_H
#define ANH_BUFFEVENT_H

#include "Utils/typedefs.h"

class Buff;

class BuffEvent
{
	public:
		explicit BuffEvent(Buff* buff);
		~BuffEvent(void);

		Buff*		getBuff() const;
		bool		Tick(uint64 CurrentTime, void* ref);

	private:
        BuffEvent();

		Buff** mBuff;
};

#endif // ANH_BUFFEVENT_H
