-- Corellia Instance test
-- This script will not start until Zoneserver is ready.

local ss

-- Change HERE when you want to run as Instance or not.
-- local runAsInstance = true;
local runAsInstance = false;

-- Program start	

ss = ScriptSupport:Instance();

if runAsInstance == true then
	ss:enableInstance();
	print("Running Corellia as an instance.");
end;



