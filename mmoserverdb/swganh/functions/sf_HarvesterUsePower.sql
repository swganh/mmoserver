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
-- Definition of function `sf_HarvesterUsePower`
--

DROP FUNCTION IF EXISTS `sf_HarvesterUsePower`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ $$
CREATE DEFINER=`root`@`localhost` FUNCTION `sf_HarvesterUsePower`(hID BIGINT(20)) RETURNS int(11)
BEGIN

--
-- Declare Variables
-- it

  DECLARE power INTEGER;
  DECLARE powerchar VARCHAR(128);
  DECLARE rate INTEGER;
  DECLARE quantity INTEGER;

  DECLARE active INTEGER;

--
-- set a proper exit handler in case we have a faulty resource ID
--

  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000'
  BEGIN
    UPDATE harvesters h SET h.active = 0 WHERE h.ID = hID;
    RETURN 3;
  END;


--
-- get the power rate
--

  SELECT st.power_used FROM structures s INNER JOIN structure_type_data st ON (s.type = st.type) WHERE s.ID =hID  INTO rate;


--
-- generators dont need power
--

  IF(rate <= 0) THEN
    RETURN 0;
  END IF;

--
-- get the power reserves
--

  SELECT sa.value FROM structure_attributes sa WHERE sa.structure_id =hID AND sa.attribute_id = 384 INTO powerchar;
  SELECT CAST(powerchar AS SIGNED) INTO power;


--
-- do we have power in the first place ?
--

  IF((power <= 0) OR (power < (rate))) THEN
    UPDATE harvesters h SET h.active = 0 WHERE h.ID = hID;

--
-- Return 1 for structure out of power
--

    return 1;
  END IF;



  SELECT CAST((power - (rate)) AS CHAR(128)) INTO powerchar;
  UPDATE structure_attributes sa SET sa.VALUE = powerchar WHERE sa.structure_id =hID AND sa.attribute_id = 384;



--
-- Return 0 for uneventful cycle
--

  RETURN 0;

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
