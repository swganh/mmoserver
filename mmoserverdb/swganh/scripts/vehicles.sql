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
-- Definition of table `vehicles`
--

DROP TABLE IF EXISTS `vehicles`;
CREATE TABLE `vehicles` (
  `id` bigint(20) NOT NULL auto_increment,
  `vehicle_types_id` bigint(20) default NULL,
  `parent` bigint(20) default NULL,
  `name` char(255) default NULL,
  `oX` float(10,6) default NULL,
  `oY` float(10,6) default NULL,
  `oZ` float(10,6) default NULL,
  `oW` float(10,6) default NULL,
  `x` float(10,6) default NULL,
  `y` float(10,6) default NULL,
  `z` float(10,6) default NULL,
  `planet_id` int(2) default NULL,
  `scale` char(255) default NULL,
  `vehicle_hitpoint_loss` int(10) unsigned NOT NULL,
  `vehicle_incline_acceleration` int(10) unsigned NOT NULL,
  `vehicle_flat_acceleration` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `fk_vehicles_vehicle_types` (`vehicle_types_id`),
  CONSTRAINT `fk_vehicles_vehicle_types` FOREIGN KEY (`vehicle_types_id`) REFERENCES `vehicle_types` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=30064771073 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `vehicles`
--

/*!40000 ALTER TABLE `vehicles` DISABLE KEYS */;
INSERT INTO `vehicles` (`id`,`vehicle_types_id`,`parent`,`name`,`oX`,`oY`,`oZ`,`oW`,`x`,`y`,`z`,`planet_id`,`scale`,`vehicle_hitpoint_loss`,`vehicle_incline_acceleration`,`vehicle_flat_acceleration`) VALUES
 (30064771072,1,0,NULL,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0,'0',0,0,0);
/*!40000 ALTER TABLE `vehicles` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;