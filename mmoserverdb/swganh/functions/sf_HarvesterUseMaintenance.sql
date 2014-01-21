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
CREATE DEFINER=`root`@`localhost` FUNCTION `sf_HarvesterUseMaintenance`(hID BIGINT(20)) RETURNS int(11)
BEGIN

--
-- Declare Variables
-- it

  DECLARE maint INTEGER;
  DECLARE maintcalc INTEGER;
  DECLARE maintchar VARCHAR(128);
  DECLARE rate INTEGER;
  DECLARE decayrate INTEGER;
  DECLARE quantity INTEGER;

  DECLARE active INTEGER;
  DECLARE struct_condition INTEGER;
  DECLARE percent FLOAT;
  DECLARE owner BIGINT(20);
  DECLARE bank INTEGER;
  DECLARE cr INTEGER;
  DECLARE ret INTEGER;

--
-- set a proper exit handler in case we have a faulty resource ID
--

  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000'
  BEGIN
    UPDATE harvesters h SET h.active = 0 WHERE h.ID = hID;
    RETURN 3;
  END;

--
-- Get the owners id in case the maintenance is drained
--
  SELECT s.owner FROM structures s WHERE s.ID =hID INTO owner;
  SELECT b.credits FROM banks b WHERE b.id =(owner+4) INTO bank;

--
-- get the maintenance reserves
--

  SELECT sa.value FROM structure_attributes sa WHERE sa.structure_id =hID AND sa.attribute_id = 382 INTO maintchar;
  SELECT CAST(maintchar AS SIGNED) INTO maint;

--
-- get the maintenance rate
--

  SELECT st.maint_cost_wk FROM structures s INNER JOIN structure_type_data st ON (s.type = st.type) WHERE s.ID =hID  INTO rate;
  SELECT st.decay_rate FROM structures s INNER JOIN structure_type_data st ON (s.type = st.type) WHERE s.ID =hID  INTO decayrate;

--
-- rate/168 is hourly maintenance - we need to deduct it every half hour as every 30 min is the condition damage intervall
--

  IF(maint >= rate)THEN
      SELECT CAST((maint - (rate/336)) AS SIGNED) INTO maintcalc;
      SELECT CAST((maintcalc) AS CHAR(128)) INTO maintchar;
      UPDATE structure_attributes sa SET sa.VALUE = maintchar WHERE sa.structure_id =hID AND sa.attribute_id = 382;

--
-- Return 0 for everything ok
--


      return 0;


   END IF;

  IF(maint < rate)THEN
    SELECT ((rate/336)-maint) INTO cr;

    SELECT '0' INTO maintchar;

    UPDATE structure_attributes sa SET sa.VALUE = maintchar WHERE sa.structure_id =hID AND sa.attribute_id = 382;
    UPDATE harvesters h SET h.active = 0 WHERE h.ID = hID;


  END IF;



  IF(bank >= cr) THEN

    UPDATE banks SET credits = credits-cr WHERE id =(owner+4);

--
-- Return 1 for structure out of maintenance having taken maintenance out of the bank
--

    return 1;

  END IF;


  IF(bank < cr) THEN

    UPDATE banks SET credits = 0 WHERE id =(owner+4);

    SELECT((rate/336) - bank) INTO cr;
--
-- reduce the condition partly due to partly maintenance missing
--

    SELECT (cr/(rate/100)) INTO percent;
    UPDATE structures s SET s.condition = (s.condition -(decayrate*percent)) WHERE s.ID =hID;

--
-- damage the structures Condition
--

    SELECT s.condition FROM structures s WHERE s.ID =hID  INTO struct_condition;

--
-- notify if the structure needs to be condemned thats return code 3
--

    if(struct_condition <= 0) THEN
      UPDATE structures s SET s.condition = 0 WHERE s.ID =hID;
      return 3;
    END IF;



    UPDATE harvesters h SET h.active = 0 WHERE h.ID = hID;
--
-- Return 2 for structure out of maintenance AND Bank account empty - structure damaged
--

    return 2;

  END IF;



--
-- Return 5 for mess up
--

  RETURN 5;

--
-- Exit
--

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