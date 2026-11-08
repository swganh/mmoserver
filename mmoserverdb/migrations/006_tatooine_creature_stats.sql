-- Tatooine creatures: stats from the Prima Official Game Guide (2004, the
-- original; docs/SOURCES.md), creature tables, printed pages as cited. The
-- database had copied the womp rat's stats onto its smaller relatives, so
-- a lesser desert womp rat had 1000 health and hit for 90-110.
--
-- Prima gives one HAM range per creature: each pool (health, action, mind)
-- gets its middle, and the secondary stats follow their pool as before.
--
--   id  creature                level  HAM       damage  XP   page
--   101 womp rat                10     810-990   90-110  356  184
--   103 lesser desert womp rat  5      140-170   45-50   85   180
--   106 rill                    6      180-220   50-55   113  182
--
-- Not in the pages at hand, left as they are: greater desert womp rat (102),
-- variegated womp rat (104), womp rat hue (105).

DROP TEMPORARY TABLE IF EXISTS creature_stats_006;
CREATE TEMPORARY TABLE creature_stats_006 (
  npc_id BIGINT UNSIGNED PRIMARY KEY, lvl INT, ham INT, dmin INT, dmax INT, xp INT);
INSERT INTO creature_stats_006 VALUES
  (101, 10, 900, 90, 110, 356),
  (103,  5, 155, 45,  50,  85),
  (106,  6, 200, 50,  55, 113);

UPDATE swganh.non_persistent_npcs n
  JOIN creature_stats_006 s ON s.npc_id = n.id
  SET n.level = s.lvl;

UPDATE swganh.non_persistent_npc_attributes a
  JOIN swganh.attributes b ON b.id = a.attribute_id
  JOIN creature_stats_006 s ON s.npc_id = a.npc_id
  SET a.value = CASE b.name
      WHEN 'creature_damage_min' THEN s.dmin
      WHEN 'creature_damage_max' THEN s.dmax
      WHEN 'creature_xp' THEN s.xp
      ELSE s.ham END
  WHERE b.name IN ('creature_health', 'creature_strength', 'creature_constitution',
                   'creature_action', 'creature_quickness', 'creature_stamina',
                   'creature_mind', 'creature_focus', 'creature_willpower',
                   'creature_damage_min', 'creature_damage_max', 'creature_xp');

DROP TEMPORARY TABLE creature_stats_006;
