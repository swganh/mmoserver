-- TutorialPart_1_9

-- Get access to Tutorial via ScriptEngine
local script = LuaScriptEngine.getScriptObj();
local SE = ScriptEngine:Init();
local tutorial = SE:getTutorial(script);

-- wait for client to become ready.
while tutorial:getReady() == false do
	LuaScriptEngine.WaitMSec(500)
end

-- We will start to use general routines from ScriptSupport,
-- aiming at making as few special Tutorial-functions as possible.
local scriptSupport = ScriptSupport:Instance();
local playerId = tutorial:getPlayer();

-- What the player does is recorded from here on, before anything else is
-- set up: a player may act within the first second (tutorial/progress.lua).
local Progress = require 'script/tutorial/progress'
local progress = Progress.new(tutorial, playerId)

local function createAndSpawnNpc(npcTypeId, npcFirstName, npcLastName, cellId, ySpawnDir, wSpawnDir, xSpawnPos, ySpawnPos, zSpawnPos)

	-- Create the npc, of type "npcTypeId".
	local npcId = scriptSupport:npcCreate(npcTypeId);

	if npcId == 0 then
		print("Failed creating NPC");
		return 0, 0;
	end;

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

	local npcObject = scriptSupport:npcGetObject(npcId);

	-- print("Tutorial: NPC " .. npcFirstName .. npcLastName .. " with Id " .. npcId .. " is spawning");
	LuaScriptEngine.WaitMSec(1000);
	scriptSupport:npcSpawnPrivate(npcObject, npcId, playerId, cellId, npcFirstName, npcLastName, ySpawnDir, wSpawnDir, xSpawnPos, ySpawnPos, zSpawnPos);

	return npcObject, npcId;
end;	

function createAndPopulateLoot(itemTypesId, inventoryOwnerId, playerId)

	-- Create the item, of type "itemTypesId".
	local itemId = scriptSupport:itemCreate(itemTypesId);

	-- How do I abort here?
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
	
	-- print("Tutorial: Item with Id " .. itemId .. " is created and stored in inventory of owner" ..  inventoryOwnerId);
	
	scriptSupport:itemPopulateInventory(itemId, inventoryOwnerId, playerId);
end;	


-- TODO npc = MM.createAndSpawnNpc(47513075899, "", "", routeX[index], routeY[index], routeZ[index]);

-- We will start to use general routines from ScriptSupport,
-- aiming at making as few special Tutorial-functions as possible.
-- local scriptSupport = ScriptSupport:Instance();


-- Verify correct state
local state = tutorial:getState();

tutorial:disableHudElement("all");	-- Disable all hud elements
tutorial:enableHudElement("buttonbar");	-- Enable buttonbar

LuaScriptEngine.WaitMSec(1000);

-- Let's create what we need.

-- Create the drum, a container with default equipment.
-- local containerId = scriptSupport:containerCreate(11);		-- Yes, 11 is the type of container we want :)

-- Get object when ready
-- while scriptSupport:objectIsReady(containerId) == false do
-- 	LuaScriptEngine.WaitMSec(500);
-- end
-- local containerObject = scriptSupport:containerGetObject(containerId);
-- scriptSupport:containerSpawn(containerObject, containerId, playerId, 2203318222961, "", 0.71, 0.71, 19, 0, -23);

-- Lets populate the Drum

-- Static container
-- local containerId = 9815512;
local containerId = 2533274790395904;
local itemRoomOfficerId = 47513079097;
local bankAndBazzarRoomOfficerId = 47513079099;
local bazzarTerminalId = 4294968325;
local bankTerminalId = 4294968324;

local npcUserBark1Id = 47513085649;
local npcUserBark2Id = 47513085651;
local npcUserBark3Id = 47513085653;

local cloningAndInsuranceDroidId = 47513085659;
local cloningTerminalId = 4294968326;
local insuranceTerminalId = 4294968327;

local npcShoutPanic = 47513085661;

local npcNervousGuy = 47513085663;
local npcCovardOfficer = 47513085665;

-- local npcDebrisTypeId = 47513085667;
local npcDebrisTypeId = 4;
local cellDebris = 2203318222973;
local yDirDebris = 0;
local wDirDebris = 1;
local xPosDebris = 76.9;
local yPosDebris = -4.0;
local zPosDebris = -94.3;

local npcBanditTypeId = 5;
-- local npcBanditTypeId = 47513085669;
local cellBandit = 2203318222967;
local yDirBandit = 0.707;
local wDirBandit = 0.707;
local xPosBandit = 38.1;
local yPosBandit = -5.9;
local zPosBandit = -113.4;

local celeb_guy1_Id = 47513085677;
local celeb_guy2_Id = 47513085675;
local cellTrainerRoom = 2203318222968;

local refugee_guy1_id = 47513085679;
local refugee_guy2_id = 47513085671;
local refugee_guy3_id = 47513085673;
local npcOfficerWithTrainerId = 47513085681;

local cellMissionRoom = 2203318222969;
local npcOfficerInMissionRoomId = 47513085683;
local npcCommonerInMissionRoomId = 47513079101;
local missionTerminalId = 4294968328;

local cellTravelRoom = 2203318222970;
local npcQuartermasterId = 47513085685;
local travelTerminalId = 4294968329;


local npcSkillTrainer;
local npcSkillTrainerId;

local yDirSkillTrainer = -0.220;
local wDirSkillTrainer = 0.975;
local xPosSkillTrainer = 7.1;
local yPosSkillTrainer = -4.2;
local zPosSkillTrainer = -128.2;

local npcSkillTrainerTypeId;
local cellSkillTrainer = 2203318222968;

-- Wait for tutorial to get valid state
while tutorial:getState() == 0 do
	LuaScriptEngine.WaitMSec(500);
	print("Waiting for Tutorial to become ready...");
end;

if tutorial:getRoom() == 16 then
	state = 3;
end

local npcDebris;
local npcDebrisId;
if (tutorial:getSubState() < 18) then
	-- Create the Debris
	npcDebris, npcDebrisId = createAndSpawnNpc(npcDebrisTypeId, "", "", cellDebris, yDirDebris, wDirDebris, xPosDebris, yPosDebris, zPosDebris);
end;

if (tutorial:getSubState() > 19) then
	-- Create and spawn the skill trainer.

	-- spawn your dedicated skill trainer.
	npcSkillTrainerTypeId = tutorial:getSkillTrainerTypeId();

	if (npcSkillTrainerTypeId ~= 0) then
		npcSkillTrainer, npcSkillTrainerId = createAndSpawnNpc(npcSkillTrainerTypeId, "", "", cellSkillTrainer, yDirSkillTrainer, wDirSkillTrainer, xPosSkillTrainer, yPosSkillTrainer, zPosSkillTrainer);
		-- print("Spawned NPC");			
	else
		-- Invalid skill trainer type id, we have to abort the script.
		print("Failed spawning NPC");
		state = 2;
	end;

	-- Activate the npc that continue to taunt us.
		
	tutorial:setTutorialCelebTaunts(celeb_guy1_Id);
	tutorial:setTutorialCelebTaunts(celeb_guy2_Id);
		
	tutorial:setTutorialRefugeeTaunts(refugee_guy1_id);
	tutorial:setTutorialRefugeeTaunts(refugee_guy2_id);
	tutorial:setTutorialRefugeeTaunts(refugee_guy3_id);
end;



-- Parts one and two (sub-states 1 to 9), on events: each step first checks
-- whether the player has already done it, so a player who is ahead gets no
-- prompt for it, and a player who is slow gets reminders. Prompts never
-- talk over each other. Sub-states are saved as before, so a reconnect
-- resumes at the step it left.

-- isContainerEmpty only answers for the container named here.
tutorial:enableItemContainerEvent(containerId);

local function room()
	return tutorial:getRoom();
end

local function distanceTo(objectId)
	return tutorial:getPlayerPosToObject(objectId);
end

-- The step being run. When the sub-state moves past it (a gate, or C++),
-- its waits end and its prompts are skipped.
local currentStep = nil;
progress.cancel = function()
	return currentStep ~= nil and tutorial:getSubState() > currentStep;
end

-- Moves the tutorial to sub-state n unless C++ or the player already went
-- further (it never goes back); true when step n still has to be done.
local function todo(n)
	local current = tutorial:getSubState();
	if current < n then
		tutorial:setSubState(n);
		current = n;
	end
	if current == n then
		currentStep = n;
		return true;
	end
	return false;
end

local function partsOneAndTwo()
	local at = todo;

	if at(1) then
		progress:sleep(3000);
		-- "PART ONE OF NINE (movement and chat)", "Welcome to Star Wars Galaxies."
		progress:say("part_1", nil, 1000);
		progress:say("welcome", 1267, 5000);			-- sound/tut_01_welcome.snd
	end

	if at(2) then
		if room() < 2 then
			-- "Use the arrow keys to move." / mouse / look around
			progress:say("movement_keyboard", 3426, 0);	-- sound/tut_02_movement.snd
			progress:say("movement_mouse", nil, 0);
			progress:say("lookaround", nil, 15000);
		end
		if not progress:reported("zoomCamera") then
			-- "Using the mouse-wheel, scroll forward to zoom into your character..."
			progress:say("mousewheel", 1174, 25000);		-- sound/tut_00_camera.snd
			progress:waitFor(function() return progress:reported("zoomCamera") end,
				function() progress:say("mousewheel_repeat", 141, 5000) end,	-- sound/tut_03_scroll_out.snd
				30000, 30000);
		end
	end

	if at(3) then
		tutorial:enableHudElement("chatbox");
		if not progress.chatted then
			-- "This is your chat window..." / "Go ahead and try it..."
			progress:say("chatwindow", 3379, 0);			-- sound/tut_04_chat.snd
			progress:say("chatprompt", nil, 10000);
			progress:waitFor(function() return progress.chatted end,
				function() progress:say("repeatchatprompt", 1294, 5000) end,	-- sound/tut_05_remind_chat.snd
				25000, 30000);
		end
	end

	if at(4) then
		tutorial:enableHudElement("chatbox");
		progress:quiet();
		progress:forget("closeHolocron");
		tutorial:openHolocron();
		-- "This is your Holocron..."
		progress:say("holocube", 1385, 17000);			-- sound/tut_00_holocron.snd
		progress:waitFor(function() return progress:reported("closeHolocron") end);
	end

	if at(5) then
		tutorial:enableHudElement("chatbox");
		if room() < 2 then
			-- "Excellent!  Come through the door and down the hallway there."
			progress:say("move_to_item_room", 1691, 5000);	-- sound/tut_06_excellent.snd
			progress:waitFor(function() return room() >= 2 end,
				function() progress:say("repeat_item_room_prompt", 1363, 5000) end,	-- sound/tut_07_comeon.snd
				30000, 30000);
		end
		-- The officer calls the player over, unless already there.
		local barks = { {20.0, "greeter1_bark1"}, {16.0, "greeter1_bark2"}, {12.0, "greeter1_bark3"} };
		for _, bark in ipairs(barks) do
			if progress.talking[itemRoomOfficerId] or distanceTo(itemRoomOfficerId) <= 12.0 then
				break;
			end
			tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:" .. bark[2]);
			progress:waitFor(function()
				return progress.talking[itemRoomOfficerId] or distanceTo(itemRoomOfficerId) <= bark[1];
			end);
		end
	end

	if at(6) then
		tutorial:enableHudElement("chatbox");
		tutorial:enableNpcConversationEvent(itemRoomOfficerId);
		-- "PART TWO OF NINE (conversing with npcs and inventory)"
		progress:say("part_2", nil, 1000);
		if not progress.talking[itemRoomOfficerId] then
			-- "Move forward and click-and-hold your mouse on the Imperial Officer..."
			progress:say("tut_08", 1550, 8000);			-- sound/tut_08_imperialofficer.snd
			if distanceTo(itemRoomOfficerId) > 10.0 then
				tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:greeting");
			end
			local function targeted()
				return progress.talking[itemRoomOfficerId] or scriptSupport:getTarget(playerId) == itemRoomOfficerId;
			end
			progress:waitFor(targeted,
				function() tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:explain_lookat") end,
				15000, 15000);
		end
		if not progress.talking[itemRoomOfficerId] then
			-- look-at target, radial menu, converse
			progress:say("tut_09", 1330, 5000);			-- sound/tut_09_lookat.snd
			progress:say("tut_10", 2726, 5000);			-- sound/tut_10_radialmenu.snd
			progress:say("tut_11", 3436, 10000);			-- sound/tut_11_converse.snd
			progress:waitFor(function() return progress.talking[itemRoomOfficerId] end);
		end
		if not progress.talked[itemRoomOfficerId] then
			-- "Now you're in a conversation..." / "...select the STOP CONVERSING option."
			progress:say("tut_12", 716, 12000);			-- sound/tut_12_conversation.snd
			progress:say("tut_13", 3009, 14000);			-- sound/tut_13_justtype.snd
			progress:waitFor(function() return progress.talked[itemRoomOfficerId] end);
		end
	end

	if at(7) then
		tutorial:enableHudElement("chatbox");
		local function emptied()
			return tutorial:isContainerEmpty(containerId) or progress.closed[containerId];
		end
		local function opened()
			return progress.opened[containerId] or (progress.taken[containerId] or 0) > 0 or emptied();
		end
		if not opened() then
			scriptSupport:sendFlyText(containerId, playerId, "newbie_tutorial/system_messages", "open_me", 0, 255, 0, 0);
			-- "To open the box just click and hold until you see the radial menu appear."
			progress:say("prompt_open_box", 2008, 5000);		-- sound/tut_14_openbox.snd
			progress:waitFor(function() return opened() or scriptSupport:getTarget(playerId) == containerId end,
				function()
					scriptSupport:sendFlyText(containerId, playerId, "newbie_tutorial/system_messages", "open_me", 0, 255, 0, 1);
					-- "You might find something in the box you need..."
					progress:say("repeat_open_box", 1799, 10000);	-- sound/tut_16_intheboxyouneed.snd
				end, 30000, 30000);
		end
		if not opened() then
			-- "Great!  Now select the Open option to access the contents of the container."
			progress:say("prompt_choose_open", 1332, 8000);	-- sound/tut_15_opencontainer.snd
			progress:waitFor(opened);
		end
		if not emptied() then
			if (progress.taken[containerId] or 0) == 0 then
				-- "You can take these items..." / "Now choose the Pick Up option..."
				progress:say("prompt_take_items", 778, 15000);	-- sound/tut_16_a_youcantake.snd
				if not emptied() and (progress.taken[containerId] or 0) == 0 then
					progress:say("prompt_select_pickup", 3041, 4000);	-- sound/tut_17_pickup.snd
				end
				progress:waitFor(function() return emptied() or (progress.taken[containerId] or 0) > 0 end);
			end
			-- "You've put the item in your inventory."
			progress:say("pickup_complete", 58, 2000);		-- sound/tut_18_inventory.snd
			progress:waitFor(emptied);
		end
	end

	if at(8) then
		tutorial:enableHudElement("chatbox");
		if not progress.ate then
			-- free mouse, ALT, CTRL-I
			progress:say("explain_freemouse", 3416, 0);		-- sound/tut_19_inventory.snd
			progress:say("explain_freemouse_toggle", nil, 0);
			progress:say("explain_inventory", nil, 15000);
		end
		-- Open the inventory, find the food, use it: each prompt only while needed.
		local nudged = nil;
		progress:waitFor(function() return progress.ate end, function()
			local nudge;
			if not progress.inventoryOpen then
				-- "Press CTRL-I to open your inventory, and select one of the food items..."
				nudge = { "repeat_open_inventory", 1664, 8000 };	-- sound/tut_25_openinventory.snd
			elseif not progress.foodExamined then
				-- "Scroll through your inventory until you locate one of the food items..."
				nudge = { "prompt_find_food", 2910, 8000 };		-- sound/tut_20_selectfooditem.snd
			else
				-- "Select the Use option to use this item."
				nudge = { "prompt_use_item", 934, 5000 };		-- sound/tut_21_usefood.snd
			end
			progress:say(nudge[1], nudge[2], nudge[3]);
			nudged = nudge[1];
		end, 1, 20000);
		-- "You used the item on yourself.  Food will help increase your attributes..."
		progress:say("explain_item_used", 2571, 10000);		-- sound/tut_22_attributes.snd

		tutorial:enableHudElement("toolbar");
		if room() < 3 then
			-- "This is your toolbar..." / "You can drag items into your toolbar..."
			progress:say("show_toolbar", 2439, 8000);		-- sound/tut_23_toolbar.snd
			progress:say("tut_00_toolbardrag", 1092, 16000);	-- sound/tut_00_toolbardrag.snd
		end
		if progress.inventoryOpen and room() < 3 then
			-- "Press CTRL-I again to close your inventory window."
			progress:say("close_inventory", 1603, 2000);		-- sound/tut_26_closeinventory.snd
			progress:waitFor(function() return not progress.inventoryOpen or room() >= 3 end);
		end
	end

	if at(9) then
		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");
		if room() < 3 then
			-- "Proceed to the next room."
			progress:say("visit_commerce_room", 2202, 8000);	-- sound/tut_27_proceed.snd
			progress:waitFor(function() return room() >= 3 end,
				function() progress:say("visit_commerce_room", 2202, 8000) end, 30000, 30000);
		end
	end

	progress:quiet();
	todo(10);
end

-- Parts three to nine (sub-states 10 to 27), on events like parts one and
-- two. The sub-states that C++ checks are set as soon as the player reaches
-- the place they matter, never held back by narration:
--   16: the soldier hands over the weapon (QuestGiver sets 17),
--   22: the mission officer gives the release documents (sets 23),
--   25: the Quartermaster takes them (sets 26),
--   27: the oath is sworn; the travel terminal works from here.

-- Rooms in the order they are walked through (room n is cell 959 + n;
-- the debris corridor, room 14, lies between rooms 7 and 8).
local STAGE = { [1] = 1, [2] = 2, [3] = 3, [4] = 4, [5] = 5, [6] = 6, [7] = 7,
                [14] = 8, [8] = 9, [9] = 10, [10] = 11, [11] = 12 };

local function stage()
	return STAGE[room()] or 0;
end

-- The sub-states C++ checks, set the moment the player walks into the room
-- where they matter, whatever the script is still saying elsewhere:
-- the soldier's weapon (16), the release documents (22), and the
-- Quartermaster (25, only once the player has the documents: 23).
local GATES = {
	{ stage = 7, substate = 16, needs = 0 },
	{ stage = 11, substate = 22, needs = 0 },
	{ stage = 12, substate = 25, needs = 23 },
};

progress.onTick = function()
	local current = tutorial:getSubState();
	local here = stage();
	for _, gate in ipairs(GATES) do
		if here == gate.stage and current < gate.substate and current >= gate.needs then
			tutorial:setSubState(gate.substate);
			return;
		end
	end
end

local function posX() return tutorial:getPlayerPosX(); end
local function posY() return tutorial:getPlayerPosY(); end
local function posZ() return tutorial:getPlayerPosZ(); end

local function flyText(objectId, key)
	scriptSupport:sendFlyText(objectId, playerId, "newbie_tutorial/system_messages", key, 0, 255, 0, 0);
end

local function npcSays(npcId, key)
	tutorial:spatialChat(npcId, "@newbie_tutorial/newbie_convo:" .. key);
end

local function npcShouts(npcId, key)
	tutorial:spatialChatShout(npcId, "@newbie_tutorial/newbie_convo:" .. key);
end

local function dead(npcId)
	return npcId == nil or progress.died[npcId] or scriptSupport:npcIsDead(npcId);
end

local function step10()	-- Part 3: banking and shopping (room 3)
	tutorial:enableHudElement("chatbox");
	tutorial:enableHudElement("toolbar");
	tutorial:enableNpcConversationEvent(bankAndBazzarRoomOfficerId);
	-- "PART THREE OF NINE (banking and shopping)"
	progress:say("part_3", nil, 1000);

	local officer = bankAndBazzarRoomOfficerId;
	local function leftRoom() return stage() > 3 or (room() == 3 and posZ() < -19) end
	if not progress.talking[officer] and not leftRoom() then
		npcSays(officer, "banker_greeting");
		-- "Converse with the Imperial Officer"
		progress:say("tut_28", 3149, 5000);		-- sound/tut_28_converse.snd
		local explained = false;
		progress:waitFor(function()
			if not explained and scriptSupport:getTarget(playerId) == officer and not progress.talking[officer] then
				explained = true;
				-- "Do you see the option to converse?..."
				progress:say("tut_11", 3436, 10000);	-- sound/tut_11_converse.snd
			end
			return progress.talking[officer] or leftRoom();
		end, function() progress:say("tut_28", 3149, 5000) end, 30000, 30000);
	end
	progress:waitFor(function() return progress.talked[officer] or leftRoom() end);

	-- The bank and the bazaar: each explained when the player looks at or uses it.
	local bankDone, bazaarDone = false, false;
	local sinceFlyText = 0;
	while not (bankDone and bazaarDone) and not leftRoom() do
		local target = scriptSupport:getTarget(playerId);
		if not bankDone and (target == bankTerminalId or progress.used[bankTerminalId]) then
			bankDone = true;
			-- "Banks let you deposit and withdraw cash on hand..." and the rest.
			progress:say("bank_info_1", 421, 6000);		-- sound/tut_32_bank.snd
			progress:say("bank_info_2", nil, 7000);
			progress:say("bank_info_3", nil, 7000);
			progress:say("bank_info_4", nil, 5000);
		elseif not bazaarDone and (target == bazzarTerminalId or progress.used[bazzarTerminalId]) then
			bazaarDone = true;
			-- "This is an item dispenser..." / galactic bazaar / everything...
			progress:say("bazaar_info_1", 936, 5500);	-- sound/tut_29_itemdispenser.snd
			progress:say("bazaar_info_2", 98, 7000);		-- sound/tut_00_bazaar_tease.snd
			progress:say("bazaar_info_3", nil, 8500);
		end
		if sinceFlyText <= 0 then
			if not bankDone then flyText(bankTerminalId, "bank_flytext") end			-- "*Access the bank here!*"
			if not bazaarDone then flyText(bazzarTerminalId, "bazaar_flytext") end	-- "*Shop!*"
			sinceFlyText = 5000;
		end
		progress:sleep(250);
		sinceFlyText = sinceFlyText - 250;
	end
end

local function step11()	-- Part 4: down the ramp past the insured commoners (room 4)
	tutorial:enableHudElement("chatbox");
	tutorial:enableHudElement("toolbar");
	-- "PART FOUR OF NINE (cloning and insurance)"
	progress:say("part_4", nil, 1000);
	if stage() < 5 then
		-- "Move down the ramp toward the cloning and insurance terminals."
		progress:say("tut_36", 2297, 3000);			-- sound/tut_36_movedownhall.snd
		local barks = {
			{ npcUserBark1Id, "ins_user_bark_1", function() return posY() <= -6.9 end },
			{ npcUserBark2Id, "ins_user_bark_2", function() return posZ() <= -47 end },
			{ npcUserBark3Id, "ins_user_bark_3", function() return posZ() <= -54 or posX() <= 45 end },
		};
		progress:waitFor(function() return stage() >= 4 end);
		for _, bark in ipairs(barks) do
			progress:waitFor(function() return stage() >= 5 or (stage() == 4 and bark[3]()) end);
			if stage() >= 5 then break end
			npcSays(bark[1], bark[2]);
		end
	end
	progress:waitFor(function() return stage() >= 5 end,
		function() progress:say("tut_36", 2297, 3000) end, 30000, 30000);
end

local function step12()	-- Part 4: cloning and insurance (room 5)
	tutorial:enableHudElement("chatbox");
	tutorial:enableHudElement("toolbar");
	tutorial:enableNpcConversationEvent(cloningAndInsuranceDroidId);
	local droid = cloningAndInsuranceDroidId;
	local function movedOn() return stage() > 5 end

	if not (progress.cloned or progress.talking[droid] or movedOn()) then
		npcSays(droid, "clone_greeting");
		progress:waitFor(function() return progress.talking[droid] or progress.cloned or movedOn() end,
			function() npcSays(droid, "clone_prompt_convo") end, 10000, 30000);
	end
	progress:waitFor(function() return progress.talked[droid] or progress.cloned or movedOn() end);

	if not (progress.cloned or movedOn()) then
		flyText(cloningTerminalId, "clone_here");	-- "*Clone!*"
		-- "Use the cloning terminal..."
		progress:say("tut_37", 3245, 5000);			-- sound/tut_37_cloning.snd
		progress:waitFor(function() return progress.cloned or movedOn() end,
			function() flyText(cloningTerminalId, "clone_here") end, 5000, 5000);
	end
	if progress.cloned then
		-- "Your clone data has been saved!"
		progress:say("declare_cloned", nil, 1000);
	end

	if not (progress.insured or movedOn()) then
		npcSays(droid, "convo_3_explain_terminal_1");
		progress:sleep(5000);
		npcSays(droid, "convo_3_explain_terminal_2");
		flyText(insuranceTerminalId, "insure_here");	-- "*Insurance!*"
		-- "Use the insurance terminal."
		progress:say("tut_38", 2678, 3000);			-- sound/tut_38_insurance.snd
		progress:waitFor(function() return progress.insured or movedOn() end,
			function() flyText(insuranceTerminalId, "insure_here") end, 5000, 5000);
	end
	if progress.insured then
		-- "All of your items have been insured!"
		progress:say("declare_insured", nil, 3000);
	end

	if not movedOn() then
		-- "Proceed to the next room."
		progress:say("visit_commerce_room", 2202, 3000);	-- sound/tut_27_proceed.snd
	end
	progress:waitFor(function() return posZ() <= -64 or stage() >= 6 end);
	if stage() <= 6 then
		npcShouts(npcShoutPanic, "shout_panic1");
	end
end

local function step13()	-- Part 4: the radar
	tutorial:enableHudElement("chatbox");
	tutorial:enableHudElement("toolbar");
	progress:waitFor(function() return posZ() <= -73 or stage() >= 6 end);
	-- "If you're going down that hallway, you might want advanced warning... This is your radar."
	progress:say("radar", 2975, 2000);			-- sound/tut_41_advancewarning.snd
	tutorial:enableHudElement("radar");
end

local function step14()	-- Part 4: the panic in the corridor (room 6)
	tutorial:enableHudElement("chatbox");
	tutorial:enableHudElement("toolbar");
	tutorial:enableHudElement("radar");
	progress:waitFor(function() return stage() >= 6 end);
	if stage() == 6 then
		npcShouts(npcShoutPanic, "shout_panic2");
		npcShouts(npcShoutPanic, "shout_panic3");
	end
end

local function step15()	-- Part 5: combat moves and radar; the nervous refugee
	tutorial:enableHudElement("chatbox");
	tutorial:enableHudElement("toolbar");
	tutorial:enableHudElement("radar");
	-- "PART FIVE OF NINE (combat moves and radar)"
	progress:say("part_5", nil, 1000);
	local lines = {
		{ "nervous_guy1", function() return posX() >= 12 end },
		{ "nervous_guy2", function() return posZ() >= -86 and posX() >= 12 end },
		{ "nervous_guy3", function() return posZ() >= -82 and posX() >= 12 end },
	};
	for _, line in ipairs(lines) do
		progress:waitFor(function() return stage() >= 7 or line[2]() end);
		if stage() >= 7 then break end
		npcSays(npcNervousGuy, line[1]);
	end
end

local function step16()	-- Part 5: the soldier who hands over a weapon (room 7)
	tutorial:enableHudElement("chatbox");
	tutorial:enableHudElement("toolbar");
	tutorial:enableHudElement("radar");
	tutorial:enableNpcConversationEvent(npcCovardOfficer);
	local function armed() return tutorial:getSubState() >= 17 end
	if stage() < 7 then
		-- "The red dots show potentially aggressive entities..." / zoom the map
		progress:say("radar_more", 1259, 18000);		-- sound/tut_42_map.snd
		progress:say("tut_43", 2327, 5000);			-- sound/tut_43_zoommap.snd
		progress:waitFor(function() return stage() >= 7 or armed() end);
	end
	if not armed() then
		-- "Converse with the soldier."
		progress:say("tut_40", 957, 3000);			-- sound/tut_40_converse.snd
		progress:waitFor(armed, function() progress:say("tut_40", 957, 3000) end, 30000, 30000);
	end
end

local function step17()	-- Part 5: the debris blocking the hallway
	if npcDebrisId == nil then
		npcDebris, npcDebrisId = createAndSpawnNpc(npcDebrisTypeId, "", "", cellDebris, yDirDebris, wDirDebris, xPosDebris, yPosDebris, zPosDebris);
	end
	tutorial:makeCreatureAttackable(npcDebrisId);
	tutorial:enableHudElement("chatbox");
	tutorial:enableHudElement("toolbar");
	tutorial:enableHudElement("radar");
	tutorial:enableHudElement("hambar");

	if stage() <= 7 and not dead(npcDebrisId) then
		progress:waitFor(function() return posX() >= 45 or stage() > 7 or dead(npcDebrisId) end);
		if stage() <= 7 then
			npcShouts(npcNervousGuy, "nervous_guy4");
		end
	end
	if not dead(npcDebrisId) then
		progress:waitFor(function() return (posZ() <= -80 and posX() >= 72) or stage() > 7 or dead(npcDebrisId) end);
	end
	if not dead(npcDebrisId) then
		-- "The hallway is blocked by debris.  Attack it to destroy it..."
		progress:say("tut_45", 3183, 8000);			-- sound/tut_45_proceed.snd
		progress:waitFor(function() return dead(npcDebrisId) end);
	end
	-- It explodes (KNOWN_ISSUES #5g).
	scriptSupport:scriptSystemMessage(playerId, npcDebrisId, "clienteffect/combat_explosion_lair_large.cef");
	npcDebrisId = nil;
end

local function step18()	-- Part 6: combat with the bandit (room 8)
	tutorial:enableHudElement("all");
	local npcBandit, npcBanditId = createAndSpawnNpc(npcBanditTypeId, "", "", cellBandit, yDirBandit, wDirBandit, xPosBandit, yPosBandit, zPosBandit);
	createAndPopulateLoot(89, npcBanditId, playerId);		-- a melon
	createAndPopulateLoot(35, npcBanditId, playerId);		-- blue milk

	-- Special attacks, the default attack, "PART SIX OF NINE (combat)", attacking.
	progress:say("explain_combat_1", 179, 5000);			-- sound/tut_47_defaultattack.snd
	progress:say("explain_combat_2", nil, 10000);
	progress:say("part_6", nil, 3000);
	progress:say("tut_44_attacking", 1553, 5000);		-- sound/tut_44_attacking.snd

	-- Up to five minutes of fighting; when either side runs low, or time
	-- is up, the bandit throws his grenade, which is defective.
	local combatMaxDuration = 300000;
	local combatTimer = combatMaxDuration;
	while not dead(npcBanditId) do
		if tutorial:isLowHam(npcBanditId, 200) then
			combatTimer = 0;
		end
		if scriptSupport:npcInCombat(npcBanditId) and scriptSupport:npcInCombat(playerId) then
			combatTimer = combatTimer - 250;
			if combatTimer <= 0 then
				tutorial:npcStopFight(npcBanditId);
				npcSays(npcBanditId, "pirate_taunt3");		-- "...A GRENADE!"
				progress:sleep(2000);
				npcSays(npcBanditId, "pirate_taunt4");		-- "...This grenade is defective!  Oh no!"
				progress:sleep(250);
				scriptSupport:scriptSystemMessage(playerId, npcBanditId, "clienteffect/combat_grenade_proton.cef");
				scriptSupport:npcKill(npcBanditId);
			end
		else
			combatTimer = combatMaxDuration;
		end
		-- Don't let the player over the bridge.
		if posX() < 17 and posZ() < -106 then
			scriptSupport:setPlayerPosition(playerId, cellBandit, 19, posY(), -112);
		end
		progress:sleep(250);
	end
	progress:sleep(3000);
	-- "Congratulations!  The pirate has been destroyed.  Now you can loot his corpse..."
	progress:say("loot_pirate", 3705, 2500);			-- sound/tut_00_congratulations.snd
end

local function step19()	-- Part 7: skill training (room 9)
	tutorial:enableHudElement("all");
	if npcSkillTrainerId == nil then
		npcSkillTrainerTypeId = tutorial:getSkillTrainerTypeId();
		if npcSkillTrainerTypeId == 0 then
			print("Tutorial: no skill trainer for this profession");
			return false;
		end
		npcSkillTrainer, npcSkillTrainerId = createAndSpawnNpc(npcSkillTrainerTypeId, "", "", cellSkillTrainer, yDirSkillTrainer, wDirSkillTrainer, xPosSkillTrainer, yPosSkillTrainer, zPosSkillTrainer);
		tutorial:setTutorialCelebTaunts(celeb_guy1_Id);
		tutorial:setTutorialCelebTaunts(celeb_guy2_Id);
		tutorial:setTutorialRefugeeTaunts(refugee_guy1_id);
		tutorial:setTutorialRefugeeTaunts(refugee_guy2_id);
		tutorial:setTutorialRefugeeTaunts(refugee_guy3_id);
	end

	if stage() < 10 then
		-- "Proceed to the next room."
		progress:say("visit_commerce_room", 685, 3000);	-- sound/tut_52_walkdown.snd
		progress:waitFor(function() return stage() >= 10 end,
			function() progress:say("visit_commerce_room", 685, 3000) end, 30000, 30000);
	end

	local officer = npcOfficerWithTrainerId;
	tutorial:enableNpcConversationEvent(officer);
	local function trained() return tutorial:isPlayerTrained() end
	local function movedOn() return stage() > 10 end
	-- "PART SEVEN OF NINE (skill training)"
	progress:say("part_7", nil, 2000);
	if not (progress.talking[officer] or trained() or movedOn()) then
		npcSays(officer, "off_1_greeting");
		-- "Converse with the soldier."
		progress:say("tut_40", 957, 3000);			-- sound/tut_40_converse.snd
		progress:waitFor(function() return progress.talking[officer] or trained() or movedOn() end,
			function() progress:say("tut_40", 957, 3000) end, 30000, 30000);
	end
	progress:waitFor(function() return progress.talked[officer] or trained() or movedOn() end);

	if not trained() then
		tutorial:enableNpcConversationEvent(npcSkillTrainerId);
		npcSays(npcSkillTrainerId, "trainer_grunt");
		-- "The Imperial Officer has directed you to a Skill Trainer..."
		progress:say("tut_49", 3821, 3000);			-- sound/tut_49_skilltrainer.snd
		progress:waitFor(trained, function() progress:say("tut_49", 3821, 3000) end, 60000, 60000);
		progress:waitFor(function() return progress.talked[npcSkillTrainerId] or movedOn() end);
	end
	if not movedOn() then
		-- skills window (CTRL-S); character sheet (CTRL-C)
		progress:say("tut_50", 883, 25000);			-- sound/tut_50_skillbrowser.snd
		progress:say("tut_51", 3773, 12000);			-- sound/tut_51_charactersheet.snd
	end
end

local function step20()	-- Part 7: on to the mission room
	tutorial:enableHudElement("all");
	if stage() < 11 then
		-- "Proceed to the next room."
		progress:say("visit_commerce_room", 685, 3000);	-- sound/tut_52_walkdown.snd
		progress:waitFor(function() return stage() >= 11 end,
			function() progress:say("visit_commerce_room", 685, 3000) end, 30000, 30000);
	end
end

local function step22()	-- Part 8: missions and waypoints (room 10); the release documents
	tutorial:enableHudElement("all");
	local officer = npcOfficerInMissionRoomId;
	tutorial:enableNpcConversationEvent(officer);
	local function documents() return tutorial:getSubState() >= 23 end

	-- "PART EIGHT OF NINE (missions and waypoints)"
	progress:say("part_8", nil, 3000);
	if not (progress.talking[officer] or documents()) then
		-- mission terminals, waypoints, NPCs with tasks
		progress:say("tut_53", 3591, 10000);			-- sound/tut_53_missions.snd
		progress:say("tut_55_waypoints", 2887, 10000);	-- sound/tut_55_waypoints.snd
		progress:say("tut_54", 131, 2000);			-- sound/tut_54_npcmission.snd
		tutorial:npcSendAnimation(npcCommonerInMissionRoomId, 7, officer);
	end

	local onTerminal = false;
	progress:waitFor(function()
		-- Looking at the mission terminal explains why it can't be used here.
		local looking = scriptSupport:getTarget(playerId) == missionTerminalId;
		if looking and not onTerminal then
			-- "You cannot take a mission from this terminal..."
			progress:say("mission_terminal", 3397, 5000);	-- sound/tut_00_mission_terminal.snd
		end
		onTerminal = looking;
		return documents();
	end, function()
		if not progress.talking[officer] or progress.talked[officer] then
			npcSays(officer, "mission_hail");
		end
	end, 1, 30000);
end

local function step23()	-- Part 8: on to the Quartermaster
	tutorial:enableHudElement("all");
	if stage() < 12 then
		-- "Move to the next room to deliver your release documents to the Quartermaster."
		progress:say("tut_56", 1731, 3000);			-- sound/tut_56_quartermaster.snd
		progress:waitFor(function() return stage() >= 12 end,
			function() progress:say("tut_56", 1731, 3000) end, 30000, 30000);
	end
end

local function step25()	-- Part 9: the Quartermaster (room 11)
	tutorial:enableHudElement("all");
	tutorial:enableNpcConversationEvent(npcQuartermasterId);
	local function delivered() return tutorial:getSubState() >= 26 end
	-- "PART NINE OF NINE (bye!)"
	progress:say("part_9", nil, 1000);
	if not (progress.talking[npcQuartermasterId] or delivered()) then
		npcSays(npcQuartermasterId, "quarter_greeting");
		-- "Converse with the Quartermaster."
		progress:say("tut_57", 1893, 3000);			-- sound/tut_57_quartermaster.snd
	end
	progress:waitFor(delivered, function() progress:say("tut_57", 1893, 3000) end, 30000, 30000);
end

local function step26()	-- Part 9: the oath, before the travel terminal works
	tutorial:enableHudElement("all");
	local function sworn() return progress:hasSaid("i promise") end
	-- "You are required to promise to obey the code of conduct..."
	progress:say("oath_explain", 1047, 6000);			-- sound/tut_00_code_of_conduct.snd
	-- "I promise to treat my fellow players and citizens with respect..."
	progress:say("the_oath", 1424, 6000);			-- sound/tut_58_pledge.snd
	-- "To swear your oath, please type the words I PROMISE into your chat window..."
	progress:say("make_promise", 2730, 5000);			-- sound/tut_00_swear.snd
	-- Only what is typed from here counts.
	progress.said = {};
	progress:waitFor(sworn,
		function() progress:say("swear_reminder", 3175, 6000) end, 30000, 30000);	-- sound/tut_00_swear_reminder.snd
	-- "Thank you.  You are now a citizen of Star Wars Galaxies."
	progress:say("tut_60", 686, 4000);				-- sound/tut_60_thankyou.snd
	tutorial:setSubState(27);
end

local function step27()	-- Done: the travel terminal shows the destinations.
	tutorial:enableHudElement("all");
end

local function partsThreeToNine()
	if todo(10) and step10() == false then
		return false;
	end

	if todo(11) and step11() == false then
		return false;
	end

	if todo(12) and step12() == false then
		return false;
	end

	if todo(13) and step13() == false then
		return false;
	end

	if todo(14) and step14() == false then
		return false;
	end

	if todo(15) and step15() == false then
		return false;
	end

	if todo(16) and step16() == false then
		return false;
	end

	if todo(17) and step17() == false then
		return false;
	end

	if todo(18) and step18() == false then
		return false;
	end

	if todo(19) and step19() == false then
		return false;
	end

	if todo(20) and step20() == false then
		return false;
	end

	todo(21);
	if todo(22) and step22() == false then
		return false;
	end

	if todo(23) and step23() == false then
		return false;
	end

	todo(24);
	if todo(25) and step25() == false then
		return false;
	end

	if todo(26) and step26() == false then
		return false;
	end

	if todo(27) and step27() == false then
		return false;
	end
	progress:quiet();
	return true;
end
local delayLeft = 0;
local count;

while state == 1 do
	local subState = tutorial:getSubState();
		
	if subState <= 9 then
		partsOneAndTwo();
		delayLeft = 0;

	else
		partsThreeToNine();
		state = 2;
	end
end

if state == 3 then

	-- No funny business
	tutorial:enableHudElement("all");		-- Enable full UI

	-- "Welcome to Star Wars Galaxies."
	tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:welcome");
	tutorial:scriptPlayMusic(1267);		-- sound/tut_01_welcome.snd
	
	LuaScriptEngine.WaitMSec(5000);
	
	-- Stat Migration Sililoquy.
	tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:stat_migration");
	tutorial:scriptPlayMusic(1838);

end

progress:stop();
