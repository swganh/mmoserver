-- Tatooine NPC test
-- This script will not start until Zoneserver is ready.

local MM = require 'Script/TatooineNpcTest2'

local routeX = { -1445, -1457, -1458, -1422, -1371, -1345, -1333, -1276, -1244, -1236, -1250, -1269, -1286, -1318, -1312, -1321, -1347, -1371, -1401, -1401, -1396, -1385, -1381, -1367, -1364, -1356, -1350, -1342, -1343, -1339, -1340, -1370}
local routeY = {    10,    10,    10,    10,    12,    12,    12,    12,    12,    12,    12,    12,    12,    12,    12,    12,    12,    12,    10,    10,    10,    10,    10,    17,    26,    26,    26,    26,    26,    26,    26,    27}
local routeZ = { -3799, -3800, -3778, -3778, -3718, -3722, -3706, -3703, -3692, -3665, -3623, -3609, -3611, -3634, -3669, -3685, -3691, -3718, -3752, -3761, -3769, -3780, -3812, -3837, -3860, -3883, -3888, -3913, -3927, -3934, -3970, -4011}

local function executeRoutes(npcData, index, currentDest)
	local myPos;	
	
	if currentDest == "VILLAGE" then
		index = index + 1;
		MM.moveTo(npcData, routeX[index], routeY[index], routeZ[index], false);	
		

	elseif currentDest == "CANTINA" then
		index = index - 1;
		MM.moveTo(npcData, routeX[index], routeY[index], routeZ[index], false);	

	else
		print("Unknown position:" .. currentDest);
		LuaScriptEngine.WaitMSec(5000);
	end
	return index;
end

-- npcObjects to use in formation.
local npcMembers = { };

local index = 17;
local destination = "VILLAGE";

-- Program start	
-- Tatooine NPC formation test

-- Get access to script-functionality.
-- scriptSupport = ScriptSupport:Instance();

-- Script interface.
local scriptSupport = ScriptSupport:Instance();

-- Create the npc, "an elit sand trooper".


local noOfLines = 1;
local columnWidth = 2;

local noOfNpcsInTheFormation = 2;

local none = 0;
LuaScriptEngine.WaitMSec(45000);

table.insert(npcMembers, noOfNpcsInTheFormation);
table.insert(npcMembers, noOfLines);
table.insert(npcMembers, columnWidth);

-- Insert center point for this formation. This is the position that everything is relative to.
table.insert(npcMembers, routeX[index]);
table.insert(npcMembers, routeY[index]);
table.insert(npcMembers, routeZ[index]);

-- Insert the leader.
-- local npc = MM.createAndSpawnNpc(47513076511, "", "", routeX[index], routeY[index], routeZ[index]);
-- table.insert(npcMembers, npc);
table.insert(npcMembers, none);

-- Create the formation
local npc
for count = 1,(noOfLines * columnWidth) do
	-- npc = MM.createAndSpawnNpc(47513075899, "", "", routeX[index], routeY[index], routeZ[index]);
	npc = MM.createAndSpawnNpc(1, "", "", routeX[index], routeY[index], routeZ[index]);
	table.insert(npcMembers, npc);
end

-- print("Starting to move plutoon");

while (1) do
	if (index == 1) then 
		destination = "VILLAGE";
		LuaScriptEngine.WaitMSec(30000);

	elseif (index == #routeX) then 
		destination = "CANTINA";
		LuaScriptEngine.WaitMSec(30000);
	end;

	index = executeRoutes(npcMembers, index, destination);
end;

