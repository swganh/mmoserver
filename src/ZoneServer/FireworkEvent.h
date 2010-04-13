/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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

		FireworkEvent(uint32 _typeId,PlayerObject* _playerObject, const glm::vec3&	_position)
		{
			this->typeId = _typeId;
			this->playerObject=_playerObject;
			this->mPosition = _position;
			this->mPosition.x +=static_cast<float>((gRandom->getRand()%5)-2.5);
			this->mPosition.z +=static_cast<float>((gRandom->getRand()%5)-2.5);
		}

		PlayerObject*		getPlayerObject() const { return playerObject; }
		uint32				getType() const { return typeId; }
		const glm::vec3&	getPosition() const { return mPosition; }

	private:
		PlayerObject* playerObject;
		uint32	typeId;
        glm::vec3 mPosition;
};


//======================================================================================================================

#endif


