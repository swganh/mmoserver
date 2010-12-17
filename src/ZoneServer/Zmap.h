
#ifndef ZONE_MAP
#define ZONE_MAP

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "Utils/typedefs.h"

class Object;
class RegionObject;

class PlayerObject;

#define GRIDWIDTH 410
#define GRIDHEIGHT 410

#define MAPWIDTH 16400
#define MAPHEIGHT 16400

#define VIEWRANGE 3
#define CHATRANGE 1

enum qtype	{
	q_all		= 1,
	q_player	= 2,
	q_object	= 3
};

typedef std::list<Object*> ObjectListType;
typedef std::list<std::shared_ptr<Object>> SharedObjectListType;
typedef std::set<Object*> ObjectSet;
typedef std::multimap<uint32, std::shared_ptr<RegionObject>> SubCellMap;

enum BucketType {
	Bucket_Creatures = 1,
	Bucket_Objects	 = 2,
	Bucket_Players	 = 4
};

struct ObjectStruct	{
public:	
	ObjectListType       Objects;
	ObjectListType       Creatures;
	ObjectListType       Players;
	SharedObjectListType SubCells;
};

class zmap {
public:
	// Contructor & Destructor
	zmap();
	~zmap();
        
	void updateRegions(Object* object);
    
	void addRegion(std::shared_ptr<RegionObject> region);
	std::shared_ptr<RegionObject> findRegion(uint64_t region_id);

	// Add an object to zmap - returns the cell
	uint32				AddObject(Object* newObject);

	// Remove object from zmap
	void				RemoveObject(Object* removeObject);

	// Update the object in the zmap
	void				UpdateObject(Object* updateObject);

	//bool		checkPlayersNearby(Object* updateObject);

	bool				GetCellValidFlag(uint32 CellID);


	bool				isObjectInRegion(Object* object, uint64 regionid);
	void				RemoveRegion(uint64 regionId);
    	

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
	
	bool		isObjectInRegionBoundary_(Object* object, std::shared_ptr<RegionObject> region);

	//This is the actual Hashtable that stores the data
	typedef std::map<uint32, ObjectListType>		MapHandler;
	
	std::map<uint32, ObjectStruct*>					ZMapCells;


	
	ObjectListType									EmptyCell;//for the return of nonexisting grids
	ObjectStruct									EmptyStruct;

	uint32	zmap_lookup[GRIDWIDTH+1][GRIDHEIGHT+1]; // one extra for protection
		

	uint32		mCurrentSubCellID;
	int32		viewRange;
	int32		chatRange;

protected:

	//FILE*			ZoneLogs;

	static zmap*	ZMAP;
	SubCellMap		subCells;


};
#define ZMap	zmap::GetZmap()
#endif
