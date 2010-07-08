-- WeatherController
-- sets random weather every 10 min
print("WeatherController v.0.2");

local updateInterval = 600000;
local wm = WorldManager:getSingletonPtr();

while (1) do
  LuaScriptEngine.WaitMSec(updateInterval);

  cloudX = math.random();
  cloudY = math.random();
  cloudZ = math.random();
  
  tempVal = math.random(0, 5);
  
  if tempVal == 5 then
	type = math.random(1,5);
	updateInterval = 60000;
  else
	type = 0;
	updateInterval = 600000;
  end;
  

  wm:updateWeather(cloudX,cloudY,cloudZ,type);
end;




