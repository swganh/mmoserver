-- TutorialPart_2_9

-- Get access to Tutorial via ScriptEngine
local script = LuaScriptEngine.getScriptObj();
local SE = ScriptEngine:Init();
local tutorial = SE:getTutorial(script);

-- Verify correct state
local state = tutorial:getState();

-- wait for client to become ready.
while tutorial:getReady() == false do
	LuaScriptEngine.WaitMSec(100)
end
-- tutorial:disableHudElement("all");	-- Disable all hud elements
-- tutorial:enableHudElement("all");	-- Disable all hud elements

while state == 2 do
	local subState = tutorial:getSubState();
		
	if subState == 1 then
		LuaScriptEngine.WaitMSec(3000);						
		-- "PART TWO OF NINE (conversing with npcs and inventory) "
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:part_2");

		LuaScriptEngine.WaitMSec(1000);

		-- "Move forward and click-and-hold your mouse on the Imperial Officer until the radial menu appears. "
		tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:tut_08");

		tutorial:scriptPlayMusic(1550);		-- sound/tut_08_imperialofficer.snd
	end	
	
	state = tutorial:getState();	
end
















