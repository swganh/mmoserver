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
-- Definition of table `zone_regions`
--

DROP TABLE IF EXISTS `zone_regions`;
CREATE TABLE `zone_regions` (
  `Id` int(11) unsigned NOT NULL auto_increment,
  `region_id` int(11) unsigned NOT NULL default '1',
  `planet_id` int(2) NOT NULL default '0',
  `qtDepth` int(2) unsigned NOT NULL default '8',
  PRIMARY KEY  (`Id`),
  KEY `fk_zone_regions_region` (`region_id`),
  KEY `fk_zone_region_planet` (`planet_id`),
  CONSTRAINT `fk_zone_regions_region` FOREIGN KEY (`region_id`) REFERENCES `planet_regions` (`region_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_zone_region_planet` FOREIGN KEY (`planet_id`) REFERENCES `planet` (`planet_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `zone_regions`
--

/*!40000 ALTER TABLE `zone_regions` DISABLE KEYS */;
INSERT INTO `zone_regions` (`Id`,`region_id`,`planet_id`,`qtDepth`) VALUES 
 (1,86,8,8),
 (2,87,0,8),
 (3,88,1,8),
 (4,89,2,8),
 (5,90,3,8),
 (6,91,4,8),
 (7,92,5,8),
 (8,93,6,8),
 (9,94,7,8),
 (10,95,9,8),
 (11,96,42,8);
/*!40000 ALTER TABLE `zone_regions` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
