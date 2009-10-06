/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_FIREWORK_EVENT_H
#define ANH_ZONESERVER_FIREWORK_EVENT_H

#include "Utils/EventHandler.h"
#include "Utils/rand.h"


//======================================================================================================================

class FireworkEvent : public Anh_Utils::Event
{
	public:

		FireworkEvent(uint32 _typeId,PlayerObject* _playerObject, Anh_Math::Vector3	_position)
		{
			this->typeId = _typeId;
			this->playerObject=_playerObject;
			this->mPosition = _position;
			this->mPosition.mX +=static_cast<float>((gRandom->getRand()%5)-2.5);
			this->mPosition.mZ +=static_cast<float>((gRandom->getRand()%5)-2.5);
		}

		PlayerObject*		getPlayerObject() const { return playerObject; }
		uint32				getType() const { return typeId; }
		Anh_Math::Vector3	getPosition() const { return mPosition; }

	private:
		PlayerObject* playerObject;
		uint32	typeId;
		Anh_Math::Vector3 mPosition;
};


//======================================================================================================================

#endif


