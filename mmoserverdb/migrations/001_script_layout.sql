-- Script files moved into data/script/{world,tutorial,commands,dev}
-- (docs/SCRIPTING.md, "Layout"). Only paths change; the scripts are the same.

UPDATE swganh.config_zone_scripts SET file = 'script/world/common/weather.lua'
 WHERE file = 'script/WeatherController.lua';
UPDATE swganh.config_zone_scripts SET file = 'script/world/common/login_reporter.lua'
 WHERE file = 'script/PlayerLoginReporter.lua';

UPDATE swganh.config_zone_scripts SET file = 'script/dev/tatooine_npc_test1.lua'
 WHERE file = 'script/TatooineNpcTest.lua';
UPDATE swganh.config_zone_scripts SET file = 'script/dev/tatooine_npc_test2.lua'
 WHERE file = 'script/TatooineNpcTest2.lua';
UPDATE swganh.config_zone_scripts SET file = 'script/dev/tatooine_npc_test3.lua'
 WHERE file = 'script/TatooineNpcTest3.lua';
UPDATE swganh.config_zone_scripts SET file = 'script/dev/tatooine_npc_test4.lua'
 WHERE file = 'script/TatooineNpcTest4.lua';
UPDATE swganh.config_zone_scripts SET file = 'script/dev/tatooine_npc_test5.lua'
 WHERE file = 'script/TatooineNpcTest5.lua';
UPDATE swganh.config_zone_scripts SET file = 'script/dev/tatooine_npc_test6.lua'
 WHERE file = 'script/TatooineNpcTest6.lua';
UPDATE swganh.config_zone_scripts SET file = 'script/dev/tatooine_npc_test7.lua'
 WHERE file = 'script/TatooineNpcTest7.lua';
UPDATE swganh.config_zone_scripts SET file = 'script/dev/corellia_instance_test.lua'
 WHERE file = 'script/CorelliaInstanceTest.lua';

UPDATE swganh.command_table SET scripthook = 'script/commands/burstrun.lua'
 WHERE scripthook = 'script/command/burstrun.lua';
