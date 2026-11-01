-- Tatooine NPC test
-- This script will not start until Zoneserver is ready.

print("Tatooine NPC test");
LuaScriptEngine.WaitMSec(1000);

local MM = require 'script/TatooineNpcTest2'

local myPos = "SL";
local prevPos = "";

-- npcObjects to use in formation.
local npcMembers = { };


local spawnPosX = -1368.0
local defaultYPos = 12
local spawnPosZ = -3725.0

-- LuaScriptEngine.WaitMSec(10000);

-- Create the npc, "an elit sand trooper".


local noOfLines = 1;
local columnWidth = 1;

local noOfNpcsInTheFormation = 1;

local none = 0;

table.insert(npcMembers, noOfNpcsInTheFormation);
table.insert(npcMembers, noOfLines);
table.insert(npcMembers, columnWidth);

-- Insert center point for this formation. This is the position that everything is relative to.
table.insert(npcMembers, spawnPosX);
table.insert(npcMembers, defaultYPos);
table.insert(npcMembers, spawnPosZ);


-- Insert empty leader.
table.insert(npcMembers, none);

-- Insert our stormie
-- local npc = MM.createAndSpawnNpc(47513075899, "", "", spawnPosX, defaultYPos, spawnPosZ);
local npc = MM.createAndSpawnNpc(1, "", "", spawnPosX, defaultYPos, spawnPosZ);
table.insert(npcMembers, npc);

-- print("Starting to move on-man-gang");

while (1) do
	local temp;

	temp = myPos;	
	myPos = MM.executeRoutes(npcMembers, myPos, prevPos);
	prevPos = temp;
end;

