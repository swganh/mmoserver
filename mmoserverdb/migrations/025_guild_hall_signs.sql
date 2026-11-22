-- House signs (migrations 022-024, docs/KNOWN_ISSUES.md #30) for the player
-- guild halls too: the planet's sign for the Corellia, Naboo and Tatooine
-- halls, the generic one for the generic hall. Height and turn as for the
-- houses; the offset (14 m ahead) is a first guess, to correct from a
-- screenshot. Reconstructed.
INSERT IGNORE INTO swganh.house_sign_placement (structure_type, sign_template, x, y, z, heading)
SELECT s.type,
       CASE
         WHEN s.object_string LIKE '%guildhall_corellia%' THEN 'object/tangible/sign/player/shared_house_address_corellia.iff'
         WHEN s.object_string LIKE '%guildhall_naboo%'    THEN 'object/tangible/sign/player/shared_house_address_naboo.iff'
         WHEN s.object_string LIKE '%guildhall_tatooine%' THEN 'object/tangible/sign/player/shared_house_address_tatooine.iff'
         ELSE 'object/tangible/sign/player/shared_house_address.iff'
       END,
       3.0, 2.5, 14.0, 180
  FROM swganh.structure_type_data s
  WHERE s.object_string LIKE 'object/building/player/shared_player_guildhall_%';
