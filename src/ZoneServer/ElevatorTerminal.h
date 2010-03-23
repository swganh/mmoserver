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
		Anh_Math::Quaternion	getDstDirUp(){ return mDstDirUp; }
		Anh_Math::Vector3		getDstPosUp(){ return mDstPosUp; }
		uint32					getEffectUp(){ return mEffectUp; }

		uint64					getDstCellDown(){ return mDstCellDown; }
		Anh_Math::Quaternion	getDstDirDown(){ return mDstDirDown; }
		Anh_Math::Vector3		getDstPosDown(){ return mDstPosDown; }
		uint32					getEffectDown(){ return mEffectDown; }

		
		virtual void		prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void		handleObjectMenuSelect(uint8 messageType,Object* srcObject);

	private:

		uint64					mDstCellUp;
		Anh_Math::Quaternion	mDstDirUp;
		Anh_Math::Vector3		mDstPosUp;
		uint32					mEffectUp;

		uint64					mDstCellDown;
		Anh_Math::Quaternion	mDstDirDown;
		Anh_Math::Vector3		mDstPosDown;
		uint32					mEffectDown;
};

//=============================================================================

#endif

