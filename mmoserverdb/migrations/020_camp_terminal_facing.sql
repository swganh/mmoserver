-- Turns the camps' control panel (migration 019) 45 degrees, which looked
-- right in a test placement (Nov 2026, docs/KNOWN_ISSUES.md #43). The
-- rotation is about the vertical axis: dirY = sin(-45/2), dirW = cos(-45/2).
-- Reconstructed, as is its position.
UPDATE swganh.structure_item_template
  SET dirX = 0, dirY = -0.383, dirZ = 0, dirW = 0.924
  WHERE tan_type = 4 AND structure_id BETWEEN 1 AND 6
    AND item_type = (SELECT id FROM swganh.terminal_types
                     WHERE object_string = 'object/tangible/camp/shared_camp_control_panel.iff');
