#include "Zmap.h"

#include <cassert>
#include <algorithm>

#include "Object.h"
#include "RegionObject.h"

#include "ScriptEngine/ScriptEngine.h"
#include "Utils/MathFunctions.h"

namespace {
// Region events are raised for players only: creatures cross regions all the time.
void raiseRegionEvent(const char* name, RegionObject* region, Object* object) {
    if (object->getType() != ObjType_Player) {
        return;
    }
    gScriptEngine->raiseEvent(ScriptEvent(name, region->getId(), object->getId())
        .set("region_name", region->getRegionName())
        .set("region_type", static_cast<double>(region->getRegionType())));
}
}
//#include "PlayerObject.h"

using std::find_if;
using std::for_each;
using std::remove_if;
using std::shared_ptr;




zmap* zmap::ZMAP = NULL;


zmap::zmap()
{
    mCurrentSubCellID = 0;

    ZMAP = this;

    viewRange = VIEWRANGE;
    chatRange = CHATRANGE;

    uint32 x = 0, i = 0, j = 0;


    // Setup the lookup array...

    for (x = 0; x <= GRIDWIDTH; x++) {
        for (j = 0; j <= GRIDHEIGHT; j++) {
            zmap_lookup[x][j] = i;
            ObjectStruct* bucket = new(ObjectStruct);
            ZMapCells.insert(std::make_pair(i,bucket));
            i++;

        }
    }
}

zmap::~zmap()
{
    uint32 x = 0, i = 0, j = 0;

    for(x = 0; x <= GRIDWIDTH; x++)
    {
        for(j = 0; j <= GRIDHEIGHT; j++)
        {
            std::map<uint32, ObjectStruct*>::iterator it = ZMapCells.find(i);


            (*it).second->Creatures.clear();
            (*it).second->Players.clear();
            (*it).second->Objects.clear();
            (*it).second->SubCells.clear();

            delete(	ZMapCells[i]);
            i++;

        }
    }
}

void zmap::updateRegions(Object* object) {
	// Check the regions the object is currently in and remove any it's no longer in.
	Uint64Set& region_set = object->zmapSubCells;
    
    auto region_set_end = region_set.end();
    auto region_set_it = region_set.begin();

    while (region_set_it != region_set_end) {
        auto region = findRegion(*region_set_it);

        if (!region) {
            region_set.erase(region_set_it++);
            continue;
        }

        if (! isObjectInRegionBoundary_(object, region)) {
            region->onObjectLeave(object);
            raiseRegionEvent("region_left", region.get(), object);
            region_set.erase(region_set_it++);
            continue;
        }

        ++region_set_it;
    }

    // Now check for any new regions the object may have entered.
    auto it = ZMapCells.find(object->getGridBucket());
    if (it == ZMapCells.end()) {
        assert(false && "Object has reference to an invalid grid bucket!");
        return;
    }

    SharedObjectListType& list = (*it).second->SubCells;
    for_each(list.begin(), list.end(), [this, &region_set, object] (shared_ptr<Object> list_object) {
        shared_ptr<RegionObject> region = std::static_pointer_cast<RegionObject>(list_object);

        // If the object is not already in the region and is now within 
        // the region's bounds, add it.
        if (region_set.find(region->getId()) == region_set.end() && isObjectInRegionBoundary_(object, region)) {
            object->zmapSubCells.insert(region->getId());
            region->onObjectEnter(object);
            raiseRegionEvent("region_entered", region.get(), object);
        }
    });
}

bool zmap::isObjectInRegionBoundary_(Object* object, shared_ptr<RegionObject> region) {
    return IsPointInRegion(glm::vec2(object->mPosition.x, object->mPosition.z),
                           glm::vec2(region->mPosition.x, region->mPosition.z),
                           region->getWidth(), region->getHeight());
}


void zmap::addRegion(std::shared_ptr<RegionObject> region) {
    for (uint32 cell : _regionCells(region.get())) {
        ZMapCells[cell]->SubCells.push_back(region);
    }

	subCells.insert(std::make_pair(region->getId(), region));
}

// The grid cells a region's bounds overlap (regions off the map use the
// edge cells, as positions do in _getCellId).
std::vector<uint32> zmap::_regionCells(RegionObject* region) {
    glm::vec2 lower_left, upper_right;
    GetRegionBounds(glm::vec2(region->mPosition.x, region->mPosition.z),
                    region->getWidth(), region->getHeight(), lower_left, upper_right);

    int32 low_row, low_column, high_row, high_column;
    _getRowColumn(lower_left.x, lower_left.y, low_row, low_column);
    _getRowColumn(upper_right.x, upper_right.y, high_row, high_column);

    std::vector<uint32> cells;
    for (int32 row = low_row; row <= high_row; ++row) {
        for (int32 column = low_column; column <= high_column; ++column) {
            cells.push_back(zmap_lookup[row][column]);
        }
    }
    return cells;
}

std::shared_ptr<RegionObject> zmap::findRegion(uint64_t region_id) {
    auto it = find_if(subCells.begin(), subCells.end(), [region_id] (const SubCellMap::value_type& map_entry) {
        return map_entry.second->getId() == region_id;
    });

    // If we found the region in the cell map return it, otherwise return a nullptr.
    return (it != subCells.end()) ? (*it).second : nullptr;
}

bool zmap::isObjectInRegion(Object* object, uint64 region_id) {
    auto it = find_if(subCells.begin(), subCells.end(), [this, object, region_id] (const SubCellMap::value_type& map_entry) {
        return isObjectInRegionBoundary_(object, map_entry.second);
    });

    // If we found the region in the cell map return it, otherwise return a nullptr.
    return it != subCells.end();
}

void zmap::RemoveRegion(uint64 regionId) {
    // By the full id: the map's key is a truncated one.
    auto it = find_if(subCells.begin(), subCells.end(), [regionId] (const SubCellMap::value_type& map_entry) {
        return map_entry.second->getId() == regionId;
    });
    if (it == subCells.end()) {
        return;
    }

    for (uint32 cell : _regionCells(it->second.get())) {
        SharedObjectListType& list = ZMapCells[cell]->SubCells;
        list.remove_if([regionId] (const std::shared_ptr<Object>& object) {
            return object->getId() == regionId;
        });
    }

    subCells.erase(it);
}

void zmap::_getRowColumn(float x, float z, int32& row, int32& column)
{
    row = (static_cast<int32>(z) + (MAPWIDTH/2)) / GRIDWIDTH;
    column = (static_cast<int32>(x) + (MAPHEIGHT/2)) / GRIDHEIGHT;
    row = std::max(0, std::min(row, GRIDWIDTH));
    column = std::max(0, std::min(column, GRIDHEIGHT));
}

uint32 zmap::_getCellId(float x, float z)
{
    // Convert through a signed integer: a negative float converted straight
    // to an unsigned one is undefined. GCC happened to wrap it (and adding
    // half the map size wrapped it back); MSVC does not, and the index left
    // the table (an access violation as soon as an NPC moved at negative
    // coordinates). Positions off the map use the edge cells.
    int32 row, column;
    _getRowColumn(x, z, row, column);
    return zmap_lookup[row][column];
}


void zmap::RemoveObject(Object *removeObject)
{
    uint32 cellId = removeObject->getGridBucket();

    if(!GetCellValidFlag(cellId))
    {
        DLOG(INFO) << "zmap::RemoveObject :: bucket " << cellId << " NOT valid";
        return;
    }

    //DLOG(INFO) << "zmap::RemoveObject :: " << removeObject->getId() << " bucket " << cellId << " ";

    ObjectListType *list;

    std::map<uint32, ObjectStruct*>::iterator it = ZMapCells.find(cellId);

    switch(removeObject->getType())
    {
    case ObjType_Player:
    {
        list = &(*it).second->Players;
    }
    break;

    case ObjType_Creature:
    case ObjType_NPC:
    {
        list = &(*it).second->Creatures;
    }
    break;

    default:
    {
        list = &(*it).second->Objects;
    }
    break;
    }

    for(ObjectListType::iterator remove_it = list->begin(); remove_it != list->end(); remove_it++)    {
        if((*remove_it)->getId() == removeObject->getId())      {
            list->erase(remove_it);
            break;
        }
    }

    //make sure we can use the mGridBucket to determine what bucket we *are* in
    //so we do not have to search the list on insert
    removeObject->setGridBucket(0xffffffff);

	//regions are only for players / creatures at this point 
    if((removeObject->getType() != ObjType_Player) && (removeObject->getType() != ObjType_NPC) & (removeObject->getType() != ObjType_Creature))
        return;

	//remove out of any regions we might be in
	//We need to check which subregions to leave
	Uint64Set*				region_set	= &removeObject->zmapSubCells;
    Uint64Set::iterator		set_it		= region_set->begin();

    while(set_it != region_set->end())    {
        auto region = findRegion(*set_it);

        if (!region) {
		    region_set->erase(set_it++);
		    continue;
        }

		region->onObjectLeave(removeObject);
		raiseRegionEvent("region_left", region.get(), removeObject);
		region_set->erase(set_it++);	
    }

}


void zmap::GetCellContents(uint32 CellID, ObjectListType* list, uint32 type)
{
    if(type == 0)    {
        assert(false && "zmap::GetCellContents QueryType must NOT be 0");
    }
    
	if(CellID > (GRIDWIDTH*GRIDHEIGHT))    {
        DLOG(INFO) << "zmap::GetCellContents :: bucket " << CellID << " out of grid";
        return;
    }

    //DLOG(INFO) << "zmap::GetCellContents :: bucket " << CellID << " type : " << type;

    ObjectListType::iterator it = list->begin();

    std::map<uint32, ObjectStruct*>::iterator mapIt = ZMapCells.find(CellID);

    //make a copy of the list !!!!
    //splice removes the entries!!!
    ObjectListType listCopy;

    if(type&&Bucket_Objects)    {
        listCopy = (*mapIt).second->Objects;
        list->splice(it, listCopy);
    }

    if(type&&Bucket_Players)    {
        listCopy = (*mapIt).second->Players;
        list->splice(it, listCopy);
    }

    if(type&&Bucket_Creatures)    {
        listCopy = (*mapIt).second->Creatures;
        list->splice(it, listCopy);
    }

}



void	zmap::GetPlayerCellContents(uint32 CellID, ObjectListType* list)
{
    //Pesudo
    // 1. Return list of objects in cell
    //if(CellID) >

    if(CellID > (GRIDWIDTH*GRIDHEIGHT))
        return;

    ObjectListType::iterator it = list->begin();

    std::map<uint32, ObjectStruct*>::iterator mapIt = ZMapCells.find(CellID);

    ObjectListType listCopy;

    listCopy = (*mapIt).second->Players;
    list->splice(it, listCopy);

}

//=================================================
//returns Players in chatrange

bool zmap::GetCellValidFlag(uint32 CellID)
{
    if(CellID > (GRIDWIDTH*GRIDHEIGHT))
        return false;
    return true;
}

//=================================================
//returns Players in chatrange

void	zmap::GetChatRangeCellContents(uint32 CellID, ObjectListType* list)
{
    GetPlayerGridContentsListRow(CellID, list);

    for(int i = 1; i <= chatRange; i++)    {
        GetPlayerGridContentsListRow(CellID + (i*GRIDWIDTH), list);
        GetPlayerGridContentsListRow(CellID - (i*GRIDWIDTH), list);
    }

}

//=====================================================
//Get Contents for a Row based on a middle cell
//depending on how far we are away of the player we need to resize the row
//this will be done by the var iteration
void zmap::GetGridContentsListRow(uint32 CellID, ObjectListType* list, uint32 type)
{
    GetCellContents(CellID,list, type);

    for(int i = 1; i <= viewRange; i++)    {
        GetCellContents(CellID + i,list, type);
        GetCellContents(CellID - i,list, type);
    }
}


void	 zmap::GetPlayerGridContentsListRow(uint32 CellID, ObjectListType* list)
{
    GetPlayerCellContents(CellID,list);

    for(int i = 1; i <= viewRange; i++)    {
        GetPlayerCellContents(CellID + i,list);
        GetPlayerCellContents(CellID - i,list);
    }

}


//==========================================================================
//column downwards
//when we move along the edges
///15.6 sch
//
void	zmap::GetPlayerGridContentsListColumnDown(uint32 CellID, ObjectListType* list)
{

    GetPlayerCellContents(CellID, list);

    for(int i = 1; i <= (viewRange*2)-1; i++)    {
        GetPlayerCellContents(CellID - (i*GRIDWIDTH), list);
    }
}


void	zmap::GetGridContentsListColumnDown(uint32 CellID, ObjectListType* list, uint32 type)
{
    GetCellContents(CellID, list, type);

    for(int i = 1; i <= (viewRange*2)-1; i++)    {
        GetCellContents(CellID - (i*GRIDWIDTH), list, type);
    }
}

void	zmap::GetGridContentsListColumnUp(uint32 CellID, ObjectListType* list, uint32 type)
{

    GetCellContents(CellID, list, type);

    for(int i = 1; i <= (viewRange*2)-1; i++)    {
        GetCellContents(CellID + (i*GRIDWIDTH), list, type);
    }
}

void	zmap::GetPlayerGridContentsListColumnUp(uint32 CellID, ObjectListType* list)
{

    GetPlayerCellContents(CellID, list);

    for(int i = 1; i <= (viewRange*2)-1; i++)    {
        GetPlayerCellContents(CellID + (i*GRIDWIDTH), list);
    }

}



//===============================================================0
// when getting content on the edges just spare the *middle* (pointy) cell
void	zmap::GetGridContentsListRowLeft(uint32 CellID, ObjectListType* list, uint32 type)
{

    GetCellContents(CellID, list, type);

    for(int i = 1; i <= ((viewRange*2)-1); i++)    {
        GetCellContents(CellID - i, list, type);
    }

}

void	zmap::GetPlayerGridContentsListRowLeft(uint32 CellID, ObjectListType* list)
{

    GetPlayerCellContents(CellID, list);

    for(int i = 1; i <= ((viewRange*2)-1); i++)    {
        GetPlayerCellContents(CellID - i, list);
    }

}


void	zmap::GetGridContentsListRowRight(uint32 CellID, ObjectListType* list, uint32 type)
{

    GetCellContents(CellID, list, type);

    for(int i = 1; i <= ((viewRange*2)-1); i++)    {
        GetCellContents(CellID + i, list, type);
    }

}

void	zmap::GetPlayerGridContentsListRowRight(uint32 CellID, ObjectListType* list)
{

    GetPlayerCellContents(CellID, list);

    for(int i = 1; i <= ((viewRange*2)-1); i++)    {
        GetPlayerCellContents(CellID + i, list);
    }

}


void	zmap::GetGridContentsListColumn(uint32 CellID, ObjectListType* list, uint32 type)
{

    GetCellContents(CellID, list, type);

    for(int i = 1; i <= viewRange; i++)    {
        GetCellContents(CellID + (i*GRIDWIDTH), list, type);
        GetCellContents(CellID - (i*GRIDWIDTH), list, type);
    }

}

void	zmap::GetPlayerGridContentsListColumn(uint32 CellID, ObjectListType* list)
{

    GetPlayerCellContents(CellID, list);

    for(int i = 1; i <= viewRange; i++)    {
        GetPlayerCellContents(CellID + (i*GRIDWIDTH), list);
        GetPlayerCellContents(CellID - (i*GRIDWIDTH), list);
    }

}

void	zmap::GetViewingRangeCellContents(uint32 CellID, ObjectListType* list, uint32 type)
{
    //gLogger->log(LogManager::DEBUG,"zmap::GetAllViewingRangeCellContents :: bucket %u", CellID);

    GetGridContentsListRow(CellID, list, type);

    for(int i = 1; i <= viewRange; i++)    {
        GetGridContentsListRow(CellID + (i*GRIDWIDTH), list, type);
        GetGridContentsListRow(CellID - (i*GRIDWIDTH), list, type);
    }
}

void	zmap::GetPlayerViewingRangeCellContents(uint32 CellID, ObjectListType* list)
{

    GetPlayerGridContentsListRow(CellID, list);

    for(int i = 1; i <= viewRange; i++)    {
        GetPlayerGridContentsListRow(CellID + (i*GRIDWIDTH), list);
        GetPlayerGridContentsListRow(CellID - (i*GRIDWIDTH), list);
    }

}


// limited to max viewing range for now
//
void	zmap::GetCustomRangeCellContents(uint32 CellID, uint32 range, ObjectListType* list, uint32 type)
{
    // query the grid with the custom range
    // TODO: need any failsafes concerning the cells???

    if(range > VIEWRANGE)
        range = VIEWRANGE;

    GetGridContentsListRow(CellID, list, type);

    for(uint32 i = 0; i <= range; i++)    {
        GetGridContentsListRow(CellID + (i*GRIDWIDTH), list, type);
        GetGridContentsListRow(CellID - (i*GRIDWIDTH), list, type);
    }

}



uint32 zmap::AddObject(Object *newObject)
{

    uint32 finalBucket = _getCellId(newObject->getWorldPosition().x, newObject->getWorldPosition().z);

    //DLOG(INFO) << "zmap::AddObject :: " << newObject->getId() << " bucket " << finalBucket<< " ";

    if(!GetCellValidFlag(finalBucket))    {
        //something fishy here
        assert(false && "zmap::AddObject :: couldnt find grid cell :(");
        return 0xffffffff;
    }

    //already in there
    if(newObject->getGridBucket() == finalBucket)    {
		DLOG(INFO) << "zmap::AddObject :: " << newObject->getId() << " bucket " << finalBucket<< " Object was already in there";
        return finalBucket;
    }

    newObject->setGridBucket(finalBucket);

    ObjectListType* list;

    std::map<uint32, ObjectStruct*>::iterator mapIt = ZMapCells.find(finalBucket);

    switch(newObject->getType())
    {
    case ObjType_Player:
    {
        list = &(*mapIt).second->Players;
    }
    break;

    case ObjType_Creature:
    case ObjType_NPC:
    {
        list = &(*mapIt).second->Creatures;
    }
    break;

    default:
    {
        list = &(*mapIt).second->Objects;
    }
    break;
    }

    list->push_back(newObject);

    return finalBucket;
}

void zmap::UpdateObject(Object *updateObject)
{
    glm::vec3   position;

    //cater for players in cells
    if (updateObject->getParentId())	{
        position = updateObject->getWorldPosition();
    }
    else	{
        position = updateObject->mPosition;
    }

    uint32 newBucket	= getCellId(position.x, position.z);
    uint32 oldBucket	= updateObject->getGridBucket();

    //no need for an update
    if(newBucket == oldBucket)	{
		assert(false && "zmap::UpdateObject :: no movement ");
        return;
    }

    //get out of old bucket
    RemoveObject(updateObject);

    //put into new bucket
    updateObject->setGridBucket(newBucket);

    ObjectListType* list;

    std::map<uint32, ObjectStruct*>::iterator mapIt = ZMapCells.find(newBucket);

    switch(updateObject->getType())
    {
    case ObjType_Player:
    {
        list = &(*mapIt).second->Players;
    }
    break;

    case ObjType_Creature:
    case ObjType_NPC:
    {
        list = &(*mapIt).second->Creatures;
    }
    break;

    default:
    {
        list = &(*mapIt).second->Objects;
    }
    break;
    }

    list->push_back(updateObject);
}
