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



local delayLeft = 3000;
local count;

while state == 1 do
	local subState = tutorial:getSubState();
		
	if subState == 1 then
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
							
		-- "PART ONE OF NINE (movement and chat)"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_1");

		LuaScriptEngine.WaitMSec(1000);

		-- "Welcome to Star Wars Galaxies."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:welcome");

		tutorial:scriptPlayMusic(1267);		-- sound/tut_01_welcome.snd

		tutorial:setSubState(2);
		delayLeft = 5000;
		
	elseif subState == 2 then	-- Scroll / zoom to third person...	
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		-- "Use the arrow keys to move."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:movement_keyboard");		

		tutorial:scriptPlayMusic(3426);		-- sound/tut_02_movement.snd

		-- "Or hold down the right mouse-button to move forward."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:movement_mouse");
		
		-- "You can look around by moving your mouse."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:lookaround");
		
		LuaScriptEngine.WaitMSec(15000);
		
		-- "Using the mouse-wheel, scroll forward to zoom into your character. Notice that if you zoom in all the way, you'll be in first-person mode. Scroll backwards to zoom-out of your character, and into third-person mode. You can also zoom your camera in and out by pressing the keypad plus and minus keys. Try zooming between 1st and 3rd person."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:mousewheel");

		tutorial:scriptPlayMusic(1174);		-- sound/tut_00_camera.snd
		
		tutorial:updateTutorial("zoomCamera");		-- Wait for user to use the zoom
		local hasZoomed = false;		
		LuaScriptEngine.WaitMSec(25000);	-- wait for sound to finish

		hasZoomed = tutorial:isZoomCamera();

		-- wait max 30 seconds before we continue
		for count = 1,30 do 
			if hasZoomed == true then
				break;
			end; 
			LuaScriptEngine.WaitMSec(1000);
			hasZoomed = tutorial:isZoomCamera();
		end

		delayLeft = 0;
		while hasZoomed == false do
			-- "Give it a try.  Scroll out to 3rd-person and take a look at yourself."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:mousewheel_repeat");
			tutorial:scriptPlayMusic(141);		-- sound/tut_03_scroll_out.snd
			
			delayLeft = 5000;	
			
			for count = 1,30 do 
				hasZoomed = tutorial:isZoomCamera();
				if hasZoomed == true then
					break
				end 
				LuaScriptEngine.WaitMSec(1000);
				if delayLeft > 0 then
					delayLeft = delayLeft - 1000
				end
			end
		end
		
		tutorial:setSubState(3);

	elseif subState == 3 then	-- Use chat window
		tutorial:enableHudElement("chatbox");

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		tutorial:updateTutorial("chatActive");
		
		-- "This is your chat window. Type here to speak to your fellow players."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:chatwindow");
		tutorial:scriptPlayMusic(3379);		-- sound/tut_04_chat.snd
		
		-- "Go ahead and try it.  Just type what you want to say and then press ENTER."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:chatprompt");
		
		LuaScriptEngine.WaitMSec(10000);	-- wait for sound to finish	
		
		local hasChat = false;
		hasChat = tutorial:isChatBox();

		for count = 1,25 do 
			if hasChat == true then
				break
			end 
			LuaScriptEngine.WaitMSec(1000);
			hasChat = tutorial:isChatBox();
		end

		delayLeft = 0;
		while hasChat == false do
			-- "Don't be shy.  Type something in your chatwindow and press Enter"
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:repeatchatprompt");
			tutorial:scriptPlayMusic(1294);		-- sound/tut_05_remind_chat.snd
			
			delayLeft = 5000;	
			
			for count = 1,30 do 
				hasChat = tutorial:isChatBox();
				if hasChat == true then
					break
				end 
				LuaScriptEngine.WaitMSec(1000);
				if delayLeft > 0 then
					delayLeft = delayLeft - 1000
				end
			end
		end

		tutorial:setSubState(4);
		
	elseif subState == 4 then		-- Use Holocron
		tutorial:enableHudElement("chatbox");

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		tutorial:openHolocron();
		tutorial:updateTutorial("closeHolocron");
		
		-- "This is your Holocron. You can access it by pressing CTRL-H. Whenever you need information about something in Star Wars Galaxies, press CTRL-H to find it in your Holocron. To close this screen, press CTRL-H again."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:holocube");
		tutorial:scriptPlayMusic(1385);		-- sound/tut_00_holocron.snd

		-- wait at least until the previous sound/music has stopped.
		delayLeft = 17000;
		while tutorial:isCloseHolocron() == false do
			LuaScriptEngine.WaitMSec(1000);
			if delayLeft > 0 then
				delayLeft = delayLeft - 1000
			end
		end
	
		tutorial:setSubState(5);
		
	elseif subState == 5 then		-- Approach Imperial Officer.
		tutorial:enableHudElement("chatbox");
		
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		-- "Excellent! Come through the door and down the hallway there."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:move_to_item_room");
		tutorial:scriptPlayMusic(1691);		-- sound/tut_06_excellent.snd

		delayLeft = 5000;
		for count = 1,30 do 
			if tutorial:getRoom() == 2 then
				break
			end 
			LuaScriptEngine.WaitMSec(1000);
			if delayLeft > 0 then
				delayLeft = delayLeft - 1000
			end
		end
		
		while tutorial:getRoom() ~= 2 do
			-- "Come on.  Don't keep me waiting all day."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:repeat_item_room_prompt");
			tutorial:scriptPlayMusic(1363);		-- sound/tut_07_comeon.snd
			
			delayLeft = 5000;	
			
			for count = 1,30 do 
				if tutorial:getRoom() == 2 then
					break
				end 
				LuaScriptEngine.WaitMSec(1000);
				if delayLeft > 0 then
					delayLeft = delayLeft - 1000
				end
			end
		end

		-- wait until sound has stopped.
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		-- tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:greeter1_bark1");
		tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:greeter1_bark1");

		while tutorial:getPlayerPosToObject(itemRoomOfficerId) > 20.0 do
			LuaScriptEngine.WaitMSec(4000);
		end;

		-- tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:greeter1_bark2");
		tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:greeter1_bark2");

		while tutorial:getPlayerPosToObject(itemRoomOfficerId) > 16.0 do
			LuaScriptEngine.WaitMSec(4000);
		end;

		-- tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:greeter1_bark3");
		tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:greeter1_bark3");

		while tutorial:getPlayerPosToObject(itemRoomOfficerId) > 12.0 do
			LuaScriptEngine.WaitMSec(4000);
		end;
	
		tutorial:setSubState(6);
		delayLeft = 3000;
		
	elseif subState == 6 then		-- Imperial Officer Conversation
		tutorial:enableHudElement("chatbox");
		tutorial:updateTutorial("changeLookAtTarget");
		
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		-- "PART TWO OF NINE (conversing with npcs and inventory) "
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_2");
		LuaScriptEngine.WaitMSec(1000);

		-- "Move forward and click-and-hold your mouse on the Imperial Officer until the radial menu appears. "
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_08");
		tutorial:scriptPlayMusic(1550);		-- sound/tut_08_imperialofficer.snd
				
		tutorial:enableNpcConversationEvent(itemRoomOfficerId);
		delayLeft = 8000;
		
		if (tutorial:getPlayerPosToObject(itemRoomOfficerId) > 10.0) then
			-- tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:greeting");
			tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:greeting");
		end;

		
		-- wait at least until the previous sound/music has stopped.
		while (scriptSupport:getTarget(playerId) ~= itemRoomOfficerId) do
			-- tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:explain_lookat");
			tutorial:spatialChat(itemRoomOfficerId, "@newbie_tutorial/newbie_convo:explain_lookat");

			LuaScriptEngine.WaitMSec(500);
			if delayLeft > 0 then
				delayLeft = delayLeft - 500
			end

			delayLeft = 5000;	
			for count = 1,30 do 
				if (scriptSupport:getTarget(playerId) == itemRoomOfficerId) then
					break
				end 
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
			end
		end;
		
		-- wait until sound has stopped.
		LuaScriptEngine.WaitMSec(delayLeft);

		-- "He is now your look-at target.  His name appears over his head."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_09");
		tutorial:scriptPlayMusic(1330);		-- sound/tut_09_lookat.snd
		
		LuaScriptEngine.WaitMSec(5000);
			

		-- "Click and hold the mouse button to get  a radial menu on the Imperial Officer."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_10");
		tutorial:scriptPlayMusic(2726);		-- sound/tut_10_radialmenu.snd
		
		LuaScriptEngine.WaitMSec(5000);


		-- "Do you see the option to converse?  While holding your mouse button, move to the converse option and let go of the mouse button to select it."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_11");
		tutorial:scriptPlayMusic(3436);		-- sound/tut_11_converse.snd
		
		-- wait at least until the previous sound/music has stopped.
		delayLeft = 10000;
		while tutorial:isNpcConversationStarted(itemRoomOfficerId) == false do
			LuaScriptEngine.WaitMSec(1000);
			if delayLeft > 0 then
				delayLeft = delayLeft - 1000
			end
		end

		-- wait until sound has stopped.
		LuaScriptEngine.WaitMSec(delayLeft);

		-- "Now you're in a conversation.  You can choose one of the responses to the right with your mouse."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_12");
		tutorial:scriptPlayMusic(716);		-- sound/tut_12_conversation.snd

		LuaScriptEngine.WaitMSec(12000);
		
		-- "Remember, you don't speak to other players this way.  Just type to talk to other players.  When you are finished conversing with the Imperial Officer, select the STOP CONVERSING option."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_13");
		tutorial:scriptPlayMusic(3009);		-- sound/tut_13_justtype.snd


		-- wait at least until the previous sound/music has stopped.
		delayLeft = 14000;
		while tutorial:isNpcConversationEnded(itemRoomOfficerId) == false do
			LuaScriptEngine.WaitMSec(1000);
			if delayLeft > 0 then
				delayLeft = delayLeft - 1000
			end
		end
		tutorial:updateTutorial("changeLookAtTarget");

		tutorial:setSubState(7);
		
	elseif subState == 7 then		-- Loot the Drum.
		tutorial:updateTutorial("changeLookAtTarget");
		tutorial:enableItemContainerEvent(containerId);
		tutorial:enableHudElement("chatbox");

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		-- send flytext "Open Me!"
		scriptSupport:sendFlyText(containerId, playerId, "newbie_tutorial/system_messages", "open_me", 0, 255, 0, 0);
		
		-- "To open the box just click and hold until you see the radial menu appear."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:prompt_open_box");
		tutorial:scriptPlayMusic(2008);		-- sound/tut_14_openbox.snd

		-- wait at least until the previous sound/music has stopped.
		delayLeft = 5000;
		local targetSelected = false;
		
		-- wait max 30 seconds before we continue
		-- while (targetSelected == false) do
		for count = 1,60 do 
			if (targetSelected == true) then
				break;
			end;
		
			if (count % 10) == 0 then
				-- send flytext "Open Me!"
				scriptSupport:sendFlyText(containerId, playerId, "newbie_tutorial/system_messages", "open_me", 0, 255, 0, 1);	
			end;
			
			if (tutorial:isChangeLookAtTarget() == false) then
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
			else
				-- Is it the correct target?
				if (scriptSupport:getTarget(playerId) == containerId)  then
					targetSelected = true;
				else
					-- print("Wrong target selected");
					tutorial:updateTutorial("changeLookAtTarget");
				end
			end;
		end;
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		while (targetSelected == false) do
			-- send flytext "Open Me!"
			-- scriptSupport:sendFlyText(containerId, playerId, "newbie_tutorial/system_messages", "open_me", 0, 255, 0, 5);

			-- "You might find something in the box you need.  Click and hold your mouse-button down on the box until you see the radial menu."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:repeat_open_box");
			tutorial:scriptPlayMusic(1799);		-- sound/tut_16_intheboxyouneed.snd
			
			delayLeft = 10000;

			for count = 1,60 do 
				if (targetSelected == true) then
					break;
				end;
		
				if (count % 10) == 0 then
					-- send flytext "Open Me!"
					scriptSupport:sendFlyText(containerId, playerId, "newbie_tutorial/system_messages", "open_me", 0, 255, 0, 1);	
				end;

				if (tutorial:isChangeLookAtTarget() == false) then
					LuaScriptEngine.WaitMSec(500);
					if delayLeft > 0 then
						delayLeft = delayLeft - 500
					end
				else
					-- Is it the correct target?
					if (scriptSupport:getTarget(playerId) == containerId)  then
						targetSelected = true;
					else
						-- print("Wrong target selected");
						tutorial:updateTutorial("changeLookAtTarget");
					end
				end;
			end;
		end;
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		-- tutorial:enableItemContainerEvent(containerId);
		
		-- "Great!  Now select the Open option to access the contents of the container."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:prompt_choose_open");
		tutorial:scriptPlayMusic(1332);		-- sound/tut_15_opencontainer.snd

		delayLeft = 8000;

		
		targetOpened = false;
		while (targetOpened == false and not tutorial:isContainerEmpty(containerId)) do
			if (tutorial:isContainerOpen(containerId) == false) then
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					print("STUCK LINE 644.");
					delayLeft = delayLeft - 500
				end
			else
				targetOpened = true;
				break;
			end;
		end;
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

-- Player has openend the container.
		-- "You can take these items by clicking-and-holding until you get the radial menu option on each item.  Pick up all of the items, and then click the X in the upper-right-hand corner of the container to close the box."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:prompt_take_items");
		tutorial:scriptPlayMusic(778);		-- sound/tut_16_a_youcantake.snd
		LuaScriptEngine.WaitMSec(15000);
		
-- A possible fix... wait until the user actually selected an item before asking for the "chosoe Pick Up...".

		if (tutorial:isContainerClosed(containerId) == false) then
-- Pick up an item
			-- "Now choose the Pick Up option to move the item to your inventory."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:prompt_select_pickup");
			tutorial:scriptPlayMusic(3041);		-- sound/tut_17_pickup.snd
			delayLeft = 4000;
		end;

-- Wait for player to pick up the items and close the container.
		while (tutorial:isContainerClosed(containerId) == false) do
		
			print("Container is NOT closed.");
			
			if (tutorial:isContainerEmpty(containerId) == true) then
				print("Container is EMPTY.");
				break;
			end;
		
			LuaScriptEngine.WaitMSec(delayLeft);
			delayLeft = 0;
		
			while tutorial:isItemTransferedFromContainer(containerId) == false do
				LuaScriptEngine.WaitMSec(500);
				if (tutorial:isContainerClosed(containerId) == true) then
					break;
				end; 
				
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
			end;
			
			-- wait until sound has stopped.
			LuaScriptEngine.WaitMSec(delayLeft);

			if (tutorial:isContainerClosed(containerId) == true) then
				break;
			end; 

-- Picked up an item
			-- "You've put the item in your inventory."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:pickup_complete");
			tutorial:scriptPlayMusic(58);		-- sound/tut_18_inventory.snd
			
			delayLeft = 2000;
			
		end;
		
		-- wait until sound has stopped.
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

-- We HAVE to close the container
--The code to show a closed container was never being called. This means we need to comment this out for now for the sake of allowing the rest of the tutorial to work.
		--while (tutorial:isContainerClosed(containerId) == false) do
			--print("STUCK LINE 715.");
 			--LuaScriptEngine.WaitMSec(500);
 		--end;
 		
 		tutorial:updateTutorial("openInventory");
 		tutorial:setSubState(8);
		
	elseif subState == 8 then		-- The container is closed, consume the food from inventory.
		tutorial:updateTutorial("openInventory");
		tutorial:enableHudElement("chatbox");
		
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		-- "Notice that your mouse mode changed when you opened that container."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:explain_freemouse");
		tutorial:scriptPlayMusic(3416);		-- sound/tut_19_inventory.snd
		
		-- "You can toggle the mouse mode yourself with the ALT key."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:explain_freemouse_toggle");
		-- LuaScriptEngine.WaitMSec(1000);

		-- "You can examine your inventory by pressing CTRL-I."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:explain_inventory");
		-- LuaScriptEngine.WaitMSec(1000);
		
		-- tutorial:scriptPlayMusic(3416);		-- sound/tut_19_inventory.snd
		
		LuaScriptEngine.WaitMSec(15000);
		
		-- "Press CTRL-I to open your inventory, and select one of the food items you found in the crate."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:repeat_open_inventory");
		tutorial:scriptPlayMusic(1664);		-- sound/tut_25_openinventory.snd
		delayLeft = 8000;	
		
		-- LuaScriptEngine.WaitMSec(1000);
		
		while tutorial:isOpenInventory() == false do
			LuaScriptEngine.WaitMSec(1000);
			if delayLeft > 0 then
				delayLeft = delayLeft - 1000
			end
		end
		
		-- Wait for the close of the inventory or user consumed the food.
		-- This construction is NOT an error, since we use different mechanichs internally. (but it's dirty).
		tutorial:updateTutorial("foodSelected");
		tutorial:updateTutorial("foodUsed");
		tutorial:updateTutorial("closeInventory");

		-- wait until sound has stopped.
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		if (tutorial:isCloseInventory() == false) then
			-- "Scroll through your inventory until you locate one of the food items that you've just picked-up."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:prompt_find_food");
			tutorial:scriptPlayMusic(2910);		-- sound/tut_20_selectfooditem.snd
			delayLeft = 8000;	
		end;

		local stateOfFood = 1;
		-- local done = false;
		local done = tutorial:isCloseInventory();
		
		while (done == false) do
			if (tutorial:isCloseInventory() == true) then
				done = true;
			end;
			LuaScriptEngine.WaitMSec(500);

			if delayLeft > 0 then
				delayLeft = delayLeft - 500;
			elseif (stateOfFood == 1) then
				if (tutorial:isFoodSelected() == true) then 
					
					-- "Select the Use option to use this item."
					tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:prompt_use_item");
					tutorial:scriptPlayMusic(934);		-- sound/tut_21_usefood.snd
					delayLeft = 5000;	
					-- print("found the food!");
					stateOfFood = 2;
				end;
			elseif (stateOfFood == 2) then
				if (tutorial:isFoodUsed() == true) then 
					-- "You used the item on yourself.  Food will help increase your attributes.  Medicine will help you recover damage to your attributes."
					tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:explain_item_used");
					tutorial:scriptPlayMusic(2571);		-- sound/tut_22_attributes.snd
					delayLeft = 10000;	
					
					-- Dev debug message
					-- tutorial:scriptSystemMessage("Dev message: 'Use' with food items are not implemented yet, no need to report it as a bug.");
					
					-- print("Eat the food!");
					done = true;
				end;
			end;
		end
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		tutorial:enableHudElement("toolbar");	-- Enable toolbar
		
		-- "This is your toolbar.  You can put things on it for instant access."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:show_toolbar");
		tutorial:scriptPlayMusic(2439);		-- sound/tut_23_toolbar.snd
		LuaScriptEngine.WaitMSec(8000);
		
		-- "You can drag items into your toolbar for easy access.  Simply click on the item and then drag it to an open slot on your toolbar.  Press the corresponding function key to use the item."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_00_toolbardrag");
		tutorial:scriptPlayMusic(1092);		-- sound/tut_00_toolbardrag.snd
		LuaScriptEngine.WaitMSec(16000);
		
		-- delayLeft = 0;	
		if (tutorial:isCloseInventory() == false) then
			-- "Press CTRL-I again to close your inventory window."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:close_inventory");
			tutorial:scriptPlayMusic(1603);		-- sound/tut_26_closeinventory.snd
			delayLeft = 2000;	
		end;

		while (tutorial:isCloseInventory() == false) do
			LuaScriptEngine.WaitMSec(500);
			if delayLeft > 0 then
				delayLeft = delayLeft - 500;
			end;
		end;
	
 		tutorial:setSubState(9);
		
	elseif subState == 9 then		-- The inventory is closed, proceed to next room.
		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");	-- Enable toolbar

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		-- "Proceed to the next room."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:visit_commerce_room");
		tutorial:scriptPlayMusic(2202);		-- sound/tut_27_proceed.snd
		
		delayLeft = 8000;			
		
		for count = 1,30 do 
			if tutorial:getRoom() == 3 then
				break
			end 
			LuaScriptEngine.WaitMSec(1000);
			if delayLeft > 0 then
				delayLeft = delayLeft - 1000
			end
		end
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		while tutorial:getRoom() ~= 3 do
			-- "No need to stick around here all day.  Walk down the corridor to the next area."
			-- tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:repeat_visit_commerce");
			-- MISSING THE CORRECT SOUND HERE
			
			-- "Proceed to the next room."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:visit_commerce_room");
			tutorial:scriptPlayMusic(2202);		-- sound/tut_27_proceed.snd

			delayLeft = 3000;	
			
			for count = 1,30 do 
				if tutorial:getRoom() == 3 then
					break
				end 
				LuaScriptEngine.WaitMSec(1000);
				if delayLeft > 0 then
					delayLeft = delayLeft - 1000
				end
			end
		end
 		tutorial:setSubState(10);
		
	elseif subState == 10 then		-- Part 3
		tutorial:updateTutorial("changeLookAtTarget");
		tutorial:enableNpcConversationEvent(bankAndBazzarRoomOfficerId);
		
		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");	-- Enable toolbar

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		-- "PART THREE OF NINE (banking and shopping)"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_3");

		LuaScriptEngine.WaitMSec(1000);

		-- "Converse with the Imperial Officer"
		-- tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_28");
		-- tutorial:scriptPlayMusic(3149);		-- sound/tut_28_converse.snd

		-- tutorial:spatialChat(bankAndBazzarRoomOfficerId, "Well hello!  Come talk to me if you need assistance with this terminal");
		-- delayLeft = 5000;
		
		local targetSelected = false;
		while (targetSelected == false) do
			-- "Converse with the Imperial Officer"
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_28");
			tutorial:scriptPlayMusic(3149);		-- sound/tut_28_converse.snd
			
			tutorial:spatialChat(bankAndBazzarRoomOfficerId, "@newbie_tutorial/newbie_convo:clone_greeting");
			delayLeft = 5000;
			
			for count = 1,60 do 
				if (tutorial:isChangeLookAtTarget() == false) then
					LuaScriptEngine.WaitMSec(500);
					if delayLeft > 0 then
						delayLeft = delayLeft - 500
					end
				end
				-- Is it the correct target?g9
				if (scriptSupport:getTarget(playerId) == bankAndBazzarRoomOfficerId)  then
					targetSelected = true;
					break;
				else
					-- print("Wrong target selected");
					tutorial:updateTutorial("changeLookAtTarget");
				end
			end;
		-- end;
		
			if (targetSelected == true) then
				targetSelected = false;
				
				-- Activate again, we need to check bank and bazzar.
				tutorial:updateTutorial("changeLookAtTarget");
	
				-- wait until sound has stopped.
				LuaScriptEngine.WaitMSec(delayLeft);
				delayLeft = 0;

				-- "Do you see the option to converse?  While holding your mouse button, move to the converse option and let go of the mouse button to select it."
				tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_11");
				tutorial:scriptPlayMusic(3436);		-- sound/tut_11_converse.snd
						
				for count = 1,60 do 
					if (tutorial:isNpcConversationStarted(bankAndBazzarRoomOfficerId) == false) then
						LuaScriptEngine.WaitMSec(500);
						if delayLeft > 0 then
							delayLeft = delayLeft - 500
						end;
					else
						targetSelected = true;
						break;
					end;
				end;
			end
	
		end;
	
		-- wait until sound has stopped.
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		local allTargetsActivated = false;
		local bankSelected = false;
		local bazzarSelected = false;
		
		while (allTargetsActivated == false) do
			for count = 1,60 do 
				if (allTargetsActivated == true) then
					break;
				end;
		
				if (count % 10) == 0 then
					if (bankSelected == false) then
						-- send flytext "*Access the bank here!*"
						scriptSupport:sendFlyText(bankTerminalId, playerId, "newbie_tutorial/system_messages", "bank_flytext", 0, 255, 0, 0);
					end;

					if (bazzarSelected == false) then
						-- send flytext "*Shop!*"
						scriptSupport:sendFlyText(bazzarTerminalId, playerId, "newbie_tutorial/system_messages", "bazaar_flytext", 0, 255, 0, 0);
					end;
				end;
				
				if (tutorial:isChangeLookAtTarget() == false) then
					LuaScriptEngine.WaitMSec(500);
					if delayLeft > 0 then
						delayLeft = delayLeft - 500
					end
				end;

				-- Is it the correct target?
				if (scriptSupport:getTarget(playerId) == bankTerminalId) and (delayLeft == 0) then
					if (bankSelected == false) then
						tutorial:updateTutorial("changeLookAtTarget");
						
						-- "Banks let you deposit and withdraw cash on hand.  You can also use the bank to store items."
						tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:bank_info_1");

						tutorial:scriptPlayMusic(421);		-- sound/tut_32_bank.snd							
						LuaScriptEngine.WaitMSec(6000);

						-- "You can only retrieve your items from the bank in which you stored the item.  You can only store items in the bank you've joined."
						tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:bank_info_2");
						LuaScriptEngine.WaitMSec(7000);
						
						-- "Although you can change banks at any time, you can only be a member of one bank at a time."
						tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:bank_info_3");
						LuaScriptEngine.WaitMSec(7000);
						
						-- "You can spend credits from your bank account at most terminals."
						tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:bank_info_4");
						LuaScriptEngine.WaitMSec(5000);
						
						-- "Banks let you deposit and withdraw cash on hand.  You can also use the bank to store items.  You can only retrieve your items from the bank in which you stored the item.  You can only store items in the bank you've joined.  Although you can change banks at any time, you can only be a member of one bank at a time.  You can spend credits from your bank account at most terminals."
						-- tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_32");

						-- tutorial:scriptPlayMusic(421);		-- sound/tut_32_bank.snd							
						-- LuaScriptEngine.WaitMSec(25000);

						-- "When you find hard credits (cash on hand), be sure to deposit them in your bank as soon as possible to avoid losing them."
						-- tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:bank_info_5");

						-- "When you find hard credits (cash on hand) be sure to deposit them in your bank as soon as possible to avoid losing them."
						-- tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_33");
						
						
						-- tutorial:scriptPlayMusic(46);		-- sound/tut_33_cash.snd							
						-- delayLeft = 12000;
						bankSelected = true;
					end;
					
				elseif (scriptSupport:getTarget(playerId) == bazzarTerminalId) and (delayLeft == 0) then
					if (bazzarSelected == false) then
						tutorial:updateTutorial("changeLookAtTarget");
						
						-- "This is an item dispenser.  You can use it to buy basic items in the game."
						tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:bazaar_info_1");

						tutorial:scriptPlayMusic(936);		-- sound/tut_29_itemdispenser.snd	-- This is an item Dispenser"
						LuaScriptEngine.WaitMSec(5500);
						
						tutorial:scriptPlayMusic(98);		-- sound/tut_00_bazaar_tease.snd
						-- "In addition to the basic item dispensers, you'll find a number of terminals that will connect you to the galactic bazaar."
						tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:bazaar_info_2");
						LuaScriptEngine.WaitMSec(7000);
						
						-- "Everything from large quantities of resources, to specialized items can be bought and sold at the bazaar terminal."
						tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:bazaar_info_3");
					
						-- tutorial:scriptPlayMusic(98);		-- sound/tut_00_bazaar_tease.snd
						delayLeft = 8500;
						bazzarSelected = true;						
					end;
				end;
				
				if (bankSelected == false) or (bazzarSelected == false) then
					tutorial:updateTutorial("changeLookAtTarget");
				else
					allTargetsActivated = true;
				end
				
				-- print("Room = " .. tutorial:getRoom() .. " and Z-position = " .. tutorial:getPlayerPosZ());
				if (tutorial:getRoom() == 3) and (tutorial:getPlayerPosZ() < -19) then
					-- print("Breaking from loop");
					allTargetsActivated = true;
				end;	

			end;
			
		end;
 		tutorial:setSubState(11);
		
	elseif subState == 11 then		-- Part 4 (first part)
		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");	-- Enable toolbar
		-- tutorial:enableNpcConversationEvent(cloningAndInsuranceDroidId);

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 3000;

		-- "PART FOUR OF NINE (cloning and insurance)"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_4");

		-- if (tutorial:getRoom() ~= 5) then
		if (tutorial:getRoom() < 5) then	
			-- "Move down the ramp toward the cloning and insurance terminals."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_36");
			tutorial:scriptPlayMusic(2297);	-- sound/tut_36_movedownhall.snd	
			
			-- We are going to pass some npc's that bark at us... and they all are in room 4.
			while tutorial:getRoom() ~= 4 do			
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
			end
			
			-- Now we are in the room with barkers...
			-- Are we down the ramp yet?
			local playerPosHeight = tutorial:getPlayerPosY();
			
			while (playerPosHeight > -6.9) do
				-- print("Height = " .. playerPosHeight);	
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
				playerPosHeight = tutorial:getPlayerPosY();
			end;

			-- Yee, we are down the ramp now.
			-- Let the dog start barking...
			tutorial:spatialChat(npcUserBark1Id, "@newbie_tutorial/newbie_convo:ins_user_bark_1");
			
			local playerPosZ = tutorial:getPlayerPosZ();			
					
			while (playerPosZ > -47) do
				-- print("playerPosZ = " .. playerPosZ);	
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
				playerPosZ = tutorial:getPlayerPosZ();					
			end;
			tutorial:spatialChat(npcUserBark2Id, "@newbie_tutorial/newbie_convo:ins_user_bark_2");					

			local playerPosX = tutorial:getPlayerPosX();
			playerPosZ = tutorial:getPlayerPosZ();			
					
			while (playerPosZ > -54) and (playerPosX > 45) do
				-- print("playerPosX = " .. playerPosX);
				-- print("playerPosZ = " .. playerPosZ);				
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
				playerPosX = tutorial:getPlayerPosX();
				playerPosZ = tutorial:getPlayerPosZ();			
			end;
			tutorial:spatialChat(npcUserBark3Id, "@newbie_tutorial/newbie_convo:ins_user_bark_3");								
		end;

		-- wait until we are in the correct room.
		while tutorial:getRoom() ~= 5 do			
			LuaScriptEngine.WaitMSec(1000);
			if delayLeft > 0 then
				delayLeft = delayLeft - 1000
			end
		end

 		tutorial:setSubState(12);
		
	elseif subState == 12 then		-- Continue Part 4
		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");	-- Enable toolbar
		tutorial:enableNpcConversationEvent(cloningAndInsuranceDroidId);

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		-- tutorial:spatialChat(cloningAndInsuranceDroidId, "@newbie_tutorial/newbie_convo:clone_greeting");
		tutorial:spatialChat(cloningAndInsuranceDroidId, "@newbie_tutorial/newbie_convo:clone_greeting");

		LuaScriptEngine.WaitMSec(2000);
		
		-- tutorial:spatialChat(cloningAndInsuranceDroidId, "@newbie_tutorial/newbie_convo:clone_prompt_convo");
		-- tutorial:spatialChat(cloningAndInsuranceDroidId, "Remember, to speak to me just click-and-hold on me until you get the radial menu, then select the CONVERSE menu option.");
		-- wait at least until the previous sound/music has stopped.

		delayLeft = 10000;		-- Shorter delay for the first time.
		while tutorial:isNpcConversationStarted(cloningAndInsuranceDroidId) == false do
			LuaScriptEngine.WaitMSec(500);
			if delayLeft > 0 then
				delayLeft = delayLeft - 500
			else
				-- tutorial:spatialChat(cloningAndInsuranceDroidId, "@newbie_tutorial/newbie_convo:clone_prompt_convo");
				tutorial:spatialChat(cloningAndInsuranceDroidId, "@newbie_tutorial/newbie_convo:clone_prompt_convo");
				delayLeft = 30000;				
			end
		end
		delayLeft = 0;

		-- Set event for using the Cloning Terminal.
		tutorial:updateTutorial("cloneDataSaved");
		
		-- Let the user have the conversation with the droid.

		while tutorial:isNpcConversationEnded(cloningAndInsuranceDroidId) == false do
			LuaScriptEngine.WaitMSec(1000);
		end
		
		-- Conversation done, let's use the terminals. Cloning first.

		-- send flytext "*Clone!*"
		scriptSupport:sendFlyText(cloningTerminalId, playerId, "newbie_tutorial/system_messages", "clone_here", 0, 255, 0, 0);

		-- "Use the cloning terminal.  You can do this through the radial menu, or you can just double-click.  The cursor changes when over a usable item."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_37");
		tutorial:scriptPlayMusic(3245);		-- sound/tut_37_cloning.snd							

		LuaScriptEngine.WaitMSec(5000);

-- TODO: We SHALL wait for the cloning data to be storead AND full Insurance before we continue.

		-- wait for the cloning data to be storead
		count = 0;
		while (tutorial:isCloneDataSaved() == false) do
			LuaScriptEngine.WaitMSec(500);

			if (count % 10) == 0 then
				-- send flytext "*Clone!*"
				scriptSupport:sendFlyText(cloningTerminalId, playerId, "newbie_tutorial/system_messages", "clone_here", 0, 255, 0, 0);
				count = 1;
			else
				count = count + 1;
			end;
		end;
		
		tutorial:updateTutorial("insureItemsDone");

		-- "Your clone data has been saved!"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:declare_cloned");

		-- tutorial:spatialChat(cloningAndInsuranceDroidId, "@newbie_tutorial/newbie_convo:convo_3_explain_terminal_1");
		tutorial:spatialChat(cloningAndInsuranceDroidId, "@newbie_tutorial/newbie_convo:convo_3_explain_terminal_1");
		
		LuaScriptEngine.WaitMSec(5000);
		
		-- tutorial:spatialChat(cloningAndInsuranceDroidId, "@newbie_tutorial/newbie_convo:convo_3_explain_terminal_2");
		tutorial:spatialChat(cloningAndInsuranceDroidId, "@newbie_tutorial/newbie_convo:convo_3_explain_terminal_2");

		-- send flytext "*Insurance!*"
		scriptSupport:sendFlyText(insuranceTerminalId, playerId, "newbie_tutorial/system_messages", "insure_here", 0, 255, 0, 0);

		LuaScriptEngine.WaitMSec(2000);
		
		-- "Use the insurance terminal."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_38");
		tutorial:scriptPlayMusic(2678);		-- sound/tut_38_insurance.snd						

		LuaScriptEngine.WaitMSec(3000);
		
		-- Is it the correct target?
		count = 0;		
		while (tutorial:isItemsInsured() == false) do
			LuaScriptEngine.WaitMSec(500);
		
			if (count % 10) == 0 then
				-- send flytext "*Insurance!*"
				scriptSupport:sendFlyText(insuranceTerminalId, playerId, "newbie_tutorial/system_messages", "insure_here", 0, 255, 0, 0);
				count = 1;
			else
				count = count + 1;
			end;
		end;

		-- "All of your items have been insured!"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:declare_insured");
		LuaScriptEngine.WaitMSec(3000);

		-- Some extra hints have been requested here, so I add this one.
		-- "Proceed to the next room."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:visit_commerce_room");
		tutorial:scriptPlayMusic(2202);		-- sound/tut_27_proceed.snd

		--  Waiting for one of the commoners to get "panic".
		local playerPosZ = tutorial:getPlayerPosZ();			
				
		while (playerPosZ > -64) do
			LuaScriptEngine.WaitMSec(500);
			if delayLeft > 0 then
				delayLeft = delayLeft - 500
			end
			playerPosZ = tutorial:getPlayerPosZ();					
		end;
		tutorial:spatialChatShout(npcShoutPanic, "@newbie_tutorial/newbie_convo:shout_panic1");					
		
 		tutorial:setSubState(13);
		
	elseif subState == 13 then		-- Continue Part 4
		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");	-- Enable toolbar
		LuaScriptEngine.WaitMSec(3000);
	
		local playerPosZ = tutorial:getPlayerPosZ();			
		while (playerPosZ > -73) do
			LuaScriptEngine.WaitMSec(500);
			playerPosZ = tutorial:getPlayerPosZ();					
		end;

		-- "If you're going down that hallway, you might want advanced warning of what's down there.  This is your radar."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:radar");
		tutorial:scriptPlayMusic(2975);		-- sound/tut_41_advancewarning.snd	
		LuaScriptEngine.WaitMSec(2000);
		tutorial:enableHudElement("radar");	-- Enable radar	
		
 		tutorial:setSubState(14);
		
	elseif subState == 14 then		-- Continue Part 4
		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");	-- Enable toolbar
		tutorial:enableHudElement("radar");	-- Enable radar
		LuaScriptEngine.WaitMSec(3000);
		
		local playerPosX = tutorial:getPlayerPosZ();			
		local playerPosZ = tutorial:getPlayerPosZ();			
		while (playerPosZ > -86) and (playerPosX < 3) and (tutorial:getRoom() < 7) do
		 	LuaScriptEngine.WaitMSec(500);
		 	playerPosZ = tutorial:getPlayerPosZ();					
		end;
		
		tutorial:spatialChatShout(npcShoutPanic, "@newbie_tutorial/newbie_convo:shout_panic2");					
		
		-- Wait until we enter the next room.
		while (tutorial:getRoom() < 6) do
			LuaScriptEngine.WaitMSec(500);
		end

		local playerPosX = tutorial:getPlayerPosX();			
				
		while (playerPosX < 4)  and (tutorial:getRoom() < 7) do
			LuaScriptEngine.WaitMSec(500);
			playerPosX = tutorial:getPlayerPosX();					
		end;

 		tutorial:setSubState(15);
		
	elseif subState == 15 then		-- Part 5

		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");	-- Enable toolbar
		tutorial:enableHudElement("radar");	-- Enable radar

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		-- "PART FIVE OF NINE (combat moves and radar)"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_5");
		LuaScriptEngine.WaitMSec(1000);
		
		-- The red dots show potentially aggressive entities.  Yellow dots indicate peaceful life signatures.  You can also access an overlay map with CTRL-M.  Press CTRL-M again to turn it off.
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:radar_more");
		tutorial:scriptPlayMusic(1259);		-- sound/tut_42_map.snd	
		LuaScriptEngine.WaitMSec(18000);
						
		-- You can zoom the overlay map using the mousewheel, while holding down the CTRL key.
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_43");
		tutorial:scriptPlayMusic(2327);		-- sound/tut_43_zoommap.snd	
		delayLeft = 5000;
	
		local playerPosX = tutorial:getPlayerPosX();			
				
		while (playerPosX < 12)  and (tutorial:getRoom() < 7) do
			LuaScriptEngine.WaitMSec(500);
			if delayLeft > 0 then
				delayLeft = delayLeft - 500
			end
			playerPosX = tutorial:getPlayerPosX();					
		end;
		tutorial:spatialChat(npcNervousGuy, "@newbie_tutorial/newbie_convo:nervous_guy1");	-- nervous_guy1

		playerPosX = tutorial:getPlayerPosX();
		playerPosZ = tutorial:getPlayerPosZ();			
		while ((playerPosZ < -86) or (playerPosX < 12)) and (tutorial:getRoom() < 7)  do
			LuaScriptEngine.WaitMSec(500);
			if delayLeft > 0 then
				delayLeft = delayLeft - 500
			end
			playerPosX = tutorial:getPlayerPosX();		
			playerPosZ = tutorial:getPlayerPosZ();						
		end;
		tutorial:spatialChat(npcNervousGuy, "@newbie_tutorial/newbie_convo:nervous_guy2");	-- nervous_guy2

		playerPosX = tutorial:getPlayerPosX();
		playerPosZ = tutorial:getPlayerPosZ();			
		while ((playerPosZ < -82) or (playerPosX < 12)) and (tutorial:getRoom() < 7)  do
			LuaScriptEngine.WaitMSec(500);
			if delayLeft > 0 then
				delayLeft = delayLeft - 500
			end
			playerPosX = tutorial:getPlayerPosX();		
			playerPosZ = tutorial:getPlayerPosZ();						
		end;
		tutorial:spatialChat(npcNervousGuy, "@newbie_tutorial/newbie_convo:nervous_guy3");	-- nervous_guy3
 		tutorial:setSubState(16);
 		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 3000;
				
	elseif subState == 16 then		-- Continue part 5
		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");	-- Enable toolbar
		tutorial:enableHudElement("radar");	-- Enable radar
		tutorial:enableNpcConversationEvent(npcCovardOfficer);

		-- Wait until we enter the next room.
		while tutorial:getRoom() < 7 do
			LuaScriptEngine.WaitMSec(500);
			if delayLeft > 0 then
				delayLeft = delayLeft - 500
			end
		end
 		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		-- NOTE: The state and npc id used are to be retained in QuestGiver::handleConversationEvent() and QuestGiver::preProcessfilterConversation().
		
		-- "Converse with the soldier."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_40");
		tutorial:scriptPlayMusic(957);		-- sound/tut_40_converse.snd	-- Converse with the soldier.
		
		while tutorial:isNpcConversationStarted(npcCovardOfficer) == false do
			LuaScriptEngine.WaitMSec(500);
		end

		while tutorial:isNpcConversationEnded(npcCovardOfficer) == false do
			LuaScriptEngine.WaitMSec(500);

			-- The state is magically changed to 17 in QuestGiver::handleConversationEvent().
			if (tutorial:getSubState() == 17) then -- "Ho-ho, now I have a machine gun" (or maybe just a pistol)
				-- tutorial:addQuestWeapon(10,2755);
				-- "A weapon has been added to your inventory."
				-- tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:receive_weapon");
				break;
			end;
		end
		
	elseif subState == 17 then		-- Continue part 5
		if (npcDebrisId == nil) then
			-- Create the Debris
			print("Adding new debris");
			npcDebris, npcDebrisId = createAndSpawnNpc(npcDebrisTypeId, "", "", cellDebris, yDirDebris, wDirDebris, xPosDebris, yPosDebris, zPosDebris);
		end;

		-- enable Debris to be attacked.
		tutorial:makeCreatureAttackable(npcDebrisId);

		tutorial:enableHudElement("chatbox");
		tutorial:enableHudElement("toolbar");	-- Enable toolbar
		tutorial:enableHudElement("radar");	-- Enable radar
		tutorial:enableHudElement("hambar");	-- Enable hambar

 		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		local playerPosX = tutorial:getPlayerPosX();			
		while (playerPosX < 45) do
			LuaScriptEngine.WaitMSec(500);
			playerPosX = tutorial:getPlayerPosX();					
		end;
		tutorial:spatialChatShout(npcNervousGuy, "@newbie_tutorial/newbie_convo:nervous_guy4");	-- nervous_guy4
		
		playerPosX = tutorial:getPlayerPosX();
		local playerPosZ = tutorial:getPlayerPosZ();			
		while (playerPosZ > -80) or (playerPosX < 72) do
			LuaScriptEngine.WaitMSec(500);
			playerPosX = tutorial:getPlayerPosX();		
			playerPosZ = tutorial:getPlayerPosZ();						
		end;

		-- enable Debris to be attacked.
		-- tutorial:makeCreatureAttackable(npcDebrisId);

		-- "The hallway is blocked by debris.  Attack it to destroy it.  Then proceed to the next room to face your enemy."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_45");
		tutorial:scriptPlayMusic(3183);		-- sound/tut_45_proceed.snd
		delayLeft = 8000;
		-- LuaScriptEngine.WaitMSec(3000);

		-- LuaScriptEngine.WaitMSec(6000);

		while (scriptSupport:npcIsDead(npcDebrisId) == false) do
			LuaScriptEngine.WaitMSec(500);
			if (delayLeft > 0) then
				delayLeft = delayLeft - 500
			end
		end;
		-- This will make the Debris re-spawn when we test and goes back to previous "state".
		npcDebrisId = nil;
 		tutorial:setSubState(18);

 		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

	elseif subState == 18 then		-- Part 6
		-- Spawn the bandit
		tutorial:enableHudElement("all");		-- Enable full UI
		
		local npcBandit;
		local npcBanditId;
		npcBandit, npcBanditId = createAndSpawnNpc(npcBanditTypeId, "", "", cellBandit, yDirBandit, wDirBandit, xPosBandit, yPosBandit, zPosBandit);

		-- 89 equals a melon :)
		createAndPopulateLoot(89, npcBanditId, playerId);
		
		-- 35 equals blue milk :)
		createAndPopulateLoot(35, npcBanditId, playerId);
		
		LuaScriptEngine.WaitMSec(2000);	

		-- "In combat, you can spend your HEALTH, MIND or ACTION pools to invoke special attacks."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:explain_combat_1");
		tutorial:scriptPlayMusic(179);		-- sound/tut_47_defaultattack.snd
		LuaScriptEngine.WaitMSec(5000);

		-- "The ATTACK option on the radial menu will be the default attack for hostile creatures."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:explain_combat_2");
		LuaScriptEngine.WaitMSec(10000);
		-- delayLeft = 8000;

		-- "PART SIX OF NINE (combat)"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_6");
		LuaScriptEngine.WaitMSec(3000);
		
		-- "When you hover your cursor over someone, your cursor changes to let you know that you can attack them.  Double-clicking them will attack them.  You can also cycle through targets with the tab key."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_44_attacking");
		tutorial:scriptPlayMusic(1553);		-- sound/tut_44_attacking.snd
		-- LuaScriptEngine.WaitMSec(5000);
		delayLeft = 5000;

		-- to be used when combat starts.
		while (scriptSupport:npcInCombat(npcBanditId) == false) do
			LuaScriptEngine.WaitMSec(500);
			if delayLeft > 0 then
				delayLeft = delayLeft - 500
			end
		end;
		
		-- fight for about 3 min, don't let the player over the bridge.
		-- if player HAM goes below 50%, let the Bandit use his grenade.
		-- if player pulls out of the fight, restart the time to completion.

		local combatMaxDuration = 300000;	-- 5 minutes max to fight uninterrupted and have fun!
		-- local combatMaxDuration = 15000;	-- 1 minute to fight and have fun!

		local combatTimer = combatMaxDuration;
		
		-- Main combat loop, continue here until npc is dead.
		-- We are in combat. Check player and target HAM.
		while (scriptSupport:npcIsDead(npcBanditId) == false) do
			if (tutorial:isLowHam(npcBanditId, 200) == true) then
				combatTimer = 0;
			end;
			
			if ((scriptSupport:npcInCombat(npcBanditId) == true) and (scriptSupport:npcInCombat(playerId) == true)) then
				if (combatTimer > 0) then
					combatTimer = combatTimer - 500;
					LuaScriptEngine.WaitMSec(500);
				else
					-- Blow the grenade.
					tutorial:npcStopFight(npcBanditId);
				
					tutorial:spatialChat(npcBanditId, "@newbie_tutorial/newbie_convo:pirate_taunt3");
					LuaScriptEngine.WaitMSec(2000);
					tutorial:spatialChat(npcBanditId, "@newbie_tutorial/newbie_convo:pirate_taunt4");
					LuaScriptEngine.WaitMSec(250);
					scriptSupport:npcKill(npcBanditId);
					scriptSupport:scriptSystemMessage(playerId, npcBanditId, "clienteffect/combat_grenade_proton.cef");
					-- scriptSupport:npcKill(npcBanditId);
				end;
			else
				-- We pulled out of combat, to bad.
				-- print("Out of combat");			
				combatTimer = combatMaxDuration;
				LuaScriptEngine.WaitMSec(500);
			end;
			
			-- Don't let the player over the bridge.
			local playerPosX = tutorial:getPlayerPosX();
			local playerPosY = tutorial:getPlayerPosY();			
			local playerPosZ = tutorial:getPlayerPosZ();	
			
			if (playerPosX < 17) and (playerPosZ < -106)  then
				scriptSupport:setPlayerPosition(playerId, cellBandit, 19, playerPosY, -112);
			end;
		end;

		tutorial:setSubState(19);				
		
		-- wait until sound has stopped.
		LuaScriptEngine.WaitMSec(6000);
		
		-- "Congratulations!  The pirate has been destroyed.  Now you can loot his corpse by clicking and holding your mouse on him until you get the radial menu option to LOOT."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:loot_pirate");
		tutorial:scriptPlayMusic(3705);		-- sound/tut_00_congratulations.snd
		delayLeft = 2500;
		
	elseif subState == 19 then		-- Part 7
		tutorial:enableHudElement("all");		-- Enable full UI

		-- Have to check the existance of the trainer, since in test mode, we may enter here more than once.
		if (npcSkillTrainerId == nil) then
			-- spawn your dedicated skill trainer.
			npcSkillTrainerTypeId = tutorial:getSkillTrainerTypeId();

			if (npcSkillTrainerTypeId ~= 0) then
				npcSkillTrainer, npcSkillTrainerId = createAndSpawnNpc(npcSkillTrainerTypeId, "", "", cellSkillTrainer, yDirSkillTrainer, wDirSkillTrainer, xPosSkillTrainer, yPosSkillTrainer, zPosSkillTrainer);
				-- print("Spawned NPC");			
			else
				-- Invalid skill trainer type id, we have to abort the script.
				print("Failed spawning NPC");
				state = 2;
				break;
			end;
			
			tutorial:setTutorialCelebTaunts(celeb_guy1_Id);
			tutorial:setTutorialCelebTaunts(celeb_guy2_Id);
			
			tutorial:setTutorialRefugeeTaunts(refugee_guy1_id);
			tutorial:setTutorialRefugeeTaunts(refugee_guy2_id);
			tutorial:setTutorialRefugeeTaunts(refugee_guy3_id);
		end;
			
		-- special for testing
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		-- LuaScriptEngine.WaitMSec(7000);
		
		-- Enable when re-testing combat.
		-- tutorial:scriptSystemMessage("Developer message: If you want to help us test combat again, go back to the imperial officer in the previous room.");
		-- tutorial:scriptSystemMessage("When there, you are ready for a new fight. No logout is required.");
		
		while (tutorial:getRoom() ~= 9)  do
		
		-- Enable when re-testing combat.
			
			-- LuaScriptEngine.WaitMSec(500);
			-- if delayLeft > 0 then
			-- 	delayLeft = delayLeft - 500
			-- end
			
			-- special for testing.
			-- if tutorial:getRoom() == 7 then
			--	tutorial:setSubState(17);	
			--	tutorial:scriptSystemMessage("Developer message: You are now ready for a new fight. Go back and kill that Pirate.");
			--	tutorial:scriptSystemMessage("Thank you, all help with testing is appreciated!");
			--	break
			-- end 
			
			local count;
			for count = 1,60 do 
				if (tutorial:getRoom() == 9) then
					break
				end 
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
			end;
			
			LuaScriptEngine.WaitMSec(delayLeft);
			delayLeft = 0;		
			
			if (tutorial:getRoom() ~= 9) then
				-- "Proceed to the next room."
				tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:visit_commerce_room");
				tutorial:scriptPlayMusic(685);			-- sound/tut_52_walkdown.snd	
				delayLeft = 3000;		
			end;
		end;
		
		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		-- Enable when re-testing combat.
		-- if (tutorial:getSubState() == 19) then
		tutorial:enableNpcConversationEvent(npcOfficerWithTrainerId);

		-- "PART SEVEN OF NINE (skill training)"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_7");
		LuaScriptEngine.WaitMSec(2000);
		
		tutorial:spatialChat(npcOfficerWithTrainerId, "@newbie_tutorial/newbie_convo:off_1_greeting");	--	off_1_greeting
		LuaScriptEngine.WaitMSec(1000);
		
		while tutorial:isNpcConversationStarted(npcOfficerWithTrainerId) == false do
			-- "Converse with the soldier."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_40");
			tutorial:scriptPlayMusic(957);		-- sound/tut_40_converse.snd	-- Converse with the soldier.

			delayLeft = 3000;	
			
			local count;
			for count = 1,60 do 
				if tutorial:isNpcConversationStarted(npcOfficerWithTrainerId) == true then
					break
				end 
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
			end
		end

		while tutorial:isNpcConversationEnded(npcOfficerWithTrainerId) == false do
			LuaScriptEngine.WaitMSec(500);
		end;

		-- if the player already have trained, do not force him to converse with trainer.
		if tutorial:isPlayerTrained() == false then
			tutorial:enableNpcConversationEvent(npcSkillTrainerId);
		
			tutorial:spatialChat(npcSkillTrainerId, "@newbie_tutorial/newbie_convo:trainer_grunt");	--	trainer_grunt
			LuaScriptEngine.WaitMSec(delayLeft);
			delayLeft = 0;

			while tutorial:isPlayerTrained() == false do
				-- "The Imperial Officer has directed you to a Skill Trainer.  Converse with the Skill Trainer to learn your first skill."
				tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_49");
				tutorial:scriptPlayMusic(3821);		-- sound/tut_49_skilltrainer.snd

				delayLeft = 3000;	
				
				local count;
				for count = 1,120 do 
					if tutorial:isPlayerTrained() == true then
						break
					end 
					LuaScriptEngine.WaitMSec(500);
					if delayLeft > 0 then
						delayLeft = delayLeft - 500
					end
				end
			end
			
			-- Wait for end of conversation.
			while tutorial:isNpcConversationEnded(npcSkillTrainerId) == false do
				LuaScriptEngine.WaitMSec(500);
			end;
			LuaScriptEngine.WaitMSec(delayLeft);
			delayLeft = 0;
					
		end;
		LuaScriptEngine.WaitMSec(3000);
		
		-- "You've picked up your first skill.  There are many skills to master in your chosen profession and there are advanced professions that only open up once you have mastered many skills.  You can check your skills using the skills window.  You can reach this with the skills button on the button bar, or by pressing CTRL-S."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_50");
		tutorial:scriptPlayMusic(883);		-- sound/tut_50_skillbrowser.snd
		LuaScriptEngine.WaitMSec(25000);
		
		-- "You can also check your character status after your fight, by pressing the character-sheet button on the button bar, or CTRL-C."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_51");
		tutorial:scriptPlayMusic(3773);		-- sound/tut_51_charactersheet.snd
		-- LuaScriptEngine.WaitMSec(12000);
		delayLeft = 12000;
	
		-- "Proceed to the next room."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:visit_commerce_room");
		tutorial:scriptPlayMusic(685);			-- sound/tut_52_walkdown.snd	
		delayLeft = 3000;
		tutorial:setSubState(20);

		-- end;
		
	elseif subState == 20 then
		tutorial:enableHudElement("all");		-- Enable full UI

		-- Wait until we enter the mission terminal room.
		while tutorial:getRoom() ~= 10 do
			local count;
			for count = 1,60 do 
				if (tutorial:getRoom() == 10) then
					break
				end 
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
			end;
			
			LuaScriptEngine.WaitMSec(delayLeft);
			delayLeft = 0;		
			
			if (tutorial:getRoom() ~= 10) then
				-- "Proceed to the next room."
				tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:visit_commerce_room");
				tutorial:scriptPlayMusic(685);			-- sound/tut_52_walkdown.snd	
				delayLeft = 3000;
			end;
		end;
			
		tutorial:setSubState(21);

	elseif subState == 21 then					-- Part 8
		tutorial:enableNpcConversationEvent(npcOfficerInMissionRoomId);
		tutorial:enableHudElement("all");		-- Enable full UI

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		-- "PART EIGHT OF NINE (missions and waypoints)"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_8");
		LuaScriptEngine.WaitMSec(3000);

		-- "Mission terminals like those to your left are located throughout the world.  Whenever you want a quick adventure you can always take a job from a mission terminal."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_53");
		tutorial:scriptPlayMusic(3591);		-- sound/tut_53_missions.snd
		LuaScriptEngine.WaitMSec(10000);
		
		-- "Waypoints indicate destinations.  When you have a job or mission, you can follow the waypoint on your radar.  They appear light blue."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_55_waypoints");
		tutorial:scriptPlayMusic(2887);		-- sound/tut_55_waypoints.snd
		LuaScriptEngine.WaitMSec(10000);
		
		-- "You can also speak to npcs and do tasks for them.  There's one over there who has something for you to do."
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_54");
		tutorial:scriptPlayMusic(131);		-- sound/sound/tut_54_npcmission.snd.snd
		
		LuaScriptEngine.WaitMSec(2000);
		tutorial:npcSendAnimation(npcCommonerInMissionRoomId, 7, npcOfficerInMissionRoomId);
		delayLeft = 5000;
		
		tutorial:setSubState(22);	-- Going to get the release documents.

	elseif subState == 22 then	
		tutorial:enableNpcConversationEvent(npcOfficerInMissionRoomId);
		tutorial:enableHudElement("all");		-- Enable full UI
		local missionTerminalSelected = false;

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		if (scriptSupport:getTarget(playerId) == missionTerminalId) then
			missionTerminalSelected = true;

			-- "You cannot take a mission from this terminal.  To take a mission, you will need to find a mission terminal on a planet."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:mission_terminal");
			tutorial:scriptPlayMusic(3397);		-- sound/tut_00_mission_terminal.snd
			delayLeft = 5000;
			tutorial:updateTutorial("changeLookAtTarget");
		else
			tutorial:updateTutorial("changeLookAtTarget");
		end;
		
		while tutorial:isNpcConversationStarted(npcOfficerInMissionRoomId) == false do
			tutorial:spatialChat(npcOfficerInMissionRoomId, "@newbie_tutorial/newbie_convo:mission_hail");	--	mission_hail
			-- LuaScriptEngine.WaitMSec(3000);
			-- delayLeft = 3000;

			local count;
			for count = 1,60 do 
				if (tutorial:isNpcConversationStarted(npcOfficerInMissionRoomId) == true) then
					break
				end 
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
				
				if (tutorial:isChangeLookAtTarget() == true) then
					-- Is it the correct target?
					if (scriptSupport:getTarget(playerId) == missionTerminalId) then
						if (missionTerminalSelected == false) then
							LuaScriptEngine.WaitMSec(delayLeft);
							delayLeft = 0;
							missionTerminalSelected = true;
							-- "You cannot take a mission from this terminal.  To take a mission, you will need to find a mission terminal on a planet."
							tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:mission_terminal");
							tutorial:scriptPlayMusic(3397);		-- sound/tut_00_mission_terminal.snd
							delayLeft = 5000;
							tutorial:updateTutorial("changeLookAtTarget");
						end;							
					else
						tutorial:updateTutorial("changeLookAtTarget");
						missionTerminalSelected = false;
					end;
				end;					
			end;
			-- tutorial:spatialChat(npcOfficerInMissionRoomId, "You there.  No time for that now.  I've got something for you to do.");	--	mission_hail
		end

		while tutorial:isNpcConversationEnded(npcOfficerInMissionRoomId) == false do
			LuaScriptEngine.WaitMSec(500);
			if (tutorial:isChangeLookAtTarget() == true) then
				-- Is it the correct target?
				if (scriptSupport:getTarget(playerId) == missionTerminalId) then
					if (missionTerminalSelected == false) then
						LuaScriptEngine.WaitMSec(delayLeft);
						delayLeft = 0;
						missionTerminalSelected = true;
						-- "You cannot take a mission from this terminal.  To take a mission, you will need to find a mission terminal on a planet."
						tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:mission_terminal");
						tutorial:scriptPlayMusic(3397);		-- sound/tut_00_mission_terminal.snd
						delayLeft = 5000;
						tutorial:updateTutorial("changeLookAtTarget");
					end;							
				else
					tutorial:updateTutorial("changeLookAtTarget");
					missionTerminalSelected = false;
				end;
			end;
		end;
		delayLeft = 1000;

		-- Will end up in state 23 when finishing the conversation...	

	elseif subState == 23 then
		tutorial:enableHudElement("all");		-- Enable full UI

		-- Wait until we enter the mission terminal room.
		while tutorial:getRoom() ~= 11 do
			-- "Move to the next room to deliver your release documents to the Quartermaster."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_56");
			tutorial:scriptPlayMusic(1731);		-- sound/tut_56_quartermaster.snd
			delayLeft = 3000;		
		
			local count;
			for count = 1,60 do 
				if (tutorial:getRoom() == 11) then
					break
				end 
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
			end;		
		end;
			
		tutorial:setSubState(24);

	elseif subState == 24 then					-- Part 9
		tutorial:enableHudElement("all");		-- Enable full UI

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;

		-- "PART NINE OF NINE (bye!)"
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_9");
		LuaScriptEngine.WaitMSec(1000);

		tutorial:spatialChat(npcQuartermasterId, "@newbie_tutorial/newbie_convo:quarter_greeting");	--	quarter_greeting
		LuaScriptEngine.WaitMSec(1000);
		
		tutorial:setSubState(25);

	elseif subState == 25 then					-- Continue part 9
		tutorial:enableNpcConversationEvent(npcQuartermasterId);
		tutorial:enableHudElement("all");		-- Enable full UI

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;
		
		while tutorial:isNpcConversationStarted(npcQuartermasterId) == false do
			-- "Converse with the Quartermaster."
			tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_57");
			tutorial:scriptPlayMusic(1893);		-- sound/tut_57_quartermaster.snd
			LuaScriptEngine.WaitMSec(3000);
			delayLeft = 3000;	
			
			local count;
			for count = 1,60 do 
				if (tutorial:isNpcConversationStarted(npcQuartermasterId) == true) then
					break
				end 
				LuaScriptEngine.WaitMSec(500);
				if delayLeft > 0 then
					delayLeft = delayLeft - 500
				end
			end
		end

		while (tutorial:isNpcConversationEnded(npcQuartermasterId) == false) do
			LuaScriptEngine.WaitMSec(500);
		end;
		delayLeft = 1000;

	elseif subState == 26 then					-- Continue part 9
		tutorial:enableHudElement("all");		-- Enable full UI

		LuaScriptEngine.WaitMSec(delayLeft);
		delayLeft = 0;


		-- "Select your destination by clicking on one of the planets on the screen.  When you have selected the planet, select which city you wish to travel to by clicking on the picture to the right of the screen.  When you are ready to travel to the city, click on the arrow in the lower right-hand corner of the screen."
		-- tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:select_dest");

		state = 2;
		
		-- LuaScriptEngine.WaitMSec(3000);
		-- tutorial:scriptSystemMessage("This is the end, for now.");
		
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