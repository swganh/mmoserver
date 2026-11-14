-- WeatherController
-- sets random weather every 10 min
print("WeatherController v.0.2");

local updateInterval = 600000;
local wm = WorldManager:getSingletonPtr();

while (1) do
  LuaScriptEngine.WaitMSec(updateInterval);

  -- Locals: scripts share one Lua state, and a global named "type"
  -- replaced Lua's type() for every other script (the event logger broke
  -- ten minutes after start).
  local cloudX = math.random();
  local cloudY = math.random();
  local cloudZ = math.random();

  local weather;
  if math.random(0, 5) == 5 then
	weather = math.random(1,5);
	updateInterval = 60000;
  else
	weather = 0;
	updateInterval = 600000;
  end;

  wm:updateWeather(cloudX,cloudY,cloudZ,weather);
end;
