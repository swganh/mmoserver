
#ifndef ZONE_MAP
#define ZONE_MAP

#include <list>
#include <map>
#include <set>
#include <vector>

#include "Utils/typedefs.h"

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
#define CHATRANGE 1

enum qtype
{
	q_all = 1,
	q_player = 2,
	q_object = 3
};

typedef std::list<Object*>		ObjectListType;
typedef std::list<SubCell*>		SubcellListType;
typedef std::set<Object*>		ObjectSet;

enum BucketType 
{
	Bucket_Creatures = 1,
	Bucket_Objects	 = 2,
	Bucket_Players	 = 4
};

struct ObjectStruct
{
public:
	
	ObjectListType		Objects;
	ObjectListType		Creatures;
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
	uint32				AddObject(Object* newObject);

	// Remove object from zmap
	void				RemoveObject(Object* removeObject);

	// Update the object in the zmap
	void				UpdateObject(Object* updateObject);

	//bool		checkPlayersNearby(Object* updateObject);

	bool				GetCellValidFlag(uint32 CellID);

	uint32				AddSubCell(float low_x, float low_z, float height, float width, RegionObject* region);
	bool				isObjectInSubCell(Object* object, uint32 subCellId);
	void				RemoveSubCell(uint32 subCellId);

	RegionObject*		getSubCell(uint32 subCellId);

	//Get the contents of current cell of the player, looked up by CellID
	void				GetCellContents(uint32 CellID, ObjectListType* list, uint32 type);//gets contents based on type enum
	void				GetPlayerCellContents(uint32 CellID, ObjectListType* list);
	
	//=====================================================
	//row
	void				GetPlayerGridContentsListRow(uint32 CellID, ObjectListType* list);
	void				GetGridContentsListRow(uint32 CellID, ObjectListType* list, uint32 type);
	
	
	//=====================================================
	//(viewRange*2)-1 to accomodate for diametral movement
	void				GetGridContentsListRowLeft(uint32 CellID, ObjectListType* list, uint32 type);
	void				GetPlayerGridContentsListRowLeft(uint32 CellID, ObjectListType* list);

	void				GetGridContentsListRowRight(uint32 CellID, ObjectListType* list, uint32 type);
	void				GetPlayerGridContentsListRowRight(uint32 CellID, ObjectListType* list);
		
	void				GetGridContentsListColumn(uint32 CellID, ObjectListType* list, uint32 type);
	void				GetPlayerGridContentsListColumn(uint32 CellID, ObjectListType* list);

	//=====================================================
	//(viewRange*2)-1 to accomodate for diametral movement
	void				GetPlayerGridContentsListColumnDown(uint32 CellID, ObjectListType* list);
	void				GetGridContentsListColumnDown(uint32 CellID, ObjectListType* list, uint32 type);
	

	void				GetPlayerGridContentsListColumnUp(uint32 CellID, ObjectListType* list);
	void				GetGridContentsListColumnUp(uint32 CellID, ObjectListType* list, uint32 type);
	

	//Get the contents of chatrange cells
	void				GetChatRangeCellContents(uint32 CellID, ObjectListType* list);

	void				GetViewingRangeCellContents(uint32 CellID, ObjectListType* list, uint32 type);
	void				GetPlayerViewingRangeCellContents(uint32 CellID, ObjectListType* list);
	
	void				GetCustomRangeCellContents(uint32 CellID, uint32 range, ObjectListType* list, uint32 type);
	
	

	uint32 getCellId(float x, float z){return _getCellId(x, z);}

	static inline zmap*	GetZmap() { return ZMAP; };

private:

	uint32		_getCellId(float x, float z);
	
	bool		_isInSubCellExtent(SubCell* subCell, float x, float z);

	//This is the actual Hashtable that stores the data
	typedef std::map<uint32, ObjectListType>		MapHandler;
	
	std::map<uint32, ObjectStruct*>					ZMapCells;


	
	ObjectListType									EmptyCell;//for the return of nonexisting grids
	ObjectStruct									EmptyStruct;

	uint32	zmap_lookup[GRIDWIDTH+1][GRIDHEIGHT+1]; // one extra for protection
	
	// CellId -> List of SubCells
	std::multimap<uint32, SubCell*> subCells;

	uint32		mCurrentSubCellID;
	int32		viewRange;
	int32		chatRange;

protected:

	//FILE*			ZoneLogs;

	static zmap*	ZMAP;


};
#define ZMap	zmap::GetZmap()
#endif