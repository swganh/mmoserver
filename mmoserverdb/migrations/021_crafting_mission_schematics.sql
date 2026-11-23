-- Crafting missions (docs/KNOWN_ISSUES.md #28): the schematics, parts and
-- crafted items, so the mission can be done as its text describes: the
-- giver hands over a schematic, a supplier the parts, the player assembles
-- them and delivers the result.
--
-- From the Publish 14 client: the five schematics
-- object/draft_schematic/item/quest_item/shared_<part>.iff, each naming
-- three slots (craft_item_ingredients_n:<slot>) and the item it makes
-- (object/tangible/mission/quest_item/shared_<part>.iff); the slot parts'
-- templates, object/tangible/component/item/quest_item/shared_<slot>.iff
-- (already in object_models). The slot rows were already in draft_slots,
-- empty (ids below).
--
-- Reconstructed (the client does not say): each slot takes one of its part
-- (a similar-component slot, type 5); complexity 5 and the generic crafting
-- tool's category (subCategory 4096, as the artisan components); no
-- experimentation; no crafting XP. The schematics are in their own group,
-- 277, which no skill grants: the server grants them for a mission.
--
-- schem_crc.crc is the client's CRC of the schematic's shared_ template.

-- The slot parts (component family 13) and the crafted parts (mission family 23).

INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_particle_sensor.iff', 'particle_sensor', 'craft_item_ingredients_n', 'particle_sensor', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_particle_sensor.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_spin_alignment_inducer.iff', 'spin_alignment_inducer', 'craft_item_ingredients_n', 'spin_alignment_inducer', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_spin_alignment_inducer.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_momentum_compensator.iff', 'momentum_compensator', 'craft_item_ingredients_n', 'momentum_compensator', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_momentum_compensator.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_cycle_control.iff', 'cycle_control', 'craft_item_ingredients_n', 'cycle_control', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_cycle_control.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_transductive_shunt.iff', 'transductive_shunt', 'craft_item_ingredients_n', 'transductive_shunt', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_transductive_shunt.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_directional_sensor.iff', 'directional_sensor', 'craft_item_ingredients_n', 'directional_sensor', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_directional_sensor.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_loop_auditor.iff', 'loop_auditor', 'craft_item_ingredients_n', 'loop_auditor', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_loop_auditor.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_variance_throttle.iff', 'variance_throttle', 'craft_item_ingredients_n', 'variance_throttle', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_variance_throttle.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_corrective_inducer.iff', 'corrective_inducer', 'craft_item_ingredients_n', 'corrective_inducer', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_corrective_inducer.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_decoherence_sensor.iff', 'decoherence_sensor', 'craft_item_ingredients_n', 'decoherence_sensor', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_decoherence_sensor.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_signal_cleanup_unit.iff', 'signal_cleanup_unit', 'craft_item_ingredients_n', 'signal_cleanup_unit', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_signal_cleanup_unit.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_signal_rerouter.iff', 'signal_rerouter', 'craft_item_ingredients_n', 'signal_rerouter', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_signal_rerouter.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_voltage_inducer.iff', 'voltage_inducer', 'craft_item_ingredients_n', 'voltage_inducer', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_voltage_inducer.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_current_motivator.iff', 'current_motivator', 'craft_item_ingredients_n', 'current_motivator', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_current_motivator.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/component/item/quest_item/shared_tuning_cell.iff', 'tuning_cell', 'craft_item_ingredients_n', 'tuning_cell', 'craft_item_ingredients_d', 262149 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/component/item/quest_item/shared_tuning_cell.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/mission/quest_item/shared_attunement_grid.iff', 'attunement_grid', 'item_n', 'attunement_grid', 'item_d', 8211 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_attunement_grid.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/mission/quest_item/shared_current_alternator.iff', 'current_alternator', 'item_n', 'current_alternator', 'item_d', 8211 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_current_alternator.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/mission/quest_item/shared_feedback_controller.iff', 'feedback_controller', 'item_n', 'feedback_controller', 'item_d', 8211 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_feedback_controller.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/mission/quest_item/shared_output_governor.iff', 'output_governor', 'item_n', 'output_governor', 'item_d', 8211 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_output_governor.iff');
INSERT INTO swganh.item_types (object_string, stf_name, stf_file, stf_detail_name, stf_detail_file, bazaar_category)
  SELECT 'object/tangible/mission/quest_item/shared_power_regulator.iff', 'power_regulator', 'item_n', 'power_regulator', 'item_d', 8211 FROM DUAL
  WHERE NOT EXISTS (SELECT 1 FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_power_regulator.iff');

-- The group no skill grants.
INSERT IGNORE INTO swganh.schematic_groups (id, name) VALUES (277, 'craftMissionSchematics');

-- The slots: one part each.
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_particle_sensor.iff', amount = 1, optional = 0, type = 5
  WHERE id = 5988 AND component_name = 'particle_sensor';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_spin_alignment_inducer.iff', amount = 1, optional = 0, type = 5
  WHERE id = 5989 AND component_name = 'spin_alignment_inducer';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_momentum_compensator.iff', amount = 1, optional = 0, type = 5
  WHERE id = 5990 AND component_name = 'momentum_compensator';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_cycle_control.iff', amount = 1, optional = 0, type = 5
  WHERE id = 5991 AND component_name = 'cycle_control';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_transductive_shunt.iff', amount = 1, optional = 0, type = 5
  WHERE id = 5992 AND component_name = 'transductive_shunt';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_directional_sensor.iff', amount = 1, optional = 0, type = 5
  WHERE id = 5993 AND component_name = 'directional_sensor';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_loop_auditor.iff', amount = 1, optional = 0, type = 5
  WHERE id = 5994 AND component_name = 'loop_auditor';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_variance_throttle.iff', amount = 1, optional = 0, type = 5
  WHERE id = 5995 AND component_name = 'variance_throttle';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_corrective_inducer.iff', amount = 1, optional = 0, type = 5
  WHERE id = 5996 AND component_name = 'corrective_inducer';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_decoherence_sensor.iff', amount = 1, optional = 0, type = 5
  WHERE id = 6019 AND component_name = 'decoherence_sensor';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_signal_cleanup_unit.iff', amount = 1, optional = 0, type = 5
  WHERE id = 6020 AND component_name = 'signal_cleanup_unit';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_signal_rerouter.iff', amount = 1, optional = 0, type = 5
  WHERE id = 6021 AND component_name = 'signal_rerouter';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_voltage_inducer.iff', amount = 1, optional = 0, type = 5
  WHERE id = 6022 AND component_name = 'voltage_inducer';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_current_motivator.iff', amount = 1, optional = 0, type = 5
  WHERE id = 6023 AND component_name = 'current_motivator';
UPDATE swganh.draft_slots SET resource_name = 'object/tangible/component/item/quest_item/shared_tuning_cell.iff', amount = 1, optional = 0, type = 5
  WHERE id = 6024 AND component_name = 'tuning_cell';

-- The schematics.
INSERT IGNORE INTO swganh.draft_schematics (object_string, group_id, weightsbatch_id, complexity, datasize, done, experience, status, subCategory, craftEnabled)
  VALUES ('object/draft_schematic/item/quest_item/attunement_grid.iff', 277, 1673, 5, 1, 1, 0, 0, 4096, 1);
INSERT IGNORE INTO swganh.schem_crc (crc, object_string, name) VALUES (4038906353, 'object/draft_schematic/item/quest_item/attunement_grid.iff', 'attunement_grid');
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (4038906353, 5988);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (4038906353, 5989);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (4038906353, 5990);
INSERT IGNORE INTO swganh.draft_schematic_item_link (schematic_id, item_family, item_type)
  SELECT 4038906353, 23, id FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_attunement_grid.iff' LIMIT 1;
INSERT IGNORE INTO swganh.draft_schematics (object_string, group_id, weightsbatch_id, complexity, datasize, done, experience, status, subCategory, craftEnabled)
  VALUES ('object/draft_schematic/item/quest_item/current_alternator.iff', 277, 1674, 5, 1, 1, 0, 0, 4096, 1);
INSERT IGNORE INTO swganh.schem_crc (crc, object_string, name) VALUES (4250243198, 'object/draft_schematic/item/quest_item/current_alternator.iff', 'current_alternator');
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (4250243198, 5991);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (4250243198, 5992);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (4250243198, 5993);
INSERT IGNORE INTO swganh.draft_schematic_item_link (schematic_id, item_family, item_type)
  SELECT 4250243198, 23, id FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_current_alternator.iff' LIMIT 1;
INSERT IGNORE INTO swganh.draft_schematics (object_string, group_id, weightsbatch_id, complexity, datasize, done, experience, status, subCategory, craftEnabled)
  VALUES ('object/draft_schematic/item/quest_item/feedback_controller.iff', 277, 1675, 5, 1, 1, 0, 0, 4096, 1);
INSERT IGNORE INTO swganh.schem_crc (crc, object_string, name) VALUES (3237757933, 'object/draft_schematic/item/quest_item/feedback_controller.iff', 'feedback_controller');
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (3237757933, 5994);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (3237757933, 5995);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (3237757933, 5996);
INSERT IGNORE INTO swganh.draft_schematic_item_link (schematic_id, item_family, item_type)
  SELECT 3237757933, 23, id FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_feedback_controller.iff' LIMIT 1;
INSERT IGNORE INTO swganh.draft_schematics (object_string, group_id, weightsbatch_id, complexity, datasize, done, experience, status, subCategory, craftEnabled)
  VALUES ('object/draft_schematic/item/quest_item/output_governor.iff', 277, 1676, 5, 1, 1, 0, 0, 4096, 1);
INSERT IGNORE INTO swganh.schem_crc (crc, object_string, name) VALUES (863982881, 'object/draft_schematic/item/quest_item/output_governor.iff', 'output_governor');
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (863982881, 6019);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (863982881, 6020);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (863982881, 6021);
INSERT IGNORE INTO swganh.draft_schematic_item_link (schematic_id, item_family, item_type)
  SELECT 863982881, 23, id FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_output_governor.iff' LIMIT 1;
INSERT IGNORE INTO swganh.draft_schematics (object_string, group_id, weightsbatch_id, complexity, datasize, done, experience, status, subCategory, craftEnabled)
  VALUES ('object/draft_schematic/item/quest_item/power_regulator.iff', 277, 1677, 5, 1, 1, 0, 0, 4096, 1);
INSERT IGNORE INTO swganh.schem_crc (crc, object_string, name) VALUES (2816707854, 'object/draft_schematic/item/quest_item/power_regulator.iff', 'power_regulator');
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (2816707854, 6022);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (2816707854, 6023);
INSERT IGNORE INTO swganh.draft_schematics_slots (schematic_id, draft_slot_id) VALUES (2816707854, 6024);
INSERT IGNORE INTO swganh.draft_schematic_item_link (schematic_id, item_family, item_type)
  SELECT 2816707854, 23, id FROM swganh.item_types WHERE object_string = 'object/tangible/mission/quest_item/shared_power_regulator.iff' LIMIT 1;
