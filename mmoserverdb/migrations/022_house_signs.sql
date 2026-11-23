-- House signs (docs/KNOWN_ISSUES.md #30): pre-CU houses had a sign by the
-- door showing the house's name. The ZoneServer puts one up whenever a
-- house enters the world (it is not saved), from this table: per house
-- type, the sign's model and its place relative to the house.
--
-- From the Publish 14 client: the signs, object/tangible/sign/player/
-- shared_house_address.iff and its _corellia, _naboo and _tatooine
-- versions. The house templates mark no place for a sign, so x, y, z (metres,
-- in the house's own frame: x right, z ahead) and heading (degrees from the
-- house's facing) are reconstructed: a first guess, to correct from
-- screenshots of each house type.
CREATE TABLE IF NOT EXISTS swganh.house_sign_placement (
  structure_type INT(10) UNSIGNED NOT NULL,
  sign_template VARCHAR(128) NOT NULL,
  x FLOAT NOT NULL DEFAULT 0,
  y FLOAT NOT NULL DEFAULT 0,
  z FLOAT NOT NULL DEFAULT 0,
  heading FLOAT NOT NULL DEFAULT 0,
  PRIMARY KEY (structure_type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Every player house: the planet's sign where the house is a planet's
-- style, the generic one otherwise; in front of the house, beside the door.
INSERT IGNORE INTO swganh.house_sign_placement (structure_type, sign_template, x, y, z, heading)
SELECT s.type,
       CASE
         WHEN s.object_string LIKE '%house_corellia%' THEN 'object/tangible/sign/player/shared_house_address_corellia.iff'
         WHEN s.object_string LIKE '%house_naboo%'    THEN 'object/tangible/sign/player/shared_house_address_naboo.iff'
         WHEN s.object_string LIKE '%house_tatooine%' THEN 'object/tangible/sign/player/shared_house_address_tatooine.iff'
         ELSE 'object/tangible/sign/player/shared_house_address.iff'
       END,
       3.0, 0, CASE WHEN s.object_string LIKE '%large%' THEN 12.0 ELSE 8.0 END, 0
  FROM swganh.structure_type_data s
  WHERE s.object_string LIKE 'object/building/player/shared_player_house_%';
