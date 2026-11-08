-- Skill boxes made to match the client's own skill table, Publish 14
-- (datatables/skill/skills.iff, the pre-CU client), which the client
-- builds the skill tree and its mods from. Written by
-- tools/client/skills_compare.py --sql; docs/PROFESSIONS.md has the review.
-- Of 1,068 skills, these 16 differed (commands compared ignoring case,
-- a mod of 0 the same as none):
--
--   brawler   Intermediate Polearm gave pistol speed for polearm speed;
--             the polearm and two-handed toughness of the Expert and
--             Master Polearm and Two-Hand boxes were swapped; Master
--             Brawler lacked two-handed accuracy and speed +5, taunt +30
--             and one-handed toughness +5; Novice lacked unarmed Center
--             of Being efficacy +10
--   marksman  Pistol Specialist and Expert Ranged Support lacked ranged
--             defense +2 (both also in An Empire Divided, pp. 147-148)
--   carbineer ranged_damage_mitigation_3 is Master's, not Novice's
--   rifleman  Intermediate Accuracy rifle accuracy +15 (was 10); Master
--             rifle aim +40; the fourth boxes' rifle difficulty 125
--   bio-engineer Master assembly and experimentation +25 (were 0)
--   ranger    Master creature harvesting 60 (was 10)
--   scout     Intermediate camp foraging +10
--
-- Skills only on the server (admin_skill...) are left as they are.

DELETE sm FROM skills_skillmods sm JOIN skills s ON s.skill_id = sm.skill_id JOIN skillmods m ON m.skillmod_id = sm.skillmod_id WHERE s.skill_name = 'combat_brawler_2handmelee_03' AND m.skillmod_name = 'polearm_toughness';
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 10 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_2handmelee_03' AND m.skillmod_name = 'twohandmelee_toughness' ON DUPLICATE KEY UPDATE value = VALUES(value);
DELETE sm FROM skills_skillmods sm JOIN skills s ON s.skill_id = sm.skill_id JOIN skillmods m ON m.skillmod_id = sm.skillmod_id WHERE s.skill_name = 'combat_brawler_2handmelee_04' AND m.skillmod_name = 'polearm_toughness';
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 10 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_2handmelee_04' AND m.skillmod_name = 'twohandmelee_toughness' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 5 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_master' AND m.skillmod_name = 'onehandmelee_toughness' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 30 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_master' AND m.skillmod_name = 'taunt' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 5 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_master' AND m.skillmod_name = 'twohandmelee_accuracy' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 5 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_master' AND m.skillmod_name = 'twohandmelee_speed' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 10 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_novice' AND m.skillmod_name = 'unarmed_center_of_being_efficacy' ON DUPLICATE KEY UPDATE value = VALUES(value);
DELETE sm FROM skills_skillmods sm JOIN skills s ON s.skill_id = sm.skill_id JOIN skillmods m ON m.skillmod_id = sm.skillmod_id WHERE s.skill_name = 'combat_brawler_polearm_01' AND m.skillmod_name = 'pistol_speed';
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 5 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_polearm_01' AND m.skillmod_name = 'polearm_speed' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 4 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_polearm_03' AND m.skillmod_name = 'polearm_toughness' ON DUPLICATE KEY UPDATE value = VALUES(value);
DELETE sm FROM skills_skillmods sm JOIN skills s ON s.skill_id = sm.skill_id JOIN skillmods m ON m.skillmod_id = sm.skillmod_id WHERE s.skill_name = 'combat_brawler_polearm_03' AND m.skillmod_name = 'twohandmelee_toughness';
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 4 FROM skills s, skillmods m WHERE s.skill_name = 'combat_brawler_polearm_04' AND m.skillmod_name = 'polearm_toughness' ON DUPLICATE KEY UPDATE value = VALUES(value);
DELETE sm FROM skills_skillmods sm JOIN skills s ON s.skill_id = sm.skill_id JOIN skillmods m ON m.skillmod_id = sm.skillmod_id WHERE s.skill_name = 'combat_brawler_polearm_04' AND m.skillmod_name = 'twohandmelee_toughness';
INSERT IGNORE INTO skills_skillcommands (skill_id, skillcommand_id) SELECT s.skill_id, c.id FROM skills s, skillcommands c WHERE s.skill_name = 'combat_carbine_master' AND c.name = 'private_carbine_master';
INSERT IGNORE INTO skills_skillcommands (skill_id, skillcommand_id) SELECT s.skill_id, c.id FROM skills s, skillcommands c WHERE s.skill_name = 'combat_carbine_master' AND c.name = 'ranged_damage_mitigation_3';
DELETE sc FROM skills_skillcommands sc JOIN skills s ON s.skill_id = sc.skill_id JOIN skillcommands c ON c.id = sc.skillcommand_id WHERE s.skill_name = 'combat_carbine_novice' AND c.name = 'ranged_damage_mitigation_3';
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 2 FROM skills s, skillmods m WHERE s.skill_name = 'combat_marksman_pistol_04' AND m.skillmod_name = 'ranged_defense' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 2 FROM skills s, skillmods m WHERE s.skill_name = 'combat_marksman_support_03' AND m.skillmod_name = 'ranged_defense' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 15 FROM skills s, skillmods m WHERE s.skill_name = 'combat_rifleman_accuracy_02' AND m.skillmod_name = 'rifle_accuracy' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 125 FROM skills s, skillmods m WHERE s.skill_name = 'combat_rifleman_accuracy_04' AND m.skillmod_name = 'private_rifle_combat_difficulty' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 40 FROM skills s, skillmods m WHERE s.skill_name = 'combat_rifleman_master' AND m.skillmod_name = 'rifle_aim' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 125 FROM skills s, skillmods m WHERE s.skill_name = 'combat_rifleman_speed_04' AND m.skillmod_name = 'private_rifle_combat_difficulty' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 25 FROM skills s, skillmods m WHERE s.skill_name = 'outdoors_bio_engineer_master' AND m.skillmod_name = 'bio_engineer_assembly' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 25 FROM skills s, skillmods m WHERE s.skill_name = 'outdoors_bio_engineer_master' AND m.skillmod_name = 'bio_engineer_experimentation' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 60 FROM skills s, skillmods m WHERE s.skill_name = 'outdoors_ranger_master' AND m.skillmod_name = 'creature_harvesting' ON DUPLICATE KEY UPDATE value = VALUES(value);
INSERT INTO skills_skillmods (skill_id, skillmod_id, value) SELECT s.skill_id, m.skillmod_id, 10 FROM skills s, skillmods m WHERE s.skill_name = 'outdoors_scout_camp_02' AND m.skillmod_name = 'foraging' ON DUPLICATE KEY UPDATE value = VALUES(value);
