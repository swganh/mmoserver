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
-- Definition of table `container_types`
--

DROP TABLE IF EXISTS `container_types`;
CREATE TABLE `container_types` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `object_string` char(255) NOT NULL,
  `name` char(255) NOT NULL default 'unknown',
  `file` char(255) NOT NULL default 'container_n',
  `details_file` char(255) NOT NULL default 'container_detail',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `pk_containers_objstr` (`object_string`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `container_types`
--

/*!40000 ALTER TABLE `container_types` DISABLE KEYS */;
INSERT INTO `container_types` (`id`,`object_string`,`name`,`file`,`details_file`) VALUES 
 (1,'object/tangible/container/loot/shared_placable_loot_crate.iff','placable_loot_crate','container_name','container_detail'),
 (2,'object/tangible/container/loot/shared_placable_loot_crate_trashpile.iff','placable_loot_crate_trashpile','container_name','container_detail'),
 (3,'object/tangible/container/loot/shared_placable_loot_crate_skeleton_bith.iff','placable_loot_crate_skeleton','container_name','container_detail'),
 (4,'object/tangible/container/loot/shared_placable_loot_crate_skeleton_human.iff','placable_loot_crate_skeleton','container_name','container_detail'),
 (5,'object/tangible/container/loot/shared_placable_loot_crate_skeleton_ithorian.iff','placable_loot_crate_skeleton','container_name','container_detail'),
 (6,'object/tangible/container/drum/shared_warren_reactor_core.iff','treasure_drum','container_name','container_detail'),
 (7,'object/tangible/container/loot/shared_placable_loot_crate_tech_armoire.iff','tech_armoire','container_name','container_detail'),
 (8,'object/tangible/container/loot/shared_placable_loot_crate_tech_chest.iff','tech_chest','container_name','container_detail'),
 (9,'object/tangible/container/drum/shared_warren_drum_skeleton.iff','warren_skeleton','container_name','container_detail'),
 (10,'object/tangible/container/drum/shared_warren_drum_loot.iff','treasure_drum','container_name','container_detail'),
 (11,'object/tangible/container/drum/shared_tatt_drum_1.iff','tatt_drum','container_name','container_detail'),
 (12,'object/tangible/container/drum/shared_warren_drum_tech_chest.iff','warren_tech_chest','container_name','container_detail'),
 (13,'object/tangible/container/drum/shared_warren_drum_tech_armoire.iff','warren_tech_armoire','container_name','container_detail');
/*!40000 ALTER TABLE `container_types` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;