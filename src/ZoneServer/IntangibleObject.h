/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_INTANGIBLE_OBJECT_H
#define ANH_ZONESERVER_INTANGIBLE_OBJECT_H

#include "Object.h"

//Forward Declerations
class PlayerObject;

//=============================================================================
//
// Intangible groups
//
enum IntangibleGroup
{
	ItnoGroup_None					= 0,
	ItnoGroup_VehicleController				= 1,
	ItnoGroup_Mount					= 2,
	ItnoGroup_Pet					= 3,
	ItnoGroup_Droid					= 4
};

class IntangibleObject : public Object
{
	friend class ObjectFactory;
	friend class VehicleControllerFactory;

public:
		IntangibleObject();
		~IntangibleObject();

		string				getName() const { return mName; }
		void				setName(const int8* name){ mName = name; }
		string				getNameFile() const { return mNameFile; }
		void				setNameFile(const int8* file){ mNameFile = file; }
		string				getCustomName() const { return mCustomName; }
		void				setCustomName(const int8* name){ mCustomName = name; }
		string				getDetailFile() { return mDetailFile; }
		void				setDetailFile(const char* detail_file) { mDetailFile = detail_file; }
		string				getDetail()	{ return mDetail; }
		void				setDetail(const char* detail) { mDetail = detail; }
		float				getComplexity(){ return mComplexity; }
		void				setComplexity(float complexity){ mComplexity = complexity; }
		int32				getVolume() { return mVolume; }
		void				setVolume(const int32 volume) { mVolume = volume; }
		int					getItnoGroup() { return mItnoGroup; }
		void				setItnoGroup(int itno_group) { mItnoGroup = itno_group; }
		string				getPhysicalModelString() { return mPhysicalModel; }
		void				setPhysicalModelString(string physical_model) { mPhysicalModel = physical_model; }
protected:
		float				mComplexity;
		int32				mVolume;
		string				mCustomName;
		string				mName;
		string				mNameFile;
		string				mDetailFile;			//ASCII
		string				mDetail;				//Description - ASCII
		string				mPhysicalModel;			//Model string for the physical part
		int					mItnoGroup;

private:

};

#endif
