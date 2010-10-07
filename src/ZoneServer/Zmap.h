
#ifndef ZONE_MAP
#define ZONE_MAP

#include "Utils/typedefs.h"

#include <list>
#include <set>
#include <vector>
#include <map>

class Object;
class RegionObject;

struct SubCell;

class ZmapSubCellCallback
{
public:
	virtual void ZmapCallback_OnEnterSubCell() {}
	virtual void ZmapCallback_OnExitSubCell() {}
};

class PlayerObject;

#define GRIDWIDTH 410
#define GRIDHEIGHT 410

#define MAPWIDTH 16400
#define MAPHEIGHT 16400

#define VIEWRANGE 3
#define CHATRANGE 3

enum qtype
{
	q_all = 1,
	q_player = 2,
	q_object = 3
};

typedef std::list<Object*>		ObjectListType;
typedef std::list<SubCell*>		SubcellListType;
typedef std::set<Object*>		ObjectSet;

struct ObjectStruct
{
public:
	
	ObjectListType		Objects;
	ObjectListType		Players;
	SubcellListType		SubCells;
};

class zmap
{
public:


	// Contructor & Destructor
	zmap();
	~zmap();

	// Add an object to zmap - returns the cell
	uint32		AddObject(Object* newObject);

	// Remove object from zmap
	void		RemoveObject(Object* removeObject);

	// Update the object in the zmap
	void		UpdateObject(Object* updateObject);

	//bool		checkPlayersNearby(Object* updateObject);

	bool		GetCellValidFlag(uint32 CellID);

	uint32			AddSubCell(float low_x, float low_z, float height, float width, RegionObject* region);
	bool			isObjectInSubCell(Object* object, uint32 subCellId);
	void			RemoveSubCell(uint32 subCellId);

	RegionObject*	getSubCell(uint32 subCellId);

	//Get the contents of current cell of the player, looked up by CellID
	ObjectStruct*		GetCellContents(uint32 CellID);
	ObjectListType*		GetAllCellContents(uint32 CellID);
	ObjectListType*		GetPlayerCellContents(uint32 CellID);
	ObjectListType*		GetObjectCellContents(uint32 CellID);
	
	//=====================================================
	//row
	ObjectListType*		GetAllGridContentsListRow(uint32 CellID);
	ObjectListType*		GetPlayerGridContentsListRow(uint32 CellID);
	ObjectListType*		GetObjectGridContentsListRow(uint32 CellID);
	
	
	//=====================================================
	//(viewRange*2)-1 to accomodate for diametral movement
	ObjectListType*		GetAllGridContentsListRowLeft(uint32 CellID);
	ObjectListType*		GetPlayerGridContentsListRowLeft(uint32 CellID);
	ObjectListType*		GetObjectGridContentsListRowLeft(uint32 CellID);

	ObjectListType*		GetAllGridContentsListRowRight(uint32 CellID);
	ObjectListType*		GetPlayerGridContentsListRowRight(uint32 CellID);
	ObjectListType*		GetObjectGridContentsListRowRight(uint32 CellID);
	
	ObjectListType*		GetAllGridContentsListColumn(uint32 CellID);
	ObjectListType*		GetPlayerGridContentsListColumn(uint32 CellID);
	ObjectListType*		GetObjectGridContentsListColumn(uint32 CellID);

	//=====================================================
	//(viewRange*2)-1 to accomodate for diametral movement
	ObjectListType*		GetPlayerGridContentsListColumnDown(uint32 CellID);
	ObjectListType*		GetAllGridContentsListColumnDown(uint32 CellID);
	ObjectListType*		GetObjectGridContentsListColumnDown(uint32 CellID);

	ObjectListType*		GetPlayerGridContentsListColumnUp(uint32 CellID);
	ObjectListType*		GetAllGridContentsListColumnUp(uint32 CellID);
	ObjectListType*		GetObjectGridContentsListColumnUp(uint32 CellID);

	//Get the contents of chatrange cells
	ObjectListType*		GetChatRangeCellContents(uint32 CellID);

	ObjectListType*		GetAllViewingRangeCellContents(uint32 CellID);
	ObjectListType*		GetPlayerViewingRangeCellContents(uint32 CellID);
	ObjectListType*		GetObjectViewingRangeCellContents(uint32 CellID);
	
	ObjectListType*		GetObjectCustomRangeCellContents(uint32 CellID, uint32 range);
	
	//void	geInRange(const Object* const object,ObjectSet* resultSet,uint32 objTypes,float range, bool cellContent);

	

	//void CheckObjectIterationForDestruction(Object* toBeTested, Object* toBeUpdated);
	//void CheckObjectIterationForCreation(Object* toBeTested, Object* toBeUpdated);
	//void ObjectCreationIteration(ObjectListType* FinalList, Object* updateObject);

	uint32 getCellId(float x, float z){return _getCellId(x, z);}

	static inline zmap*	GetZmap() { return ZMAP; };

private:

	uint32		_getCellId(float x, float z);
	
	bool _isInSubCellExtent(SubCell* subCell, float x, float z);

	//This is the accual Hashtable that stores the data
	typedef std::map<uint32, ObjectListType>		MapHandler;
	std::map<uint32, ObjectStruct>						ZMapCells;


	
	ObjectListType									EmptyCell;//for the return of nonexisting grids
	ObjectStruct										EmptyStruct;

	uint32	zmap_lookup[GRIDWIDTH+1][GRIDHEIGHT+1]; // one extra for protection
	
	// CellId -> List of SubCells
	std::multimap<uint32, SubCell*> subCells;

	uint32		mCurrentSubCellID;
	int32		viewRange;

protected:

	//FILE*			ZoneLogs;

	static zmap*	ZMAP;


};
#define ZMap	zmap::GetZmap()
#endif