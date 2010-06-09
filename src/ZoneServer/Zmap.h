
#ifndef ZONE_MAP
#define ZONE_MAP

#include "Utils/typedefs.h"

#include <list>
#include <map>

class Object;

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


	//Get the contents of current cell of the player, looked up by CellID
	std::list<Object*>* GetCellContents(uint32 CellID);

	//Get the contents of chatrange cells
	std::list<Object*>* zmap::GetChatRangeCellContents(uint32 CellID);

	//Update functions for spawn and despawn
	void zmap::UpdateBackCells(Object* updateObject,uint32);
	void zmap::UpdateFrontCells(Object* updateObject, uint32);

	static inline zmap*	GetZmap() { return ZMAP; };


private:

	//This is the accual Hashtable that stores the data
	typedef std::map<uint32, std::list<Object*>>		MapHandler;
	std::map<uint32, std::list<Object*>>				ZMapCells;

	uint32	zmap_lookup[411][411]; // one extra for pretection
	
protected:

	FILE*			ZoneLogs;

	static zmap*	ZMAP;


};
#define ZMap	zmap::GetZmap()
#endif