-- Tatooine NPC test
-- This script will not start until Zoneserver is ready.

-- This script file is a placeholder, and free to be used for any testing.


-- Start of new code
local script = LuaScriptEngine.getScriptObj();
local SE = ScriptEngine:Init();

local scriptSupport = ScriptSupport:Instance();

scriptSupport:lairSpawn(1);
print("LUA spawned a random lair");

scriptSupport:lairSpawn(2);
print("LUA spawned a random lair");
-- End of new code


-- local script = LuaScriptEngine.getScriptObj();
-- local SE = ScriptEngine:Init();

-- local scriptSupport = ScriptSupport:Instance();

local function createAndReSpawnNpc(npcTypeId, npcFirstName, npcLastName, cellId, respawnPeriod, ySpawnDir, wSpawnDir, xSpawnPos, ySpawnPos, zSpawnPos)

	-- print("Attempt to create a WompRat");
	
	-- Create the npc, of type "npcTypeId".
	local npcId = scriptSupport:npcCreate(npcTypeId);

	if npcId == 0 then
		print("Failed creating NPC");
		return 0, 0;
	end;
	-- print("Created NPC OK");

	-- Get npc-object when ready
	local watchDog = 10000;
	while scriptSupport:objectIsReady(npcId) == false do
		LuaScriptEngine.WaitMSec(100);
		if (watchDog > 0) then
			watchDog = watchDog - 100;
		else
			break;
		end;
	end
	
	if (watchDog == 0) then
		print("TIMEOUT WAITING FOR NPC TO BE CREATED");
		return 0, 0;
	end;

	-- print("Getting NPC object");
	local npcObject = scriptSupport:npcGetObject(npcId);

	-- print("Script: NPC " .. npcFirstName .. npcLastName .. " with Id " .. npcId .. " is spawning");
	LuaScriptEngine.WaitMSec(1000);
	scriptSupport:npcSpawnPersistent(npcObject, npcId, cellId, npcFirstName, npcLastName, ySpawnDir, wSpawnDir, xSpawnPos, ySpawnPos, zSpawnPos, respawnPeriod, npcTypeId);

	return npcObject, npcId;

end;	



local function createAndSpawnLair(npcTypeId, cellId, respawnPeriod, ySpawnDir, wSpawnDir, xSpawnPos, ySpawnPos, zSpawnPos, xWidth, zWidth, fixedPosition)

	-- Create the npc, of type "npcTypeId".
	local npcId = scriptSupport:npcCreate(npcTypeId);

	if npcId == 0 then
		print("Failed creating lair");
		return 0, 0;
	end;
	-- print("Created LAIR OK");

	-- Get npc-object when ready
	local watchDog = 10000;
	while scriptSupport:objectIsReady(npcId) == false do
		LuaScriptEngine.WaitMSec(100);
		if (watchDog > 0) then
			watchDog = watchDog - 100;
		else
			break;
		end;
	end
	
	if (watchDog == 0) then
		print("TIMEOUT WAITING FOR LAIR TO BE CREATED");
		return 0, 0;
	end;

	-- print("Getting NPC object");
	local npcObject = scriptSupport:npcGetObject(npcId);

	-- print("Script: LAIR with Id " .. npcId .. " is spawning");
	LuaScriptEngine.WaitMSec(1000);
	scriptSupport:lairSpawn(npcObject, npcId, cellId, ySpawnDir, wSpawnDir, xSpawnPos, ySpawnPos, zSpawnPos, xWidth, zWidth, fixedPosition, respawnPeriod, npcTypeId);

	return npcObject, npcId;

end;	




local function createAndPopulateLoot(itemTypesId, inventoryOwnerId)

	-- Create the item, of type "itemTypesId".
	local itemId = scriptSupport:itemCreate(itemTypesId);

	if itemId == 0 then
		print("Failed creating item");
		return;
	end;

	-- Get item-object when ready
	while scriptSupport:objectIsReady(itemId) == false do
		LuaScriptEngine.WaitMSec(100);
	end
	
	-- Script support will not be the owner of this item....
	scriptSupport:eraseObject(itemId);
	
	print("Script: Item with Id " .. itemId .. " is created and stored in inventory of owner" ..  inventoryOwnerId);
	scriptSupport:itemPopulateInventory(itemId, inventoryOwnerId);
	
end;	

LuaScriptEngine.WaitMSec(6000);

local wompratTypeId = 101;
local npcWompRat;
local npcWompRatId;
local wompRatCellId = 0;
local wompRatRespawnPeriod = 120000;
local wompRatSpawnDirY = 0;
local wompRatSpawnDirW = 1;

local wompRatSpawnPosX = -1603;
local wompRatSpawnPosY = 64;
local wompRatSpawnPosZ = -3519;
npcWompRat, npcWompRatId = createAndReSpawnNpc(wompratTypeId, "", "", wompRatCellId, wompRatRespawnPeriod, wompRatSpawnDirY, wompRatSpawnDirW, wompRatSpawnPosX, wompRatSpawnPosY, wompRatSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


wompRatSpawnDirY = 1;
wompRatSpawnDirW = 0;
wompRatRespawnPeriod = wompRatRespawnPeriod;
wompRatSpawnPosX = -1617;
wompRatSpawnPosY = 69;
wompRatSpawnPosZ = -3545;
npcWompRat, npcWompRatId = createAndReSpawnNpc(wompratTypeId, "", "", wompRatCellId, wompRatRespawnPeriod, wompRatSpawnDirY, wompRatSpawnDirW, wompRatSpawnPosX, wompRatSpawnPosY, wompRatSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

wompRatSpawnDirY = 0.707;
wompRatSpawnDirW = 0.707;
wompRatRespawnPeriod = wompRatRespawnPeriod;
wompRatSpawnPosX = -1606;
wompRatSpawnPosY = 61;
wompRatSpawnPosZ = -3564;
npcWompRat, npcWompRatId = createAndReSpawnNpc(wompratTypeId, "", "", wompRatCellId, wompRatRespawnPeriod, wompRatSpawnDirY, wompRatSpawnDirW, wompRatSpawnPosX, wompRatSpawnPosY, wompRatSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

wompRatSpawnDirY = -0.707;
wompRatSpawnDirW = 0.707;
wompRatRespawnPeriod = wompRatRespawnPeriod;
wompRatSpawnPosX = -1612;
wompRatSpawnPosY = 51;
wompRatSpawnPosZ = -3588;
npcWompRat, npcWompRatId = createAndReSpawnNpc(wompratTypeId, "", "", wompRatCellId, wompRatRespawnPeriod, wompRatSpawnDirY, wompRatSpawnDirW, wompRatSpawnPosX, wompRatSpawnPosY, wompRatSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

wompRatSpawnDirY = 0.707;
wompRatSpawnDirW = -0.707;
wompRatRespawnPeriod = wompRatRespawnPeriod;
wompRatSpawnPosX = -1609;
wompRatSpawnPosY = 43;
wompRatSpawnPosZ = -3619;
npcWompRat, npcWompRatId = createAndReSpawnNpc(wompratTypeId, "", "", wompRatCellId, wompRatRespawnPeriod, wompRatSpawnDirY, wompRatSpawnDirW, wompRatSpawnPosX, wompRatSpawnPosY, wompRatSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

wompRatSpawnDirY = -1;
wompRatSpawnDirW = 0;
wompRatRespawnPeriod = wompRatRespawnPeriod;
wompRatSpawnPosX = -1598;
wompRatSpawnPosY = 62;
wompRatSpawnPosZ = -3543;
npcWompRat, npcWompRatId = createAndReSpawnNpc(wompratTypeId, "", "", wompRatCellId, wompRatRespawnPeriod, wompRatSpawnDirY, wompRatSpawnDirW, wompRatSpawnPosX, wompRatSpawnPosY, wompRatSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

wompRatSpawnDirY = 0;
wompRatSpawnDirW = -1;
wompRatRespawnPeriod = wompRatRespawnPeriod;
wompRatSpawnPosX = -1601;
wompRatSpawnPosY = 39;
wompRatSpawnPosZ = -3651;
npcWompRat, npcWompRatId = createAndReSpawnNpc(wompratTypeId, "", "", wompRatCellId, wompRatRespawnPeriod, wompRatSpawnDirY, wompRatSpawnDirW, wompRatSpawnPosX, wompRatSpawnPosY, wompRatSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

wompRatSpawnDirY = 0.707;
wompRatSpawnDirW = 0.707;
wompRatRespawnPeriod = wompRatRespawnPeriod;
wompRatSpawnPosX = -1586;
wompRatSpawnPosY = 37;
wompRatSpawnPosZ = -3678;
npcWompRat, npcWompRatId = createAndReSpawnNpc(wompratTypeId, "", "", wompRatCellId, wompRatRespawnPeriod, wompRatSpawnDirY, wompRatSpawnDirW, wompRatSpawnPosX, wompRatSpawnPosY, wompRatSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

local creatureTemplateId = 106;		-- a rill
local creatureObject;
local creatureId;
local creatureCellId = 0;
local creatureRespawnPeriod = 120000;
local creatureSpawnDirY = 0;
local creatureSpawnDirW = 1;

local creatureSpawnPosX = -1590;
local creatureSpawnPosY = 63;
local creatureSpawnPosZ = -3483;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1610;
creatureSpawnPosY = 63;
creatureSpawnPosZ = -3504;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1568;
creatureSpawnPosY = 61;
creatureSpawnPosZ = -3467;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1552;
creatureSpawnPosY = 64;
creatureSpawnPosZ = -3446;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1527;
creatureSpawnPosY = 72;
creatureSpawnPosZ = -3427;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1502;
creatureSpawnPosY = 81;
creatureSpawnPosZ = -3402;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1472;
creatureSpawnPosY = 99;
creatureSpawnPosZ = -3377;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1458;
creatureSpawnPosY = 108;
creatureSpawnPosZ = -3360;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1444;
creatureSpawnPosY = 113;
creatureSpawnPosZ = -3345;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1428;
creatureSpawnPosY = 116;
creatureSpawnPosZ = -3325;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1411;
creatureSpawnPosY = 122;
creatureSpawnPosZ = -3303;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1398;
creatureSpawnPosY = 129;
creatureSpawnPosZ = -3286;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1379;
creatureSpawnPosY = 116;
creatureSpawnPosZ = -3265;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1379;
creatureSpawnPosY = 110;
creatureSpawnPosZ = -3305;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1391;
creatureSpawnPosY = 94;
creatureSpawnPosZ = -3340;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1422;
creatureSpawnPosY = 85;
creatureSpawnPosZ = -3374;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1460;
creatureSpawnPosY = 77;
creatureSpawnPosZ = -3408;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);


 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1492;
creatureSpawnPosY = 69;
creatureSpawnPosZ = -3427;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);



 -- creatureTemplateId = 47513085693;		-- a rill
creatureCellId = 0;
creatureRespawnPeriod = 120000;
creatureSpawnDirY = 0;
creatureSpawnDirW = 1;

creatureSpawnPosX = -1523;
creatureSpawnPosY = 67;
creatureSpawnPosZ = -3445;
creatureObject, creatureId = createAndReSpawnNpc(creatureTemplateId, "", "", creatureCellId, creatureRespawnPeriod, creatureSpawnDirY, creatureSpawnDirW, creatureSpawnPosX, creatureSpawnPosY, creatureSpawnPosZ);
-- LuaScriptEngine.WaitMSec(1000);

local count;

-- for count = 25 do 
-- 	scriptSupport:lairSpawn(3);
-- end;

-- for count = 1,25 do 
-- 	scriptSupport:lairSpawn(4);
-- end;

-- a single womp rat lair.
-- local lairSpawnPosX = -1631;
-- local lairSpawnPosY = 33;
-- local lairSpawnPosZ = -4031;
-- local fixedPosition = true;
-- lairObject, lairId = createAndSpawnLair(pileOfRocksTemplateId, lairCellId, lairRespawnPeriod, lairSpawnDirY, lairSpawnDirW, lairSpawnPosX, lairSpawnPosY, lairSpawnPosZ, lairXwidth, lairYwidth, fixedPosition);


-- a single rill lair.
-- local lairSpawnPosX = -1377;
-- local lairSpawnPosY = 60;
-- local lairSpawnPosZ = -4235;
-- local fixedPosition = true;
-- lairObject, lairId = createAndSpawnLair(nestTemplateId, lairCellId, lairRespawnPeriod, lairSpawnDirY, lairSpawnDirW, lairSpawnPosX, lairSpawnPosY, lairSpawnPosZ, lairXwidth, lairYwidth, fixedPosition);













-- local npcDarthVaderTypeId = 47513085691;
-- local npcDarthVader;
-- local npcDarthVaderId;
-- npcDarthVader, npcDarthVaderId = createAndSpawnNpc(npcDarthVaderTypeId, "", "", 0, 0, 1, -1279, 12, -3612);


-- 89 equals a melon :)
-- createAndPopulateLoot(89, npcWompRatId, playerId);
	

