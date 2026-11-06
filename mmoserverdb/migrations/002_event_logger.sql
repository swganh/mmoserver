-- Starts the event logger dev script on Tatooine (planet 8), next to the
-- other dev scripts there. It only prints to the zone's log.
INSERT INTO swganh.config_zone_scripts (planet_id, file, priority)
SELECT 8, 'script/dev/event_logger.lua', 0
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM swganh.config_zone_scripts WHERE file = 'script/dev/event_logger.lua');
