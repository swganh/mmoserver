#pragma once

#include "Utils/typedefs.h"

class atMacroString
{
public:
	atMacroString();
	~atMacroString();
	void			addTO(string TO) {mTO = TO;}
	void			addTOstf(string dir, string str) {mTOdir = dir;mTOstr = str;}

	void			addDI(uint32 di) {mDI = di;}

	void			addTT(string tt) {mTT = tt;}
	void			addTTstf(string dir, string str) {mTTdir = dir;mTTstr = str;}

	void			addTU(string tu) {mTU = tu;}
	void			addTUId(uint64 tuid) {mTUId = tuid;}

	void			addMB(string mb) {mMB = mb;}
	void			addMBstf(string dir, string str) {mMBdir = dir;mMBstr = str;}
	
	string			assemble();
	void			addTextModule();

	void			setPlanetString(string planet);
	void			addWaypoint();
	void			setWP(float x, float y, float z, string name){mX = x; mY = y; mZ = z; mWPName = name;}


private:

	void			_addByte(uint8 mByte);
	void			_addString(string mString);
	void			_adduint16(uint16 mWord);
	void			_adduint32(uint32 mInteger);
	void			_adduint64(uint64 mLong);
	void			_addfloat(float mFloat);


	bool	mAssembled;
	string	mTO;
	string	mTOdir;
	string	mTOstr;

	string	mTT;
	string	mTTdir;
	string	mTTstr;
	
	uint64	mTUId;
	string	mTU;

	string	mMB;
	string	mMBdir;
	string	mMBstr;

	float		mX;
	float		mY;
	float		mZ;
	uint32		mPlanetCRC;
	string		mWPName;

	uint8 Hallo[5000];
	uint8* mPoint; 
	uint8* mPoint1;
	uint32 mSize;
	uint32 mDI;
	uint16 mCounter;
};
