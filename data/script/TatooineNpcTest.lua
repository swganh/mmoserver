-- Tatooine NPC test
-- This script will not start until Zoneserver is ready.

-- math.pi

local MM = require 'script/TatooineNpcTest2'

local baseUpdateTime = 750

-- local spawnPosX = -1197
local spawnPosX = -1325.0

local defaultYPos = 12
-- local spawnPosZ = -3561
local spawnPosZ = -3657.0

local spawnPos1X = -1212
local spawnPos1Z = -3557

local spawnPos2X = -1221
local spawnPos2Z = -3567

local spawnPos3X = -1208
local spawnPos3Z = -3578.5

local spawnPos4X = -1220
local spawnPos4Z = -3593

local spawnPos5X = -1231
local spawnPos5Z = -3606

local spawnPos6X = -1219
local spawnPos6Z = -3615

local spawnPos7X = -1209
local spawnPos7Z = -3615

-- Turn place
local spawnPos8X = -1205
local spawnPos8Z = -3612

local spawnPos9X = -1220
local spawnPos9Z = -3634

local spawnPos10X = -1229
local spawnPos10Z = -3644

local spawnPos11X = -1284
local spawnPos11Z = -3617

local spawnPos12X = -1365
local spawnPos12Z = -3723

local spawnPos13X = -1371
local spawnPos13Z = -3747


local spawnPosA1X = -1211
local spawnPosA1Z = -3603

local spawnPosA2X = -1215
local spawnPosA2Z = -3604

local spawnPosA3X = -1222
local spawnPosA3Z = -3613

local spawnPosA4X = -1246
local spawnPosA4Z = -3621

local spawnPosA5X = -1275
local spawnPosA5Z = -3600

local spawnPosA6X = -1275
local spawnPosA6Z = -3596

local spawnPosA7X = -1267
local spawnPosA7Z = -3580

local spawnPosA8X = -1289
local spawnPosA8Z = -3592

local spawnPosA9X = -1288
local spawnPosA9Z = -3566

local spawnPosA10X = -1243
local spawnPosA10Z = -3551


local spawnPosB1X = -1413
local spawnPosB1Z = -3770

local spawnPosB2X = -1427
local spawnPosB2Z = -3767.5

local spawnPosB3X = -1417
local spawnPosB3Z = -3774

local spawnPosB4X = -1414
local spawnPosB4Z = -3737

local spawnPosB5X = -1345
local spawnPosB5Z = -3687

local spawnPosB6X = -1348
local spawnPosB6Z = -3666

local spawnPosB7X = -1318
local spawnPosB7Z = -3663


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

local ss
local npc

local routeTab = { };
local posTab = { };

-- Functions
local getLength = function(x, z)
	return math.sqrt((x * x) + (z * z));
end

local getIntOf = function(length)
	i, f = math.modf(length);
	return i;
end

local moveTo = function(newPosX, newPosY, newPosZ)
	local x = newPosX - xPos;
	local z = newPosZ - zPos;
	local h = getLength(x, z);
	local i = getIntOf(h);

	local deltaX = x/i;
	local deltaZ = z/i;
	local updateTime = baseUpdateTime * (h /i);
	
	ss:npcDirection(npc, x, z);
	
	-- print("Tatooine test NPC is moving a distance of " .. (h /i) .. " m every" .. updateTime .. " ms");
	
	for count = 1,i do 
		xPos = xPos + deltaX;
		zPos = zPos + deltaZ;
		-- print("Moving to " .. xPos .. ", " .. zPos)
		ss:npcMove(npc, xPos , yPos, zPos );
		LuaScriptEngine.WaitMSec(updateTime);
	end;
end;

routeA6ToB2 = function()
	moveTo(spawnPosB1X, yPos, spawnPosB1Z);	
	moveTo(spawnPosB2X, yPos, spawnPosB2Z);	
end;

routeB2ToA6 = function()
	moveTo(spawnPosB3X, yPos, spawnPosB3Z);
	moveTo(spawnPosB4X, yPos, spawnPosB4Z);
	moveTo(spawnPosB5X, yPos, spawnPosB5Z);
	moveTo(spawnPosB6X, yPos, spawnPosB6Z);
	moveTo(spawnPosB7X, yPos, spawnPosB7Z);
	moveTo(spawnPosA6X, yPos, spawnPosA6Z);
end;

routeA6ToA9 = function()
	moveTo(spawnPosA8X, yPos, spawnPosA8Z);
	moveTo(spawnPosA9X, yPos, spawnPosA9Z);
end;

routeA6ToA7 = function()
	moveTo(spawnPosA7X, yPos, spawnPosA7Z);
end;

routeA6ToX5 = function()
	moveTo(spawnPosA5X, yPos, spawnPosA5Z);
	moveTo(spawnPosA4X, yPos, spawnPosA4Z);
	moveTo(spawnPos5X, yPos, spawnPos5Z);
end;

routeA9ToA6 = function()
	moveTo(spawnPosA8X, yPos, spawnPosA8Z);
	moveTo(spawnPosA6X, yPos, spawnPosA6Z);
end;

routeA7ToA9 = function()
	moveTo(spawnPosA9X, yPos, spawnPosA9Z);
end;

routeA9ToA7 = function()
	moveTo(spawnPosA7X, yPos, spawnPosA7Z);
end;

routeA7ToA6 = function()
	moveTo(spawnPosA6X, yPos, spawnPosA6Z);
end;

routeA7ToX2 = function()
	moveTo(spawnPosA10X, yPos, spawnPosA10Z);
	moveTo(spawnPos2X, yPos, spawnPos2Z);
end;

routeX2ToX1 = function()
	moveTo(spawnPos1X, yPos, spawnPos1Z);
end;

routeX1ToX2 = function()
	moveTo(spawnPos2X, yPos, spawnPos2Z);
end;
	
routeX2ToX5 = function()
	moveTo(spawnPos3X, yPos, spawnPos3Z);
	moveTo(spawnPos4X, yPos, spawnPos4Z);
	moveTo(spawnPos5X, yPos, spawnPos5Z);
end;
	
routeX2ToA7 = function()
	moveTo(spawnPosA10X, yPos, spawnPosA10Z);
	moveTo(spawnPosA7X, yPos, spawnPosA7Z);
end;
	
routeX5ToX8 = function()
	moveTo(spawnPos6X, yPos, spawnPos6Z);
	moveTo(spawnPos7X, yPos, spawnPos7Z);
	moveTo(spawnPos8X, yPos, spawnPos8Z);
end;

routeX5ToX2 = function()
	moveTo(spawnPos4X, yPos, spawnPos4Z);
	moveTo(spawnPos3X, yPos, spawnPos3Z);
	moveTo(spawnPos2X, yPos, spawnPos2Z);
end;

routeX5ToA6 = function()
	moveTo(spawnPosA4X, yPos, spawnPosA4Z);
	moveTo(spawnPosA5X, yPos, spawnPosA5Z);
	moveTo(spawnPosA6X, yPos, spawnPosA6Z);
end;

-- New
routeX8ToX5 = function()
	moveTo(spawnPosA1X, yPos, spawnPosA1Z);
	moveTo(spawnPosA2X, yPos, spawnPosA2Z);
	moveTo(spawnPosA3X, yPos, spawnPosA3Z);
	moveTo(spawnPos5X, yPos, spawnPos5Z);
end;

routeX8ToB2 = function()
	moveTo(spawnPos9X, yPos, spawnPos9Z);
	moveTo(spawnPos10X, yPos, spawnPos10Z);
	moveTo(spawnPos11X, yPos, spawnPos11Z);
	moveTo(spawnPos12X, yPos, spawnPos12Z);
	moveTo(spawnPos13X, yPos, spawnPos13Z);
	moveTo(spawnPosB1X, yPos, spawnPosB1Z);	
	moveTo(spawnPosB2X, yPos, spawnPosB2Z);	
end;


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

-- Program start	
-- Tatooine NPC test

math.randomseed( os.time() );

ss = ScriptSupport:Instance();
-- LuaScriptEngine.WaitMSec(45000);

-- Create the npc
-- local npcId = ss:npcCreate(47513075777);

-- if npcId == 0 then
-- 	print("Failed creating NPC");
-- end;

-- while ss:objectIsReady(npcId) == false do
-- 	LuaScriptEngine.WaitMSec(1000);
-- end

-- npc = ss:npcGetObject(npcId);

-- print("Tatooine-Bestine: an ace Imperial storm commando is spawning");
-- ss:npcSpawn(npc, npcId, 0, "", "", 0.71, 0.71, xPos, yPos, zPos)


local routeFromB2 = {routeB2ToA6};
local destFromB2 = {"A6", "", "", ""};

local routeFromA6 = {routeA6ToA7, routeA6ToA9, routeA6ToX5, routeA6ToB2};
local destFromA6 = {"A7", "A9", "X5", "B2"};

local routeFromA7 = {routeA7ToA6, routeA7ToA9, routeA7ToX2};
local destFromA7 = {"A6", "A9", "X2", ""};

local routeFromA9 = {routeA9ToA6, routeA9ToA7};
local destFromA9 = {"A6", "A7", "", ""};

local routeFromX5 = {routeX5ToA6, routeX5ToX2, routeX5ToX8};
local destFromX5 = {"A6", "X2", "X8", ""};

local routeFromX8 = {routeX8ToX5, routeX8ToB2, routeX8ToX5, routeX8ToX5 };
local destFromX8 = {"X5", "B2", "X5", "X5"};

local routeFromX2 = {routeX2ToX5, routeX2ToX1, routeX2ToA7, routeX2ToA7};
local destFromX2 = {"X5", "X1", "A7", "A7"};

local routeFromX1 = {routeX1ToX2};
local destFromX1 = {"X2", "", "", ""};

-- npc = MM.createAndSpawnNpc(47513075777, "", "", xPos, yPos, zPos);
npc = MM.createAndSpawnNpc(3, "", "", xPos, yPos, zPos);

-- print("Starting to move that crackdown_storm_commando_hard");

moveTo(spawnPosB1X, yPos, spawnPosB1Z);
moveTo(spawnPosB2X, yPos, spawnPosB2Z);

local myPos = "B2"
local prevPos = "A6";	-- kind of....

while (1) do
	local index;	

	if myPos == "B2" then
		index = routeSelection(destFromB2, "");
		prevPos = myPos;	
		myPos = destFromB2[index];
		routeFromB2[index]()

		-- one way only, to A6.
	
	elseif myPos == "A6" then
		index = routeSelection(destFromA6, prevPos);
		-- prevPos = myPos;
		prevPos = "";		
		myPos = destFromA6[index];
		routeFromA6[index]()
	
	elseif myPos == "A7" then
		-- index = routeSelection(destFromA7, "X2");
		index = routeSelection(destFromA7, prevPos);
		prevPos = myPos;	
		myPos = destFromA7[index];
		routeFromA7[index]()

	elseif myPos == "A9" then
		index = routeSelection(destFromA9, "");
		prevPos = myPos;	
		myPos = destFromA9[index];
		routeFromA9[index]()

	elseif myPos == "X5" then
		index = routeSelection(destFromX5, prevPos);
		prevPos = myPos;	
		myPos = destFromX5[index];
		routeFromX5[index]()

	elseif myPos == "X8" then
		index = routeSelection(destFromX8, "");
		prevPos = myPos;	
		myPos = destFromX8[index];
		routeFromX8[index]()

	elseif myPos == "X2" then
		index = routeSelection(destFromX2, prevPos);
		prevPos = myPos;	
		myPos = destFromX2[index];
		routeFromX2[index]()

	elseif myPos == "X1" then
		index = routeSelection(destFromX1, "");
		prevPos = myPos;	
		myPos = destFromX1[index];
		routeFromX1[index]()
	else
		print("Unknown position:" .. myPos);
		LuaScriptEngine.WaitMSec(5000);
	end
end

