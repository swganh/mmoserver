
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

class zmap
{
public:

	// Contructor & Destructor
	zmap();
	~zmap();

	// Add an object to zmap
	void AddObject(Object* newObject);

	// Remove object from zmap
	void RemoveObject(Object* removeObject);

	// Update the object in the zmap
	void UpdateObject(Object* updateObject);

	uint32	AddSubCell(float low_x, float low_z, float height, float width, ZmapSubCellCallback* callback);
	bool	isObjectInSubCell(Object* object, uint32 subCellId);
	void	RemoveSubCell(uint32 subCellId);

	//Get the contents of current cell of the player, looked up by CellID
	std::list<Object*>* GetCellContents(uint32 CellID);
	std::list<Object*>* GetGridContentsListRow(uint32 CellID);
	std::list<Object*>* GetGridContentsListColumn(uint32 CellID);

	//Get the contents of chatrange cells
	std::list<Object*>* GetChatRangeCellContents(uint32 CellID);

	std::list<Object*>* GetViewingRangeCellContents(uint32 CellID);

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
	std::map<uint32, std::list<Object*>>				ZMapCells;
	
	std::list<Object*>									EmptyCell;//for the return of nonexisting grids

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