-- PlayerLoginReporter
-- sends a zone message when a player logs in
print("PlayerLoginReporter v.0.3");

local wm = WorldManager:getSingletonPtr();
local script = LuaScriptEngine.getScriptObj();

-- register us for callback
wm:ScriptRegisterEvent(script,"onPlayerEntered");
wm:ScriptRegisterEvent(script,"onPlayerLeft");

-- parameters are planet name, player name, total player count
function onPlayerEntered(params)
  splitParams = LuaScriptEngine.splitString(params," ");
  wm:zoneSystemMessage("Zone[" .. splitParams[1] .. "]: " .. splitParams[2] .. " has arrived (total players : " .. splitParams[3] .. ")");
end;

-- parameters are planet name, player name, total player count
function onPlayerLeft(params)
  splitParams = LuaScriptEngine.splitString(params," ");
  wm:zoneSystemMessage("Zone[" .. splitParams[1] .. "]: " .. splitParams[2] .. " has left (total players : " .. splitParams[3] .. ")");
end;

