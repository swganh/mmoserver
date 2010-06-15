
#ifndef ZONE_MAP
#define ZONE_MAP

#include "Utils/typedefs.h"

#include <list>
#include <vector>
#include <map>

class Object;
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

#define VIEWRANGE 3
#define CHATRANGE 3

enum qtype
{
	q_all = 1,
	q_player = 2,
	q_object = 3
};

struct ObjectStruct
{
public:
	
	std::list<Object*>		Objects;
	std::list<Object*>		Players;
};

class zmap
{
public:

	// Contructor & Destructor
	zmap();
	~zmap();

	// Add an object to zmap
	bool		AddObject(Object* newObject);

	// Remove object from zmap
	void		RemoveObject(Object* removeObject);

	// Update the object in the zmap
	void UpdateObject(Object* updateObject);

	uint32	AddSubCell(float low_x, float low_z, float height, float width, ZmapSubCellCallback* callback);
	bool	isObjectInSubCell(Object* object, uint32 subCellId);
	void	RemoveSubCell(uint32 subCellId);

	//Get the contents of current cell of the player, looked up by CellID
	ObjectStruct*		GetCellContents(uint32 CellID);
	std::list<Object*>* GetAllCellContents(uint32 CellID);
	std::list<Object*>* GetPlayerCellContents(uint32 CellID);
	std::list<Object*>* GetObjectCellContents(uint32 CellID);
	
	//=====================================================
	//row
	std::list<Object*>* GetAllGridContentsListRow(uint32 CellID);
	std::list<Object*>* GetPlayerGridContentsListRow(uint32 CellID);
	std::list<Object*>* GetObjectGridContentsListRow(uint32 CellID);
	
	
	//=====================================================
	//(viewRange*2)-1 to accomodate for diametral movement
	std::list<Object*>* GetAllGridContentsListRowLeft(uint32 CellID);
	std::list<Object*>* GetPlayerGridContentsListRowLeft(uint32 CellID);
	std::list<Object*>* GetObjectGridContentsListRowLeft(uint32 CellID);

	std::list<Object*>* GetAllGridContentsListRowRight(uint32 CellID);
	std::list<Object*>* GetPlayerGridContentsListRowRight(uint32 CellID);
	std::list<Object*>* GetObjectGridContentsListRowRight(uint32 CellID);
	
	std::list<Object*>* GetAllGridContentsListColumn(uint32 CellID);
	std::list<Object*>* GetPlayerGridContentsListColumn(uint32 CellID);
	std::list<Object*>* GetObjectGridContentsListColumn(uint32 CellID);

	//=====================================================
	//(viewRange*2)-1 to accomodate for diametral movement
	std::list<Object*>* GetPlayerGridContentsListColumnDown(uint32 CellID);
	std::list<Object*>* GetAllGridContentsListColumnDown(uint32 CellID);
	std::list<Object*>* GetObjectGridContentsListColumnDown(uint32 CellID);

	std::list<Object*>* GetPlayerGridContentsListColumnUp(uint32 CellID);
	std::list<Object*>* GetAllGridContentsListColumnUp(uint32 CellID);
	std::list<Object*>* GetObjectGridContentsListColumnUp(uint32 CellID);

	//Get the contents of chatrange cells
	std::list<Object*>* GetChatRangeCellContents(uint32 CellID);

	std::list<Object*>* GetAllViewingRangeCellContents(uint32 CellID);
	std::list<Object*>* GetPlayerViewingRangeCellContents(uint32 CellID);
	std::list<Object*>* GetObjectViewingRangeCellContents(uint32 CellID);

	//Update functions for spawn and despawn
	void zmap::UpdateBackCells(Object* updateObject,uint32);
	void zmap::UpdateFrontCells(Object* updateObject, uint32);

	void CheckObjectIterationForDestruction(Object* toBeTested, Object* toBeUpdated);
	void CheckObjectIterationForCreation(Object* toBeTested, Object* toBeUpdated);
	void ObjectCreationIteration(std::list<Object*>* FinalList, Object* updateObject);

	static inline zmap*	GetZmap() { return ZMAP; };

private:

	uint32 _getCellId(float x, float z);
	bool _isInSubCellExtent(SubCell* subCell, float x, float z);

	//This is the accual Hashtable that stores the data
	typedef std::map<uint32, std::list<Object*>>		MapHandler;
	std::map<uint32, ObjectStruct>						ZMapCells;


	
	std::list<Object*>									EmptyCell;//for the return of nonexisting grids
	ObjectStruct										EmptyStruct;

	uint32	zmap_lookup[GRIDWIDTH+1][GRIDHEIGHT+1]; // one extra for protection
	
	// CellId -> List of SubCells
	std::multimap<uint32, SubCell*> subCells;

	uint32		mCurrentSubCellID;

protected:

	//FILE*			ZoneLogs;

	static zmap*	ZMAP;


};
#define ZMap	zmap::GetZmap()
#endif