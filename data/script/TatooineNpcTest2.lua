-- Tatooine NPC test
-- This script will not start until Zoneserver is ready.

-- testing with public interfaces.
local M = {} -- public interface

-- Script interface.
local scriptSupport

function M.createAndSpawnNpc(npcTypeId, npcFirstName, npcLastName, xSpawnPos, ySpawnPos, zSpawnPos)

	-- Create the npc, "an elit sand trooper".
	local npcId = scriptSupport:npcCreate(npcTypeId);

	-- How do I abort here?
	if npcId == 0 then
		print("Failed creating NPC");
		return 0;
	end;

	-- Get npc-object when ready
	while scriptSupport:objectIsReady(npcId) == false do
		LuaScriptEngine.WaitMSec(100);
	end

	local npcObject = scriptSupport:npcGetObject(npcId);

	-- print("Tatooine-Bestine: NPC " .. npcFirstName .. npcLastName .. " is spawning");
	scriptSupport:npcSpawn(npcObject, npcId, 0, npcFirstName, npcLastName, 0.71, 0.71, xSpawnPos, ySpawnPos, zSpawnPos);
	
	return npcObject;
end;	


local baseUpdateTime = 750
-- local baseUpdateTime = 375

local spawnPosX = -1368.0
local defaultYPos = 12
local spawnPosZ = -3725.0

local wpStartLeftX = -1368.0
local wpStartLeftZ = -3725.0

local wpStartRightX = -1371.0
local wpStartRightZ = -3726.0

local wpFirstGateX = -1368.0
local wpFirstGateZ = -3725.0

local wpOutsideTentX = -1362
local wpOutsideTentZ = -3721

local wpRavinRightX = -1384
local wpRavinRightZ = -3725

local wpRavinLeftX = -1376
local wpRavinLeftZ = -3730

local wpLampLeftX = -1368
local wpLampLeftZ = -3704

local wpLampRightX = -1351
local wpLampRightZ = -3715

local wpOpenAreaX = -1379
local wpOpenAreaZ = -3711


-- local npcLeader		-- We will run a formation

-- npcObjects to use in formation.
local npcMembers = { };

local routeTab = { };
local posTab = { };

-- formula for moving
-- x = new x position - current x position
-- z = new z position - current z position

-- deltaX = x/h , where h is the length to travel.
-- deltaZ = z/h , where h is the length to travel.

-- Adjust the base speed time for any increased length due to the truncation above.
-- updateTime = baseUpdateTime * (h /getIntOf(h))

local xPos = spawnPosX;
local yPos = defaultYPos;
local zPos = spawnPosZ;


-- Functions
local getLength = function(x, z)
	return math.sqrt((x * x) + (z * z));
end

local getIntOf = function(length)
	i, f = math.modf(length);
	return i;
end

local TestValueX = 0;
local TestValueXInc = 0.1;

function moveTo(npcData, newPosX, newPosY, newPosZ, onTheDouble)

	-- print("Moving to " .. newPosX .. ", " .. newPosY .. ", " .. newPosZ);
	
	-- Movement for first line, for now, take data from virtual center point of formation
	
	-- Direction for formation.
	local x = newPosX - npcData[4];
	local y = newPosY - npcData[5];
	local z = newPosZ - npcData[6];
	
	local h = getLength(x, z);
	if (onTheDouble == true) then
		h = getLength(x, z)/2;
	end	
	
	local i = getIntOf(h);
	local deltaX = x/i;
	local deltaY = y/i;
	local deltaZ = z/i;
	local updateTime = baseUpdateTime * (h /i);
	if (onTheDouble == true) then
		updateTime = (baseUpdateTime / 2 ) * (h /i);
	end
	
	local dataSize = 7;
	local lineFirstNpc = dataSize + 1;
	local lineLastNpc = lineFirstNpc + (npcData[3] - 1);
	local noOfLines = npcData[2];
	local colWidth = npcData[3];

	-- Let members in first line have their offset to the formation set.
	local xOffset = -((colWidth - 1.0) / 2.0);
	
	local firstLineUpdated = false;
	
	if (npcData[7] ~= 0) then
		-- Set new direction for the npc leader.
		scriptSupport:npcDirection(npcData[7], x, z);
	else
		-- Update direction and formation position of members in first line		
		for sel = lineFirstNpc, lineLastNpc do
			if (npcData[sel] ~= 0) then	-- we found a npc.
				scriptSupport:npcDirection(npcData[sel], x, z);
				scriptSupport:npcFormationPosition(npcData[sel], xOffset, -0.01);
			end;
			xOffset = xOffset + 1.0;
		end;
	end;

	local total = (colWidth * noOfLines);
	
	for deltas = 1,i do 
		-- Update every line in formation with the data from the line in front of them.
		for count =  dataSize + total, dataSize  + colWidth + 1, -1 do 
			-- clone the positions.
			if (npcData[count] ~= 0) and (npcData[count - colWidth] ~= 0) then
				-- print("count and count - colWidth = "  .. count .. ", " .. count - colWidth);
				scriptSupport:npcClonePosition(npcData[count], npcData[count - colWidth]);
			end
		end;
		
		if (npcData[7] ~= 0) then
			-- Update pos in first line also
			for sel = lineFirstNpc, lineLastNpc do
				if (npcData[sel] ~= 0) then	-- we found a npc.
					scriptSupport:npcFormationMove(npcData[sel], npcData[4], npcData[5], npcData[6]);
				end;
			end;

			-- Update formation position
			npcData[4] = npcData[4] + deltaX;
			npcData[5] = npcData[5] + deltaY;
			npcData[6] = npcData[6] + deltaZ;
		
		
			-- Update leaders position
			scriptSupport:npcFormationMove(npcData[7], npcData[4], npcData[5], npcData[6]);
			
			if (firstLineUpdated == false) then
				firstLineUpdated = true;
				-- Update direction and formation position of members in first line
				for sel = lineFirstNpc, lineLastNpc do
					if (npcData[sel] ~= 0) then	-- we found a npc.
						scriptSupport:npcDirection(npcData[sel], x, z);
						scriptSupport:npcFormationPosition(npcData[sel], xOffset, -0.5);
					end;
					xOffset = xOffset + 1.0;
				end;
			end;

		else
			npcData[4] = npcData[4] + deltaX;
			npcData[5] = npcData[5] + deltaY;
			npcData[6] = npcData[6] + deltaZ;

			-- Update pos in first line also
			for sel = lineFirstNpc, lineLastNpc do
				if (npcData[sel] ~= 0) then	-- we found a npc.
					scriptSupport:npcFormationMove(npcData[sel], npcData[4], npcData[5], npcData[6]);
				end;
			end;
		end;

		LuaScriptEngine.WaitMSec(updateTime);
	end;
end;


function moveToZone(npcData, cellId, newPosX, newPosY, newPosZ)
	local x = newPosX - npcData[2];
	local y = newPosY - npcData[3];
	local z = newPosZ - npcData[4];
	local h = getLength(x, z);
	local i = getIntOf(h);
	local deltaX = x/i;
	local deltaY = y/i;
	local deltaZ = z/i;
	local updateTime = baseUpdateTime * (h /i);

	scriptSupport:npcDirection(npcData[1], x, z);

	for count = 1,i do 
		npcData[2] = npcData[2] + deltaX;
		npcData[3] = npcData[3] + deltaY;
		npcData[4] = npcData[4] + deltaZ;

		scriptSupport:npcMoveToZone(npcData[1], cellId, npcData[2], npcData[3], npcData[4]);
		LuaScriptEngine.WaitMSec(updateTime);
	end;
	npcData[5] = cellId;
end;

function oldMoveTo(npcData, newPosX, newPosY, newPosZ)

	-- print("Moving to " .. newPosX .. ", " .. newPosY .. ", " .. newPosZ);
	
	-- Movement for first line, for now, take data from virtual center point of formation
	
	-- Direction for formation.
	local x = newPosX - npcData[4];
	local y = newPosY - npcData[5];
	local z = newPosZ - npcData[6];
	local h = getLength(x, z);
	local i = getIntOf(h);
	local deltaX = x/i;
	local deltaY = y/i;
	local deltaZ = z/i;
	local updateTime = baseUpdateTime * (h /i);
	
	local dataSize = 6;
	local lineFirstNpc = dataSize + 1;
	local lineLastNpc = lineFirstNpc + (npcData[3] - 1);
	local noOfLines = npcData[2];
	local colWidth = npcData[3];
	
	
	-- Set new direction for the npc's in the front line.
	
	-- Let members in first line have their offset to the formation set.
	local xOffset = -((colWidth - 1.0) / 2.0);
	
	for sel = lineFirstNpc, lineLastNpc do

		-- print("xOffset = " .. xOffset);
		
		if (npcData[sel] ~= 0) then	-- we found a npc.
			scriptSupport:npcDirection(npcData[sel], x, z);
			scriptSupport:npcFormationPosition(npcData[sel], xOffset, -0.01);
		end;
		xOffset = xOffset + 1.0;
	end;

	-- print("formationCenter = "  .. ((colWidth - 1.0) / 2.0));

	local total = (colWidth * noOfLines);
	
	-- print("count = 1,"  .. i);
	
	for deltas = 1,i do 
		-- Update every line in formation with the data from the line in front of them.
		for count =  dataSize + total, dataSize  + colWidth + 1, -1 do 
			-- clone the positions.
			if (npcData[count] ~= 0) and (npcData[count - colWidth] ~= 0) then
				-- print("count and count - colWidth = "  .. count .. ", " .. count - colWidth);
				scriptSupport:npcClonePosition(npcData[count], npcData[count - colWidth]);
			end
		end;
		
		-- Update formation position
		npcData[4] = npcData[4] + deltaX;
		npcData[5] = npcData[5] + deltaY;
		npcData[6] = npcData[6] + deltaZ;
		
		-- Update pos in first line also
		for sel = lineFirstNpc, lineLastNpc do
			if (npcData[sel] ~= 0) then	-- we found a npc.
				scriptSupport:npcFormationMove(npcData[sel], npcData[4], npcData[5], npcData[6]);
			end;
		end;
		LuaScriptEngine.WaitMSec(updateTime);
	end;
end;

-- external interface
M.moveTo = moveTo;
M.moveToZone = moveToZone;

local routeSLtoOT = function(npcs)
	moveTo(npcs, wpOutsideTentX, yPos, wpOutsideTentZ, false);	
end

local routeOTtoSL = function(npcs)
	moveTo(npcs, wpStartLeftX, yPos, wpStartLeftZ, false);	
end
	
local routeOTtoLR = function(npcs)
	moveTo(npcs, wpLampRightX, yPos, wpLampRightZ, false);	
end
		
local routeOTtoLL = function(npcs)
	moveTo(npcs, wpLampLeftX, yPos, wpLampLeftZ, false);	
end

local routeOTtoOA = function(npcs)
	moveTo(npcs, wpOpenAreaX, yPos, wpOpenAreaZ, false);	
end

local routeLRtoLL = function(npcs)
	moveTo(npcs, wpLampLeftX, yPos, wpLampLeftZ, false);	
end

local routeLRtoOA = function(npcs)
	moveTo(npcs, wpOpenAreaX, yPos, wpOpenAreaZ, false);	
end

local routeLRtoOT = function(npcs)
	moveTo(npcs, wpOutsideTentX, yPos, wpOutsideTentZ, false);
end

local routeLLtoLR = function(npcs)
	moveTo(npcs, wpLampRightX, yPos, wpLampRightZ, false);	
end

local routeLLtoOA = function(npcs)
	moveTo(npcs, wpOpenAreaX, yPos, wpOpenAreaZ, false);	
end

local routeLLtoRL = function(npcs)
	moveTo(npcs, wpRavinLeftX, yPos, wpRavinLeftZ, false);	
end

local routeLLtoRR = function(npcs)
	moveTo(npcs, wpRavinRightX, yPos, wpRavinRightZ, false);	
end

local routeOAtoSR = function(npcs)
	moveTo(npcs, wpStartRightX, yPos, wpStartRightZ, false);	
end

local routeOAtoRR = function(npcs)
	moveTo(npcs, wpRavinRightX, yPos, wpRavinRightZ, false);	
end

local routeOAtoLL = function(npcs)
	moveTo(npcs, wpLampLeftX, yPos, wpLampLeftZ, false);	
end

local routeOAtoLR = function(npcs)
	moveTo(npcs, wpLampRightX, yPos, wpLampRightZ, false);	
end

local routeOAtoRL = function(npcs)
	moveTo(npcs, wpRavinLeftX, yPos, wpRavinLeftZ, false);	
end

local routeRLtoSR = function(npcs)
	moveTo(npcs, wpStartRightX, yPos, wpStartRightZ, false);	
end

local routeRLtoRR = function(npcs)
	moveTo(npcs, wpRavinRightX, yPos, wpRavinRightZ, false);	
end

local routeRLtoOA = function(npcs)
	moveTo(npcs, wpOpenAreaX, yPos, wpOpenAreaZ, false);	
end

local routeRLtoLL = function(npcs)
	moveTo(npcs, wpLampLeftX, yPos, wpLampLeftZ, false);	
end


local routeRRtoOA = function(npcs)
	moveTo(npcs, wpOpenAreaX, yPos, wpOpenAreaZ, false);	
end

local routeRRtoLL = function(npcs)
	moveTo(npcs, wpLampLeftX, yPos, wpLampLeftZ, false);	
end

local routeRRtoRL = function(npcs)
	moveTo(npcs, wpRavinLeftX, yPos, wpRavinLeftZ, false);	
end


local routeSRtoOA = function(npcs)
	moveTo(npcs, wpOpenAreaX, yPos, wpOpenAreaZ, false);
end

local routeFromSL = {routeSLtoOT, routeSLtoOT, routeSLtoOT, routeSLtoOT};
local destFromSL = {"OT", "OT", "OT", "OT"};

local routeFromOT = {routeOTtoSL, routeOTtoLR, routeOTtoLL, routeOTtoOA};
local destFromOT = {"SL", "LR", "LL", "OA"};

local routeFromLR = {routeLRtoOT, routeLRtoLL, routeLRtoOA, routeLRtoLL};
local destFromLR = {"OT", "LL", "OA", "LL"};

local routeFromLL = {routeLLtoRL, routeLLtoLR, routeLLtoOA, routeLLtoRR};
local destFromLL = {"RL", "LR", "OA", "RR"};

local routeFromOA = {routeOAtoRL, routeOAtoLR, routeOAtoLL, routeOAtoRR};
local destFromOA = {"RL", "LR", "LL", "RR"};

local routeFromRR = {routeRRtoOA, routeRRtoOA, routeRRtoLL, routeRRtoRL};
local destFromRR = {"OA", "OA", "LL", "RL"};

local routeFromRL = {routeRLtoSR, routeRLtoRR, routeRLtoOA, routeRLtoLL};
local destFromRL = {"SR", "RR", "OA", "LL"};

local routeFromSR = {routeSRtoOA, routeSRtoOA, routeSRtoOA, routeSRtoOA};
local destFromSR = {"OA", "OA", "OA", "OA"};


local routeSelection = function(destTab, prevPos)
	local index = 0;
	while (index == 0) do
		local rand = math.random();
		-- print("Random value is " .. rand );

		if rand <= 0.25 then
			index = 1; 
		elseif rand <= 0.50 then
			index = 2;
		elseif rand <= 0.75 then
			index = 3;
		else
			index = 4;
		end

		if (destTab[index] == prevPos) or (destTab[index] == "") then
			-- We will not go back to where we came from.
			index = 0;
		end;
	end;	
	return index;
end;

function M.executeRoutes(npcs, currentPos, oldPos)
	local index;
	local myPos;	

	if currentPos == "SL" then
		local delay = math.random(10, 60)
		LuaScriptEngine.WaitMSec(delay * 1000);
		index = routeSelection(destFromSL, "");
		myPos = destFromSL[index];
		routeFromSL[index](npcs)

	elseif currentPos == "OT" then
		index = routeSelection(destFromOT, oldPos);
		myPos = destFromOT[index];
		routeFromOT[index](npcs)
	
	elseif currentPos == "LR" then
		index = routeSelection(destFromLR, oldPos);
		myPos = destFromLR[index];
		routeFromLR[index](npcs)

	elseif currentPos == "LL" then
		index = routeSelection(destFromLL, oldPos);
		myPos = destFromLL[index];
		routeFromLL[index](npcs)

	elseif currentPos == "OA" then
		index = routeSelection(destFromOA, oldPos);
		myPos = destFromOA[index];
		routeFromOA[index](npcs)

	elseif currentPos == "RR" then
		index = routeSelection(destFromRR, oldPos);
		myPos = destFromRR[index];
		routeFromRR[index](npcs)

	elseif currentPos == "RL" then
		index = routeSelection(destFromRL, oldPos);
		myPos = destFromRL[index];
		routeFromRL[index](npcs)

	elseif currentPos == "SR" then
		local delay = math.random(10, 60)
		LuaScriptEngine.WaitMSec(delay * 1000);
		index = routeSelection(destFromSR, "");
		myPos = destFromSR[index];
		routeFromSR[index](npcs)
		
	else
		print("Unknown position:" .. currentPos);
		LuaScriptEngine.WaitMSec(5000);
	end
	return myPos;
end

math.randomseed( os.time() );

-- Get access to script-functionality.
scriptSupport = ScriptSupport:Instance();


return M
