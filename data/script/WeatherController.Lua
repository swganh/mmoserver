-- WeatherController
-- sets random weather every 5 min
print("WeatherController v.0.1");

local updateInterval = 100;
local wm = WorldManager:getSingletonPtr();

while (1) do
  LuaScriptEngine.WaitMSec(updateInterval);

  cloudX = 1000;--math.random();
  cloudY = 1000;--math.random();
  cloudZ = 1000;--math.random();
  type = 6;--math.random() * 3;

  wm:updateWeather(cloudX,cloudY,cloudZ,type);
end;




