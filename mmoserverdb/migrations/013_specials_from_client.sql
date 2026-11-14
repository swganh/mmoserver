-- Specials where the client's own command descriptions (Publish 14,
-- string/en/cmd_d.stf) contradict An Empire Divided's special moves table
-- (pp. 76-81, migrations 010 and 012). The client is the baseline
-- (docs/SOURCES.md); the guide's values stay in
-- mmoserverdb/reference/prima_ed/specials.csv as printed.
--
--   melee2hArea3      "can blind, and damage and lower the posture of
--                     multiple targets"; the table has dizzy. Blind with the
--                     table's dizzy chance and duration (30%, 30 s).
--   burstShot2        "This area ranged attack is more damaging than Burst
--                     Shot 1"; the table has a single target. An area of the
--                     weapon's range, like the other areas without a size.
--   pointBlankArea2   "This advanced ranged area attack can be performed at
--                     point blank range"; the table has a single target. The
--                     12 m of Point Blank Area 1.
--   chargeShot2       "can knock multiple targets down at once"; the table's
--                     own description says "knockdown cone" but its target
--                     column says single, with no knockdown. A knockdown
--                     cone, as Charge Shot 1 knocks down.
--   wildShot2         "can stun multiple targets at once"; the table says
--                     "stuns cone" but single. A cone.
--   multiTargetPistolShot, multiTargetShot
--                     "can hit multiple targets at once"; the table has
--                     single targets. Cones.
-- The cones take the table's size for every other cone, 64 m by 15 m:
-- ranged attacks on several targets hit "the ranged weapon's cone of fire"
-- (p. 75).

UPDATE command_table SET blind_chance = dizzy_chance, blind_duration = dizzy_duration,
    dizzy_chance = 0, dizzy_duration = 0
  WHERE LOWER(commandname) = 'melee2harea3' AND dizzy_chance > 0;

UPDATE command_table SET area_shape = 'area', area_length = 0, area_width = 0
  WHERE LOWER(commandname) = 'burstshot2';

UPDATE command_table SET area_shape = 'area', area_length = 12, area_width = 0
  WHERE LOWER(commandname) = 'pointblankarea2';

UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15, knockdown_chance = 100
  WHERE LOWER(commandname) = 'chargeshot2';

UPDATE command_table SET area_shape = 'cone', area_length = 64, area_width = 15
  WHERE LOWER(commandname) IN ('wildshot2', 'multitargetpistolshot', 'multitargetshot');
