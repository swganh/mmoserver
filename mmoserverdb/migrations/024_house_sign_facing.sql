-- House signs (migrations 022-023, docs/KNOWN_ISSUES.md #30): turned 180
-- degrees, so the sign's wall post is against the wall and the board
-- stands out from it, as in screenshots of pre-CU Tatooine houses. Chosen in
-- test placements on a Tatooine small house (Nov 2026) and applied to every
-- house type. Reconstructed.
UPDATE swganh.house_sign_placement SET heading = 180;
