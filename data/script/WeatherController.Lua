-- WeatherController
-- sets random weather every 5 min
print("WeatherController v.0.1");

local updateInterval = 300000;
local wm = WorldManager:getSingletonPtr();

while (1) do
  LuaScriptEngine.WaitMSec(updateInterval);

  cloudX = math.random();
  cloudY = math.random();
  cloudZ = math.random();
  type = math.random(0, 5);

  wm:updateWeather(cloudX,cloudY,cloudZ,type);
end;




