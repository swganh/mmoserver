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
-- Definition of table `vehicle_types`
--

DROP TABLE IF EXISTS `vehicle_types`;
CREATE TABLE `vehicle_types` (
  `id` bigint(20) NOT NULL,
  `vehicle_families_id` bigint(20) default NULL,
  `vehicle_object_string` char(255) default NULL,
  `vehicle_itno_object_string` char(255) default NULL,
  `vehicle_name_file` char(255) default NULL,
  `vehicle_detail_file` char(255) default NULL,
  `vehicle_name` char(255) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `fk_vehicle_types_vehicle_families` (`vehicle_families_id`),
  CONSTRAINT `fk_vehicle_types_vehicle_families` FOREIGN KEY (`vehicle_families_id`) REFERENCES `vehicle_families` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `vehicle_types`
--

/*!40000 ALTER TABLE `vehicle_types` DISABLE KEYS */;
INSERT INTO `vehicle_types` (`id`,`vehicle_families_id`,`vehicle_object_string`,`vehicle_itno_object_string`,`vehicle_name_file`,`vehicle_detail_file`,`vehicle_name`) VALUES 
 (1,1,'object/mobile/vehicle/shared_jetpack.iff','object/intangible/vehicle/shared_jetpack_pcd.iff','monster_name','monster_detail','jetpack'),
 (2,1,'object/mobile/vehicle/shared_landspeeder_av21.iff','object/intangible/vehicle/shared_landspeeder_av21_pcd.iff','monster_name','monster_detail','landspeeder_av21'),
 (3,1,'object/mobile/vehicle/shared_landspeeder_base.iff',NULL,'monster_name','monster_detail',''),
 (4,1,'object/mobile/vehicle/shared_landspeeder_x31.iff','object/intangible/vehicle/shared_landspeeder_x31_pcd.iff','monster_name','monster_detail','landspeeder_x31'),
 (5,1,'object/mobile/vehicle/shared_landspeeder_x34.iff','object/intangible/vehicle/shared_landspeeder_x34_pcd.iff','monster_name','monster_detail','landspeeder_x34'),
 (6,1,'object/mobile/vehicle/shared_landspeeder_xp38.iff',NULL,'monster_name','monster_detail',''),
 (7,1,'object/mobile/vehicle/shared_speederbike.iff','object/intangible/vehicle/shared_speederbike_pcd.iff','monster_name','monster_detail','speederbike'),
 (8,1,'object/mobile/vehicle/shared_speederbike_base.iff',NULL,'monster_name','monster_detail',''),
 (9,1,'object/mobile/vehicle/shared_speederbike_flash.iff','object/intangible/vehicle/shared_speederbike_flash_pcd.iff','monster_name','monster_detail','speederbike_flash'),
 (10,1,'object/mobile/vehicle/shared_speederbike_swoop.iff','object/intangible/vehicle/shared_speederbike_swoop_pcd.iff','monster_name','monster_detail','speederbike_swoop'),
 (11,1,'object/mobile/vehicle/shared_vehicle_base.iff','object/intangible/vehicle/shared_vehicle_pcd_base.iff','monster_name','monster_detail','');
/*!40000 ALTER TABLE `vehicle_types` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;