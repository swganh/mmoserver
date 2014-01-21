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
-- Definition of function `sf_HarvestResource`
--

DROP FUNCTION IF EXISTS `sf_HarvestResource`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ $$
CREATE DEFINER=`root`@`localhost` FUNCTION `sf_HarvestResource`(hID BIGINT(20)) RETURNS int(11)
BEGIN

--
-- Declare Variables
-- it

  DECLARE rate float;
  DECLARE resID BIGINT(20);
  DECLARE hoppersize INTEGER;
  DECLARE quantity INTEGER;

  DECLARE active INTEGER;

--
-- set a proper exit handler in case we have a faulty resource ID
--

  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000'
  BEGIN
    UPDATE harvesters h SET h.ResourceID = 0 WHERE h.ID = hID;
    UPDATE harvesters h SET h.active = 0 WHERE h.ID = hID;
    RETURN 3;
  END;


--
-- get the extraction rate and the relevant resource ID
--
  SELECT h.rate FROM harvesters h WHERE (h.ID = hID)  INTO rate;
  SELECT h.ResourceID FROM harvesters h WHERE (h.ID = hID)  INTO resID;


--
-- check whether our resource is still available
--
  SELECT r.active FROM resources r WHERE (r.id = resID)  INTO active;
  IF (active =0) THEN
    UPDATE harvesters h SET h.ResourceID = 0 WHERE h.ID = hID;
    UPDATE harvesters h SET h.active = 0 WHERE h.ID = hID;
    RETURN 2;
  END IF;



--
-- get the hoppersize
--

  SELECT sa.value FROM structure_attributes sa WHERE sa.structure_id =hID AND sa.attribute_id = 381 INTO hoppersize;

--
-- Now make sure the resource is still available and active
--


--
-- update the resources quantity
--

  UPDATE harvester_resources hr SET hr.quantity = hr.quantity+(rate/60) WHERE hr.ID = hID AND hr.resourceID = resID;

--
-- now delete all the zero quantity entries
--

  DELETE FROM harvester_resources WHERE ((hID = ID) AND (quantity < 1.0) AND (ResourceID != resID));


--
-- still have to make sure that our hopper isnt full - turn harvester off in this event
--
  SELECT SUM(hr.quantity) FROM harvester_resources hr WHERE (hr.ID = hID)  INTO quantity;

  IF(quantity>=hoppersize)THEN
     UPDATE harvesters h SET h.active = 0 WHERE h.ID = hID;
     RETURN 1;
  END IF;



--
-- Return the Harvesters ID
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
