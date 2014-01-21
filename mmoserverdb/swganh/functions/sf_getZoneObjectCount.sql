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

use swganh;

--
-- Definition of function `sf_getZoneObjectCount`
--

DROP FUNCTION IF EXISTS `sf_getZoneObjectCount`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,NO_AUTO_CREATE_USER' */ $$
CREATE FUNCTION `sf_getZoneObjectCount`(zoneId INT) RETURNS int(11)
BEGIN

	DECLARE buildingCount INT;
	DECLARE cellCount INT;
	DECLARE terminalCount INT;
    DECLARE containerCount INT;
	DECLARE ticketCollectorCount INT;
  	DECLARE persistentNPCCount INT;
  	DECLARE shuttleCount INT;
   	DECLARE itemCount INT;
   	DECLARE resourceContainerCount INT;
    DECLARE cityCount INT;
    DECLARE badgeRegionCount INT;
	DECLARE zoneRegionCount INT;
	DECLARE totalCount INT;

	SELECT COUNT(id) FROM buildings WHERE planet_id = zoneId INTO buildingCount;
	SELECT COUNT(cells.id) FROM cells INNER JOIN buildings ON (cells.parent_id = buildings.id) WHERE (buildings.planet_id = zoneId) INTO cellCount;
	SELECT COUNT(terminals.id) FROM terminals INNER JOIN terminal_types ON(terminals.terminal_type = terminal_types.id) WHERE (terminals.planet_id = zoneId) AND (terminal_types.name NOT LIKE 'unknown') INTO terminalCount;
   	SELECT COUNT(containers.id) FROM containers INNER JOIN container_types ON(containers.container_type = container_types.id) WHERE (containers.planet_id = zoneId) AND (container_types.name NOT LIKE 'unknown') INTO containerCount;
	SELECT COUNT(id) FROM ticket_collectors WHERE planet_id = zoneId INTO ticketCollectorCount;
	SELECT COUNT(id) FROM persistent_npcs WHERE planet_id = zoneId INTO persistentNPCCount;
	SELECT COUNT(id) FROM shuttles WHERE planet_id = zoneId INTO shuttleCount;
   	SELECT COUNT(id) FROM items WHERE planet_id = zoneId INTO itemCount;
    SELECT COUNT(id) FROM resource_containers WHERE planet_id = zoneId INTO resourceContainerCount;
    SELECT COUNT(id) FROM cities WHERE planet_id = zoneId INTO cityCount;
    SELECT COUNT(id) FROM badge_regions WHERE planet_id = zoneId INTO badgeRegionCount;
    SELECT COUNT(id) FROM zone_regions WHERE planet_id = zoneId INTO zoneRegionCount;
    
    SET totalCount = (buildingCount + cellCount + terminalCount + resourceContainerCount + containerCount + ticketCollectorCount + persistentNPCCount + shuttleCount + itemCount + cityCount + badgeRegionCount + zoneRegionCount);

	RETURN totalCount;
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