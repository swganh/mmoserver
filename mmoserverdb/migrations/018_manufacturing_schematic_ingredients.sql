-- What a manufacturing schematic needs per item made, so a factory can take
-- it from its input hopper and List Ingredients can show it
-- (docs/KNOWN_ISSUES.md #36). One row per filled slot, written when the
-- schematic is made (CraftingSession::createManufactureSchematic).
--
-- kind is the slot's draft slot type: 4 a resource (resource_id), 2 an
-- identical component (item_type and serial must match), 5 a similar
-- component (item_type must match). amount is what one item takes.
-- name_file and name are the slot's string, for listing.
--
-- Schematics made before this have no rows; factories keep producing from
-- them without taking ingredients.
CREATE TABLE IF NOT EXISTS swganh.manufactureschematic_ingredients (
  schematic_id BIGINT(20) UNSIGNED NOT NULL,
  slot TINYINT(3) UNSIGNED NOT NULL,
  kind TINYINT(3) UNSIGNED NOT NULL,
  resource_id BIGINT(20) UNSIGNED NOT NULL DEFAULT 0,
  item_type INT(10) UNSIGNED NOT NULL DEFAULT 0,
  serial VARCHAR(64) NOT NULL DEFAULT '',
  amount INT(10) UNSIGNED NOT NULL,
  name_file VARCHAR(64) NOT NULL DEFAULT '',
  name VARCHAR(64) NOT NULL DEFAULT '',
  PRIMARY KEY (schematic_id, slot)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
