-- Creature stats from the guides' creature tables (docs/SOURCES.md),
-- transcribed in mmoserverdb/reference/prima_2004/creatures.csv (the 2004
-- guide, levels 1-11) and mmoserverdb/reference/prima_ed/creatures.csv
-- (An Empire Divided, 2003, for creatures above level 11). Written by
-- tools/reference/creatures_sql.py; edit that, not this.
--
-- For each spawned creature: level; each pool the middle of the guide's
-- HAM range, and the range itself (creature_ham_min/max) for spawns to
-- roll in; to-hit (creature_tohit, %); damage; XP; the behaviour flags
-- (aggressive, killer, stalker, pack, herd; pack creatures assist their
-- social group); and, from the 2004 guide, meat, hide and bone with the
-- quantities a scout harvests, and milk.
--
-- Neither guide marks the womp rats or the rill aggressive: they were
-- aggressive here, and are not any more.
--
--   id   creature                 lvl HAM        to-hit damage  XP    source
--   101  womp rat                 10  810-990     28%  90-110   356   Prima 2004, pp. 184-185
--        Pack, Herd, Tamable, Intimidation attack
--   102  greater desert womprat   16  1200-2000   35%  85-90    1355  An Empire Divided, pp. 472-473
--        Stalker, Tamable, BlindAttack, StunAttack
--   103  lesser desert womp rat   5   140-170     25%  45-50    85    Prima 2004, pp. 180-181
--        Herd, Tamable
--   104  variegated womprat       14  1050-1750   30%  75-80    1085  An Empire Divided, pp. 478-479
--        Herd, Tamable, StunAttack
--   105  womp rat                 10  810-990     28%  90-110   356   Prima 2004, pp. 184-185
--        Pack, Herd, Tamable, Intimidation attack
--   106  rill                     6   180-220     25%  50-55    113   Prima 2004, pp. 182-183
--        Tamable
--
-- The greater desert and variegated womp rats keep their meat, hide and
-- bone: An Empire Divided's table has no resources.

INSERT IGNORE INTO swganh.attributes (name, internal, attribute_internal_description)
  VALUES ('creature_ham_min', 1, 'Lowest health, action and mind a spawn rolls (guide HAM range)'),
         ('creature_ham_max', 1, 'Highest health, action and mind a spawn rolls (guide HAM range)'),
         ('creature_social_group', 1, 'Social group: pack creatures assist their own'),
         ('creature_meat_qty', 1, 'Meat a scout harvests'),
         ('creature_hide_qty', 1, 'Hide a scout harvests'),
         ('creature_bone_qty', 1, 'Bone a scout harvests'),
         ('creature_milk', 1, 'Milk: milk, wild, or empty');

DROP TEMPORARY TABLE IF EXISTS creatures_011;
CREATE TEMPORARY TABLE creatures_011 (
  npc_id BIGINT UNSIGNED, attribute VARCHAR(64), value VARCHAR(64),
  PRIMARY KEY (npc_id, attribute));
INSERT INTO creatures_011 VALUES
  (101, 'creature_health', '900'),
  (101, 'creature_strength', '900'),
  (101, 'creature_constitution', '900'),
  (101, 'creature_action', '900'),
  (101, 'creature_quickness', '900'),
  (101, 'creature_stamina', '900'),
  (101, 'creature_mind', '900'),
  (101, 'creature_focus', '900'),
  (101, 'creature_willpower', '900'),
  (101, 'creature_ham_min', '810'),
  (101, 'creature_ham_max', '990'),
  (101, 'creature_tohit', '28'),
  (101, 'creature_damage_min', '90'),
  (101, 'creature_damage_max', '110'),
  (101, 'creature_xp', '356'),
  (101, 'creature_social_group', 'Womprat'),
  (101, 'killer', '0'),
  (101, 'Pack', '1'),
  (101, 'Herd', '1'),
  (101, 'stalking', '0'),
  (101, 'creature_is_aggressive', '0'),
  (101, 'creature_group_assist', '1'),
  (101, 'res_meat', 'Wild Meat'),
  (101, 'creature_meat_qty', '4'),
  (101, 'res_hide', 'Leathery Hide'),
  (101, 'creature_hide_qty', '3'),
  (101, 'res_bone', 'Mammal Bone'),
  (101, 'creature_bone_qty', '2'),
  (102, 'creature_health', '1600'),
  (102, 'creature_strength', '1600'),
  (102, 'creature_constitution', '1600'),
  (102, 'creature_action', '1600'),
  (102, 'creature_quickness', '1600'),
  (102, 'creature_stamina', '1600'),
  (102, 'creature_mind', '1600'),
  (102, 'creature_focus', '1600'),
  (102, 'creature_willpower', '1600'),
  (102, 'creature_ham_min', '1200'),
  (102, 'creature_ham_max', '2000'),
  (102, 'creature_tohit', '35'),
  (102, 'creature_damage_min', '85'),
  (102, 'creature_damage_max', '90'),
  (102, 'creature_xp', '1355'),
  (102, 'creature_social_group', 'greater desert womprat'),
  (102, 'killer', '0'),
  (102, 'Pack', '0'),
  (102, 'Herd', '0'),
  (102, 'stalking', '1'),
  (102, 'creature_is_aggressive', '0'),
  (102, 'creature_group_assist', '0'),
  (103, 'creature_health', '155'),
  (103, 'creature_strength', '155'),
  (103, 'creature_constitution', '155'),
  (103, 'creature_action', '155'),
  (103, 'creature_quickness', '155'),
  (103, 'creature_stamina', '155'),
  (103, 'creature_mind', '155'),
  (103, 'creature_focus', '155'),
  (103, 'creature_willpower', '155'),
  (103, 'creature_ham_min', '140'),
  (103, 'creature_ham_max', '170'),
  (103, 'creature_tohit', '25'),
  (103, 'creature_damage_min', '45'),
  (103, 'creature_damage_max', '50'),
  (103, 'creature_xp', '85'),
  (103, 'creature_social_group', 'Lesser Desert Womprat'),
  (103, 'killer', '0'),
  (103, 'Pack', '0'),
  (103, 'Herd', '1'),
  (103, 'stalking', '0'),
  (103, 'creature_is_aggressive', '0'),
  (103, 'creature_group_assist', '0'),
  (103, 'res_meat', 'Wild Meat'),
  (103, 'creature_meat_qty', '2'),
  (103, 'res_hide', 'Leathery Hide'),
  (103, 'creature_hide_qty', '2'),
  (103, 'res_bone', 'Mammal Bone'),
  (103, 'creature_bone_qty', '1'),
  (104, 'creature_health', '1400'),
  (104, 'creature_strength', '1400'),
  (104, 'creature_constitution', '1400'),
  (104, 'creature_action', '1400'),
  (104, 'creature_quickness', '1400'),
  (104, 'creature_stamina', '1400'),
  (104, 'creature_mind', '1400'),
  (104, 'creature_focus', '1400'),
  (104, 'creature_willpower', '1400'),
  (104, 'creature_ham_min', '1050'),
  (104, 'creature_ham_max', '1750'),
  (104, 'creature_tohit', '30'),
  (104, 'creature_damage_min', '75'),
  (104, 'creature_damage_max', '80'),
  (104, 'creature_xp', '1085'),
  (104, 'creature_social_group', 'variegated womprat'),
  (104, 'killer', '0'),
  (104, 'Pack', '0'),
  (104, 'Herd', '1'),
  (104, 'stalking', '0'),
  (104, 'creature_is_aggressive', '0'),
  (104, 'creature_group_assist', '0'),
  (105, 'creature_health', '900'),
  (105, 'creature_strength', '900'),
  (105, 'creature_constitution', '900'),
  (105, 'creature_action', '900'),
  (105, 'creature_quickness', '900'),
  (105, 'creature_stamina', '900'),
  (105, 'creature_mind', '900'),
  (105, 'creature_focus', '900'),
  (105, 'creature_willpower', '900'),
  (105, 'creature_ham_min', '810'),
  (105, 'creature_ham_max', '990'),
  (105, 'creature_tohit', '28'),
  (105, 'creature_damage_min', '90'),
  (105, 'creature_damage_max', '110'),
  (105, 'creature_xp', '356'),
  (105, 'creature_social_group', 'Womprat'),
  (105, 'killer', '0'),
  (105, 'Pack', '1'),
  (105, 'Herd', '1'),
  (105, 'stalking', '0'),
  (105, 'creature_is_aggressive', '0'),
  (105, 'creature_group_assist', '1'),
  (105, 'res_meat', 'Wild Meat'),
  (105, 'creature_meat_qty', '4'),
  (105, 'res_hide', 'Leathery Hide'),
  (105, 'creature_hide_qty', '3'),
  (105, 'res_bone', 'Mammal Bone'),
  (105, 'creature_bone_qty', '2'),
  (106, 'creature_health', '200'),
  (106, 'creature_strength', '200'),
  (106, 'creature_constitution', '200'),
  (106, 'creature_action', '200'),
  (106, 'creature_quickness', '200'),
  (106, 'creature_stamina', '200'),
  (106, 'creature_mind', '200'),
  (106, 'creature_focus', '200'),
  (106, 'creature_willpower', '200'),
  (106, 'creature_ham_min', '180'),
  (106, 'creature_ham_max', '220'),
  (106, 'creature_tohit', '25'),
  (106, 'creature_damage_min', '50'),
  (106, 'creature_damage_max', '55'),
  (106, 'creature_xp', '113'),
  (106, 'creature_social_group', 'Rill'),
  (106, 'killer', '0'),
  (106, 'Pack', '0'),
  (106, 'Herd', '0'),
  (106, 'stalking', '0'),
  (106, 'creature_is_aggressive', '0'),
  (106, 'creature_group_assist', '0'),
  (106, 'res_meat', 'Carnivore Meat'),
  (106, 'creature_meat_qty', '35'),
  (106, 'res_hide', 'Bristley Hide'),
  (106, 'creature_hide_qty', '20'),
  (106, 'res_bone', 'Mammal Bone'),
  (106, 'creature_bone_qty', '15');

UPDATE swganh.non_persistent_npcs SET level = CASE id
    WHEN 101 THEN 10
    WHEN 102 THEN 16
    WHEN 103 THEN 5
    WHEN 104 THEN 14
    WHEN 105 THEN 10
    WHEN 106 THEN 6
    ELSE level END
  WHERE id IN (101, 102, 103, 104, 105, 106);

-- Existing attributes keep their place in the examine window; new ones are
-- added (internal ones are not shown there).
INSERT INTO swganh.non_persistent_npc_attributes (npc_id, attribute_id, value, `order`)
  SELECT c.npc_id, a.id, c.value, 0
  FROM creatures_011 c JOIN swganh.attributes a ON a.name = c.attribute
  JOIN swganh.non_persistent_npcs n ON n.id = c.npc_id
  ON DUPLICATE KEY UPDATE value = VALUES(value);

DROP TEMPORARY TABLE creatures_011;
