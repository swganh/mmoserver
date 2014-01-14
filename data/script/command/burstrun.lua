-- burstrun
-- just an example

local wm 	 = WorldManager:getSingletonPtr();
local cmdMap = ObjectControllerCommandMap:getSingletonPtr();
local script = LuaScriptEngine.getScriptObj();

-- register us for callback
cmdMap:ScriptRegisterEvent(script,"burstrun");

-- parameters: none
function burstrun(params)
 --  wm:zoneSystemMessage("Run Forrest Run");
end;


