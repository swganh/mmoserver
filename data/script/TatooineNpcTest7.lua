-- Tatooine NPC test
-- This script will not start until Zoneserver is ready.

local MM = require 'Script/TatooineNpcTest2'

-- Script interface.
-- local scriptSupport

 local routeX = { -1276, -1269, -1265, -1264, -1262, -1258, -1263, -1295,   -1301,      -1,    -6.5,     -9,      -14,     -15,     -13,     15,      20,       28,      33,      40,     49,      51,      53,       56,      57,      53, -1373, -1374, -1375, -1345, -1355, -1372, -1369, -1364,   -1361,     -45,     -55,    -55,      -55,     -54,     -48,     -44,     -43,    -39,      -35,     -23,     -17,     -15,      -6,     13,       14,      14,      14,       8,       4,       1, -1302, -1292, -1291, -1291, -1323, -1325, -1351, -1359, -1362,      45,      38,      35,      30,      25}
 local routeY = {    12,    12,    12,    12,    12,    12,    12,    12,      12,     0.6,     0.6,    0.6,      0.6,     0.6,     0.6,    0.6,     0.6,      0.6,     0.6,     0.6,   -0.5,    -0.5,       1,        1,       1,       1,    12,    12,    12,    12,    12,    12,    12,    12,      12,       1,       1,     2.7,     2.7,     2.7,     2.7,     1.6,     1.6,     1.6,     1.6,     1.6,     0.6,     0.6,     0.6,     0.6,     0.6,     0.6,     0.6,     0.6,     0.6,     0.6,    12,    12,    12,    12,    12,    12,    12,    12,    12,       0,       0,       0,       0,      -1}
 local routeZ = { -3652, -3636, -3636, -3643, -3643, -3641, -3617, -3589,   -3588,      76,      76,      79,      80,      70,      67,      62,      62,      62,      59,      48,     39,      31,      25,       20,      14,      12, -3630, -3626, -3611, -3587, -3551, -3540, -3534, -3533,   -3533,       7,      10,      24,      29,      33,      42,      46,      48,      52,      55,      60,      61,      60,      53,      54,      67,      70,      80,      79,      75,      75, -3589, -3590, -3606, -3610, -3631, -3644, -3687, -3689, -3686,      -3,      -5,      -7,       3,       3}
 local Zone   = {     0,     0,     0,     0,     0,     0,     0,       0,     0, 1026825, 1026825, 1026825, 1026826, 1026826, 1026828, 1026828, 1026829, 1026829, 1026830, 1026831, 1026831, 1026831, 1026837, 1026837, 5515495, 5515495,     0,     0,     0,     0,     0,     0,     0,     0,       0, 1026839, 1026838, 1026838, 1026838, 1026836, 1026836, 1026835, 1026834, 1026834, 1026833, 1026833, 1026832, 1026828, 1026828, 1026828, 1026828, 1026827, 1026827, 1026825, 1026825, 1026825,     0,     0,     0,     0,     0,     0,     0,     0,     0, 1028645, 1028645, 1028646, 1028646, 1028647}
 
 -- npcObjects to use in formation.
local npcMembers = { };

local function executeRoutes(npcData, index, currentDest)
	local myPos;	
	
	if currentDest == "CANTINA" then
		index = index + 1;
		if (index < #routeX) then
			if (Zone[index] ~= Zone[index - 1]) and ((Zone[index] == 0) or (Zone[index - 1] == 0)) then
				-- We have to change cell.
				npcMembers[2] = routeX[index];
				npcMembers[3] = routeY[index];
				npcMembers[4] = routeZ[index];
				npcMembers[5] = Zone[index];
				index = index + 1;
			end;
		end;	
		-- print("Move to " .. routeX[index] .. ", " .. routeY[index] .. ", " .. routeZ[index]);	
		MM.moveToZone(npcData, Zone[index], routeX[index], routeY[index], routeZ[index]);	
		-- LuaScriptEngine.WaitMSec(30000);

	elseif currentDest == "BANK" then
		index = index - 1;
		if (index > 1) then 
			if (Zone[index] ~= Zone[index + 1]) and ((Zone[index] == 0) or (Zone[index + 1] == 0))then
				-- We have to change cell.
				npcMembers[2] = routeX[index];
				npcMembers[3] = routeY[index];
				npcMembers[4] = routeZ[index];
				npcMembers[5] = Zone[index];
				index = index - 1;
			end;
		end;		
		-- print("Move to " .. routeX[index] .. ", " .. routeY[index] .. ", " .. routeZ[index]);
		MM.moveToZone(npcData, Zone[index], routeX[index], routeY[index], routeZ[index]);	
		-- LuaScriptEngine.WaitMSec(30000);

	else
		print("Unknown position:" .. currentDest);
		LuaScriptEngine.WaitMSec(5000);
	end
	return index;
end


local index = 1;
local destination = "CANTINA";

-- Create the npc, "an elit imperial security guard".

-- local npc = MM.createAndSpawnNpc(47513075719, "", "", routeX[index], routeY[index], routeZ[index]);
local npc = MM.createAndSpawnNpc(2, "", "", routeX[index], routeY[index], routeZ[index]);
table.insert(npcMembers, npc);
table.insert(npcMembers,routeX[index]);
table.insert(npcMembers,routeY[index]);
table.insert(npcMembers,routeZ[index]);
table.insert(npcMembers,Zone[index]);

-- print("Starting to move officer");

while (1) do
	if (index == 1) then 
		destination = "CANTINA";
		-- print("Setting destination to " .. destination);
		LuaScriptEngine.WaitMSec(30000);
		-- print("Done waiting");

	elseif (index == #routeX) then 
		destination = "BANK";
		-- print("Setting destination to " .. destination);
		LuaScriptEngine.WaitMSec(10000);
		-- print("Done waiting");
	end;
	
	-- print("Calling executeRoutes with index " .. index);
	index = executeRoutes(npcMembers, index, destination);
end;

