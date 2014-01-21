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
-- Definition of function `sf_DefaultVehicleCreate`
--

DROP FUNCTION IF EXISTS `sf_DefaultVehicleCreate`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='' */ $$
CREATE FUNCTION `sf_DefaultVehicleCreate`(type_id INT(11),parent_id BIGINT(20)) RETURNS bigint(20)
BEGIN

  --
  -- Declare Vars
  --

  DECLARE tmpId BIGINT(20);
  DECLARE att_id INT;
  DECLARE att_order INT;
  DECLARE att_value CHAR(255);
  DECLARE vehicle_name CHAR(255);
  DECLARE vehicle_type BIGINT(20);
  DECLARE loopEnd INT DEFAULT 0;
  DECLARE datapad_id BIGINT;
  DECLARE character_firstname CHAR(255);
  DECLARE character_lastname CHAR(255);

  DECLARE cur_1 CURSOR FOR SELECT attribute_id, attribute_value, attribute_order FROM vehicle_type_attributes WHERE vehicle_types_id = vehicle_type;

  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET loopEnd = 1;

  --
  -- Set our datapad ID
  --

  SET datapad_id = parent_id + 3;

  --
  -- Find the vehicle name
  --

  SELECT stf_name FROM item_types WHERE id = type_id INTO vehicle_name;

  --
  -- Select the vehicle type
  --

  SELECT id FROM vehicle_types WHERE vehicle_object_string LIKE CONCAT('%', vehicle_name, '.iff') INTO vehicle_type;

	IF vehicle_type = NULL THEN
	    return 0;
	END IF;

  --
  -- Create the new vehicle
  --

  SELECT MAX(id) + 2 FROM vehicles INTO tmpId;

  INSERT INTO vehicles VALUES (tmpId, vehicle_type, datapad_id, '', 0, 0, 0, 0, 0, 0, 0, 99, 1, 1, 1, 1);

  --
  -- Create the vehicle attributes
  --

  OPEN cur_1;
    REPEAT
      FETCH cur_1 INTO att_id, att_value, att_order;
        IF NOT loopEnd THEN
          INSERT INTO vehicle_attributes VALUES (tmpId, att_id, att_value, att_order);
        END IF;
    UNTIL loopEnd END REPEAT;
  CLOSE cur_1;

  --
  -- Find out the character Firstname / Lastname
  --

  SELECT firstname, lastname FROM characters WHERE id = parent_id INTO character_firstname, character_lastname;

  --
  -- Update the owner attribute with the character firstname / lastname
  --

    UPDATE vehicle_attributes SET attribute_value = character_firstname WHERE attribute_id = 568 AND vehicles_id = tmpId;
  --
  -- Return the new vehicle ID
  --

  RETURN(tmpId);

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