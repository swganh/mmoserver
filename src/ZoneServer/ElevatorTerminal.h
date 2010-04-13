/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_ELEVATORTERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_ELEVATORTERMINAL_H

#include "Terminal.h"

//=============================================================================

class ElevatorTerminal : public Terminal
{
	friend class TerminalFactory;

	public:

		ElevatorTerminal();
		~ElevatorTerminal();

		uint64					getDstCellUp(){ return mDstCellUp; }
		const glm::quat&	    getDstDirUp(){ return mDstDirUp; }
		const glm::vec3&		getDstPosUp(){ return mDstPosUp; }
		uint32					getEffectUp(){ return mEffectUp; }

		uint64					getDstCellDown(){ return mDstCellDown; }
		const glm::quat&	    getDstDirDown(){ return mDstDirDown; }
		const glm::vec3&		getDstPosDown(){ return mDstPosDown; }
		uint32					getEffectDown(){ return mEffectDown; }

		
		virtual void		prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void		handleObjectMenuSelect(uint8 messageType,Object* srcObject);

	private:

		uint64		mDstCellUp;
        glm::quat	mDstDirUp;
        glm::vec3	mDstPosUp;
		uint32		mEffectUp;

		uint64		mDstCellDown;
        glm::quat	mDstDirDown;
        glm::vec3	mDstPosDown;
		uint32		mEffectDown;
};

//=============================================================================

#endif

