/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

USE swganh;

--
-- Definition of function `sf_FactoryIngredients`
--
-- Whether a factory's input hopper holds what its manufacturing schematic
-- takes for one item (manufactureschematic_ingredients), and with consume
-- set, takes it. Returns 1 when everything is there (or the schematic has
-- no ingredients recorded: made before they were), 0 when something is
-- missing. Resources come out of the hopper's resource containers (an
-- emptied container stays, at 0, for the player to take out: the zone may
-- still hold it); components are items of the slot's type, with the same
-- serial for an identical component, one item each (no crates).
--

DELIMITER $$

DROP FUNCTION IF EXISTS `swganh`.`sf_FactoryIngredients` $$
CREATE FUNCTION `sf_FactoryIngredients`(fID BIGINT(20), consume TINYINT) RETURNS int(11) SQL SECURITY INVOKER
BEGIN
  DECLARE done INT DEFAULT 0;
  DECLARE manID BIGINT(20) DEFAULT NULL;
  DECLARE inHopper BIGINT(20) DEFAULT NULL;
  DECLARE vKind INT;
  DECLARE vResource BIGINT(20);
  DECLARE vType INT;
  DECLARE vSerial VARCHAR(64);
  DECLARE vAmount INT;
  DECLARE have INT;
  DECLARE need INT;
  DECLARE containerID BIGINT(20);
  DECLARE containerAmount INT;
  DECLARE take INT;
  DECLARE itemID BIGINT(20);
  DECLARE taken INT;

  DECLARE ingredients CURSOR FOR
    SELECT kind, resource_id, item_type, serial, amount
    FROM manufactureschematic_ingredients WHERE schematic_id = manID ORDER BY slot;
  DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;

  SELECT ManSchematicID FROM factories WHERE id = fID INTO manID;
  SELECT id FROM items WHERE item_type = 2773 AND parent_id = fID LIMIT 1 INTO inHopper;
  SET done = 0;
  IF manID IS NULL OR inHopper IS NULL THEN
    RETURN 0;
  END IF;

  OPEN ingredients;
  each_ingredient: LOOP
    FETCH ingredients INTO vKind, vResource, vType, vSerial, vAmount;
    IF done THEN
      LEAVE each_ingredient;
    END IF;

    IF vKind = 4 THEN
      SELECT COALESCE(SUM(amount), 0) FROM resource_containers
        WHERE parent_id = inHopper AND resource_id = vResource INTO have;
      IF have < vAmount THEN
        CLOSE ingredients;
        RETURN 0;
      END IF;
      IF consume THEN
        SET need = vAmount;
        WHILE need > 0 DO
          SELECT id, amount FROM resource_containers
            WHERE parent_id = inHopper AND resource_id = vResource AND amount > 0
            ORDER BY amount LIMIT 1 INTO containerID, containerAmount;
          SET take = LEAST(need, containerAmount);
          UPDATE resource_containers SET amount = amount - take WHERE id = containerID;
          SET need = need - take;
        END WHILE;
      END IF;
    ELSE
      SELECT COUNT(*) FROM items i
        WHERE i.parent_id = inHopper AND i.item_type = vType
          AND (vKind <> 2 OR EXISTS (SELECT 1 FROM item_attributes ia
                 WHERE ia.item_id = i.id AND ia.attribute_id = 16 AND ia.value = vSerial))
        INTO have;
      IF have < vAmount THEN
        CLOSE ingredients;
        RETURN 0;
      END IF;
      IF consume THEN
        SET taken = 0;
        WHILE taken < vAmount DO
          SELECT i.id FROM items i
            WHERE i.parent_id = inHopper AND i.item_type = vType
              AND (vKind <> 2 OR EXISTS (SELECT 1 FROM item_attributes ia
                     WHERE ia.item_id = i.id AND ia.attribute_id = 16 AND ia.value = vSerial))
            LIMIT 1 INTO itemID;
          DELETE FROM item_attributes WHERE item_id = itemID;
          DELETE FROM items WHERE id = itemID;
          SET taken = taken + 1;
        END WHILE;
      END IF;
    END IF;
  END LOOP;
  CLOSE ingredients;

  RETURN 1;
END $$

DELIMITER ;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
