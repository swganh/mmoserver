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
-- Definition of table `travel_planet_routes`
--

DROP TABLE IF EXISTS `travel_planet_routes`;
CREATE TABLE `travel_planet_routes` (
  `srcId` int(2) NOT NULL,
  `destId` int(2) NOT NULL default '0',
  `price` int(11) unsigned NOT NULL default '1000',
  PRIMARY KEY  (`srcId`,`destId`),
  KEY `fk_travelroutes_dest_dest` (`destId`),
  CONSTRAINT `travel_planet_routes_ibfk_1` FOREIGN KEY (`srcId`) REFERENCES `planet` (`planet_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `travel_planet_routes_ibfk_2` FOREIGN KEY (`destId`) REFERENCES `planet` (`planet_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `travel_planet_routes`
--

/*!40000 ALTER TABLE `travel_planet_routes` DISABLE KEYS */;
INSERT INTO `travel_planet_routes` (`srcId`,`destId`,`price`) VALUES 
 (0,0,100),
 (0,1,1000),
 (0,2,2000),
 (0,3,4000),
 (0,5,500),
 (0,7,300),
 (0,8,600),
 (0,9,3000),
 (0,42,1250),
 (1,0,1000),
 (1,1,100),
 (2,0,2000),
 (2,2,100),
 (3,0,4000),
 (3,3,100),
 (3,5,1750),
 (4,4,100),
 (4,5,1250),
 (4,8,1250),
 (5,0,500),
 (5,3,1750),
 (5,4,1250),
 (5,5,100),
 (5,6,300),
 (5,8,500),
 (6,5,300),
 (6,6,100),
 (7,0,300),
 (7,7,100),
 (8,0,600),
 (8,4,1250),
 (8,5,500),
 (8,8,100),
 (9,0,3000),
 (9,9,100),
 (42,0,1250),
 (42,42,100);
/*!40000 ALTER TABLE `travel_planet_routes` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;