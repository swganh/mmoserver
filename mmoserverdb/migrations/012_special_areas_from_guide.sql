-- Specials' areas, cones and attacker postures from Prima's An Empire
-- Divided (2003; docs/SOURCES.md), special moves table (pp. 76-83),
-- transcribed in mmoserverdb/reference/prima_ed/specials.csv. Written by
-- tools/reference/special_areas_sql.py; edit that, not this.
--
-- New command_table columns:
--   area_shape            single, area or cone. "Area affects all targets
--                         within the melee weapon's range. Cone affects all
--                         targets with the ranged weapon's cone of fire"
--                         (p. 75).
--   area_length           metres: an area's radius around the attacker, a
--                         cone's length; 0 = the weapon's max range.
--   area_width            metres: how wide a cone is at its far end.
--   attacker_end_posture  the posture the attacker ends in (Dive Shot:
--                         prone, Kip Up Shot: standing, Roll Shot: kneeling),
--                         empty for the rest.
--
-- 32 of the table's 134 rows are areas or cones or move the attacker.
-- PointBlankArea1 is the one area with a size, 12 by 15: its 12 m is taken
-- as the radius.

ALTER TABLE command_table
  ADD COLUMN IF NOT EXISTS area_shape VARCHAR(8) NOT NULL DEFAULT 'single',
  ADD COLUMN IF NOT EXISTS area_length FLOAT NOT NULL DEFAULT 0,
  ADD COLUMN IF NOT EXISTS area_width FLOAT NOT NULL DEFAULT 0,
  ADD COLUMN IF NOT EXISTS attacker_end_posture VARCHAR(16) NOT NULL DEFAULT '';

-- 1H Melee BlindHit2: blind area (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee1hBlindHit2');
-- 1H Melee DizzyHit2: dizzy area (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee1hDizzyHit2');
-- 1H Melee SpinAttack1: area attack (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee1hSpinAttack1');
-- 1H Melee SpinAttack2: area attack + blind (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee1hSpinAttack2');
-- 2H Melee Area1: area hit + lower posture base damage (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee2hArea1');
-- 2H Melee Area2: area hit + damage + lower posture (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee2hArea2');
-- 2H Melee Area3: area hit + damage + dizzy + lower posture (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee2hArea3');
-- 2H Melee SpinAttack1: area attack (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee2hSpinAttack1');
-- 2H Melee SpinAttack2: area attack + more damage (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee2hSpinAttack2');
-- 2H Melee Sweep2: lower posture area, low damage (pp. 76-77)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('melee2hSweep2');
-- Carbine ActionShot2: action drain/regen (pp. 78-79)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('ActionShot2');
-- Carbine FullAutoArea1: chance to stun, blind, and dizzy multiple (pp. 78-79)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('FullAutoArea1');
-- Carbine FullAutoArea2: chance to stun, blind, and dizzy multiple (pp. 78-79)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('FullAutoArea2');
-- Carbine LegShot3: shoots legs (pp. 78-79)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('LegShot3');
-- Pistol DisarmingShot2: hits gun (pp. 78-79)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('DisarmingShot2');
-- Pistol DiveShot: to prone attack (pp. 78-79)
UPDATE command_table SET area_shape = 'single', area_length = 0, area_width = 0, attacker_end_posture = 'prone' WHERE LOWER(commandname) = LOWER('DiveShot');
-- Pistol KipUpShot: to standing attack (pp. 78-79)
UPDATE command_table SET area_shape = 'single', area_length = 0, area_width = 0, attacker_end_posture = 'standing' WHERE LOWER(commandname) = LOWER('KipUpShot');
-- Pistol PanicShot: delays everyone in cone (pp. 78-79)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('PanicShot');
-- Pistol RollShot: to kneeling attack (pp. 80-81)
UPDATE command_table SET area_shape = 'single', area_length = 0, area_width = 0, attacker_end_posture = 'kneeling' WHERE LOWER(commandname) = LOWER('RollShot');
-- Polearm Area1: area attack + damage (pp. 80-81)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('polearmArea1');
-- Polearm Area2: more damage + dizzy + stun (pp. 80-81)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('polearmArea2');
-- Polearm SpinAttack1: area attack (pp. 80-81)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('polearmSpinAttack1');
-- Polearm SpinAttack2: area attack + dizzy (pp. 80-81)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('polearmSpinAttack2');
-- Polearm Stun2: stun area (pp. 80-81)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('polearmStun2');
-- Polearm Sweep2: area knockdown (pp. 80-81)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('polearmSweep2');
-- Ranged PointBlankArea1: no close range penalty (pp. 80-81)
UPDATE command_table SET area_shape = 'area', area_length = 12, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('PointBlankArea1');
-- Rifle FlurryShot2: dizzies cone (pp. 80-81)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('FlurryShot2');
-- Rifle FlushingShot2: raises posture + stun (pp. 80-81)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('FlushingShot2');
-- Rifle StartleShot2: raises posture (pp. 82-83)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('StartleShot2');
-- Rifle StrafeShot2: clears cover + adds delay (pp. 82-83)
UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('StrafeShot2');
-- Unarmed SpinAttack1: area attack (pp. 82-83)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('unarmedSpinAttack1');
-- Unarmed SpinAttack2: area attack (pp. 82-83)
UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0, attacker_end_posture = '' WHERE LOWER(commandname) = LOWER('unarmedSpinAttack2');
