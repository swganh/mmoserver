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
-- Definition of function `sf_HarvesterUseMaintenance`
--

DROP FUNCTION IF EXISTS `sf_HarvesterUseMaintenance`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ $$
CREATE FUNCTION `sf_HarvesterUseMaintenance`(hID BIGINT(20)) RETURNS int(11) SQL SECURITY INVOKER
BEGIN

--
-- One hour's maintenance for a structure of any kind: the chat server runs
-- it hourly for every structure (houses, harvesters, factories...).
-- structure_type_data.maint_cost_wk is per week; the hour's charge is a
-- 168th of it (the status window shows the same rate). It took a 336th,
-- half the hour's charge, which rounded to nothing, and compared the pool
-- with the weekly rate (#39).
--
-- Returns 0 paid from the pool, 1 the pool ran out and the owner's bank
-- paid the rest, 2 nothing left to pay with: the structure loses
-- condition, and a harvester or factory stops, 3 its condition reached 0,
-- 5 not a structure with maintenance.
--

  DECLARE owner BIGINT(20) DEFAULT NULL;
  DECLARE weekly INTEGER DEFAULT NULL;
  DECLARE decayRate INTEGER DEFAULT 0;
  DECLARE maxCondition INTEGER DEFAULT 0;
  DECLARE hourly INTEGER;
  DECLARE poolChar VARCHAR(128) DEFAULT NULL;
  DECLARE pool INTEGER DEFAULT 0;
  DECLARE bank INTEGER DEFAULT 0;
  DECLARE shortfall INTEGER;
  DECLARE struct_condition INTEGER;

  -- a missing row leaves the variable as it was
  DECLARE CONTINUE HANDLER FOR NOT FOUND BEGIN END;

  SELECT s.owner, st.maint_cost_wk, st.decay_rate, st.max_condition
    FROM structures s INNER JOIN structure_type_data st ON (s.type = st.type)
    WHERE s.ID = hID INTO owner, weekly, decayRate, maxCondition;
  IF weekly IS NULL OR weekly <= 0 THEN
    RETURN 5;
  END IF;
  SET hourly = CEIL(weekly / 168);

  SELECT sa.value FROM structure_attributes sa
    WHERE sa.structure_id = hID AND sa.attribute_id = 382 INTO poolChar;
  SET pool = COALESCE(CAST(poolChar AS SIGNED), 0);

  IF pool >= hourly THEN
    UPDATE structure_attributes sa SET sa.value = CAST(pool - hourly AS CHAR(128))
      WHERE sa.structure_id = hID AND sa.attribute_id = 382;
    RETURN 0;
  END IF;

  -- the pool runs out: the bank pays the rest if it can
  SET shortfall = hourly - GREATEST(pool, 0);
  UPDATE structure_attributes sa SET sa.value = '0'
    WHERE sa.structure_id = hID AND sa.attribute_id = 382;

  SELECT b.credits FROM banks b WHERE b.id = owner + 4 INTO bank;
  IF bank >= shortfall THEN
    UPDATE banks SET credits = credits - shortfall WHERE id = owner + 4;
    RETURN 1;
  END IF;
  IF bank > 0 THEN
    UPDATE banks SET credits = 0 WHERE id = owner + 4;
    SET shortfall = shortfall - bank;
  END IF;

  -- nothing left to pay with: harvesters and factories stop, and the
  -- structure loses condition (health, down to 0): decay_rate percent of
  -- its maximum per unpaid hour (a factory, 6, lasts about 17 hours; a
  -- small house, 3, about 33). Reconstructed, not from a source: the old
  -- formula took 60% of a factory's condition in an unpaid hour.
  UPDATE harvesters h SET h.active = 0 WHERE h.ID = hID;
  UPDATE factories f SET f.active = 0 WHERE f.ID = hID;
  UPDATE structures s
    SET s.condition = GREATEST(0, CAST(s.condition AS SIGNED) - CEIL(maxCondition * decayRate / 100 * shortfall / hourly))
    WHERE s.ID = hID;

  SELECT s.condition FROM structures s WHERE s.ID = hID INTO struct_condition;
  IF struct_condition <= 0 THEN
    RETURN 3;
  END IF;
  RETURN 2;
END $$
/*!50003 SET SESSION SQL_MODE=@TEMP_SQL_MODE */  $$

DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;