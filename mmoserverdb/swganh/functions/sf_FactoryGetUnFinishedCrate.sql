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
-- Definition of function `sf_FactoryGetUnFinishedCrate`
--

DROP FUNCTION IF EXISTS `sf_FactoryGetUnFinishedCrate`;

DELIMITER $$

DROP FUNCTION IF EXISTS `swganh`.`sf_FactoryGetUnFinishedCrate` $$
CREATE DEFINER=`root`@`localhost` FUNCTION `sf_FactoryGetUnFinishedCrate`(hopperID BIGINT(20), schematicID BIGINT(20)) RETURNS bigint(20)
BEGIN

--
-- Declare Variables
-- it


  DECLARE item_serial VARCHAR(128);
  DECLARE amount INTEGER;
  DECLARE tangfamily INTEGER;
  DECLARE tangtype INTEGER;
  DECLARE thecrc BIGINT(20);
  DECLARE rescount INTEGER;
  DECLARE itemcount INTEGER;
  DECLARE template BIGINT(20);
  DECLARE dummy INTEGER;

  DECLARE crate BIGINT(20);
  DECLARE crate_type INTEGER;


--
-- gets the serial nr of the template
--

SELECT ms.serial FROM manufactureschematic ms WHERE ms.id = schematicID INTO item_serial;
SELECT ms.crc FROM manufactureschematic ms WHERE ms.id = schematicID INTO thecrc;
SELECT item_family FROM draft_schematic_item_link WHERE schematic_id = thecrc INTO tangfamily;
SELECT item_type FROM draft_schematic_item_link WHERE schematic_id = thecrc INTO tangtype;
SELECT i.crate_type FROM item_families i WHERE i.id = tangfamily INTO crate_type;

SELECT ms.itemTemplate FROM manufactureschematic ms WHERE ms.id = schematicID INTO template;



SELECT COUNT(*) FROM resource_containers WHERE parent_id = hopperID into rescount;
SELECT COUNT(*) FROM items WHERE parent_id = hopperID into itemcount;
--
-- gets the max amount for the crates
--

SELECT crate_size FROM item_families WHERE id = tangfamily INTO amount;

--
-- crate size of 0 means we dont get to create a crate
--

IF (amount = 0) THEN

--
-- returns 0 if the hopper is full returns 1 if we need to create the item without crate
--
  IF (itemcount + rescount >= 100) THEN
      RETURN 0;
  END IF;

  RETURN 1;
END IF;

--
-- get a crate with the same serial that is not full
--

SELECT i.id FROM items i INNER JOIN item_attributes ia ON (ia.item_id = i.id AND ia.attribute_id = 400) INNER JOIN items ii ON (i.id = ii.parent_id) INNER JOIN item_attributes iaa ON (iaa.item_id = ii.id AND iaa.attribute_id = 16) WHERE iaa.value like item_serial AND (CAST(ia.value AS signed) < amount) AND i.parent_id = hopperID LIMIT 1 INTO crate;

IF (crate IS NOT NULL) THEN
  RETURN crate;
END IF;


--
-- is there enough room for a new crate ???
--

IF (itemcount + rescount >= 100) THEN
      RETURN 0;
END IF;

--
-- create a new crate
--

SELECT sf_DefaultItemCreate(30,crate_type,hopperID,0,99,0,0,0,"") INTO crate;
SELECT sf_DefaultItemCreatebyTangibleTemplate(crate,template) INTO dummy;

--
-- now create and link a template tangible
--

RETURN crate;

--
-- Return id
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
