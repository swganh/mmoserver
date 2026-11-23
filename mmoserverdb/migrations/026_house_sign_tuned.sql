-- House signs (migrations 022-025, docs/KNOWN_ISSUES.md #30): positions
-- tuned in game (Nov 2026) by moving each sign until it hung on the wall
-- beside the door, against screenshots of pre-CU Tatooine houses.
-- Reconstructed. The other house types keep the first guess.
UPDATE swganh.house_sign_placement SET x = -2, y = 2.5, z = 16.6, heading = 180 WHERE structure_type = 84; -- Tatooine large house
UPDATE swganh.house_sign_placement SET x = 3, y = 4.5, z = 12.5, heading = 180 WHERE structure_type = 47;  -- Tatooine guild hall
UPDATE swganh.house_sign_placement SET x = 5, y = 4, z = 18.5, heading = 180 WHERE structure_type = 45;    -- generic guild hall
