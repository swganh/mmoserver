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
-- Definition of function `sf_FactoryCreateItem`
--

DROP FUNCTION IF EXISTS `sfsf_FactoryCreateItem`;

DELIMITER $$

DROP FUNCTION IF EXISTS `swganh`.`sf_FactoryCreateItem` $$
CREATE DEFINER=`root`@`localhost` FUNCTION `sf_FactoryCreateItem`(fID BIGINT(20)) RETURNS int(11)
BEGIN

--
-- Declare Variables
-- it

  DECLARE rate float;
  DECLARE resID BIGINT(20);
  DECLARE manID BIGINT(20);
  DECLARE templateID BIGINT(20);
  DECLARE hopperID BIGINT(20);
  DECLARE crateID BIGINT(20);
  DECLARE valuechar VARCHAR(128);
  DECLARE quantity INTEGER;


  DECLARE hoppersize INTEGER;


  DECLARE active INTEGER;
  DECLARE dummy INTEGER;
  DECLARE OutHopper BIGINT(20);
  DECLARE InHopper BIGINT(20);

  DECLARE itemcount INTEGER;



--
-- get the factories out/in put hopper
--

  SELECT id from items where item_type = 2774 AND parent_id = fID INTO OutHopper;
  SELECT id from items where item_type = 2773 AND parent_id = fID INTO InHopper;

--
-- get the schematics ID
--

  SELECT ManSchematicID FROM factories WHERE id = fID into manID;
  SELECT id FROM items WHERE parent_id = manID into templateID;


--
-- returns with the id of a crate that isnt full yet, creates if necessary an empty crate
-- returns 0 when the hopper is full or 1 if the item is not packaged in crates

  SELECT sf_FactoryGetUnfinishedCrate(OutHopper,manID) INTO crateID;

--
-- check whether the output hopper is full
--

IF (crateID = 0) THEN
  UPDATE factories f SET f.active = 0 WHERE f.ID = fID;
  return 2;
END IF;

--
-- if createID is 1 we need to create our item without a crate
--

IF (crateID = 1) THEN
  SELECT sf_DefaultItemCreatebyTangibleTemplate(OutHopper,templateID) INTO dummy;
  return 1;
END IF;


--
-- look up our counter uses remaining attribute and increment it attribute
--

SELECT ia.value FROM item_attributes ia WHERE ia.item_id = crateID AND ia.attribute_id = 400 INTO valuechar;

SELECT CAST(valuechar AS SIGNED) INTO quantity;
SELECT CAST(quantity+1 AS CHAR) INTO valuechar;

UPDATE item_attributes ia SET ia.VALUE = valuechar WHERE ia.item_id = crateID AND ia.attribute_id = 400;



--
-- Return 1 for succesful cycle
--

  RETURN 1;

--
-- Exit
--

END $$

DELIMITER ;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
