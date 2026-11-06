-- Starts the event logger dev script in the tutorial (planet 41) as well,
-- to see what the client reports while the tutorial moves onto events.
-- It only prints to the zone's log.
INSERT INTO swganh.config_zone_scripts (planet_id, file, priority)
SELECT 41, 'script/dev/event_logger.lua', 0
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM swganh.config_zone_scripts
                  WHERE planet_id = 41 AND file = 'script/dev/event_logger.lua');
