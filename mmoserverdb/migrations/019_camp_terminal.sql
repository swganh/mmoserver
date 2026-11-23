-- Camps had no terminal, so they could not be checked or disbanded
-- (docs/KNOWN_ISSUES.md #43). The code is there: ScoutManager::createCamp
-- spawns a CampTerminal (Status, Disband) from a camp's structure_item_template
-- row with tan_type 4 (TanGroup_Terminal); no camp had one.
--
-- The terminal is the client's camp control panel,
-- object/tangible/camp/shared_camp_control_panel.iff (Publish 14); it is
-- named after the camp when spawned. Camps are structure types 1-6
-- (structure_type_data: the scout_camp POIs).
--
-- Its place in the camp is reconstructed, not from the client or a guide:
-- 2 m right of and 3 m ahead of where the camp is placed, beside the fire
-- the basic camp puts 5.2 m ahead.
INSERT IGNORE INTO swganh.terminal_types (object_string, name, file)
  VALUES ('object/tangible/camp/shared_camp_control_panel.iff', 'unknown', 'terminal_detail');

INSERT INTO swganh.structure_item_template
  (structure_id, cell, item_type, relX, relY, relZ, dirX, dirY, dirZ, dirW, tan_type, rekW)
SELECT s.type, 0, tt.id, 2.0, 0, 3.0, 0, 0, 0, 1, 4, 0
  FROM swganh.structure_type_data s
  JOIN swganh.terminal_types tt ON tt.object_string = 'object/tangible/camp/shared_camp_control_panel.iff'
  WHERE s.type BETWEEN 1 AND 6
    AND NOT EXISTS (SELECT 1 FROM swganh.structure_item_template sit
                    WHERE sit.structure_id = s.type AND sit.tan_type = 4);
