-- House signs (migration 022, docs/KNOWN_ISSUES.md #30): mounted on the wall
-- by the door, as in a screenshot of a pre-CU house: raised 2.5 m and turned
-- 135 degrees, chosen in test placements on a Tatooine small house (Nov
-- 2026). The other house types share the sign model and take the same
-- height and turn; their offsets are still the first guess. Reconstructed.
UPDATE swganh.house_sign_placement SET y = 2.5, heading = 135;
