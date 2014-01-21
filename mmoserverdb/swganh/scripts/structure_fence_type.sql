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
-- Definition of table `structure_fence_type`
--

DROP TABLE IF EXISTS `structure_fence_type`;
CREATE TABLE `structure_fence_type` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `object_sting` varchar(128) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=28 DEFAULT CHARSET=utf8 COMMENT='what construction fence type structures use.';

--
-- Dumping data for table `structure_fence_type`
--

/*!40000 ALTER TABLE `structure_fence_type` DISABLE KEYS */;
INSERT INTO `structure_fence_type` (`id`,`object_sting`) VALUES 
 (1,'object/building/player/construction/shared_construction_player_guildhall_corellia_style_01.iff'),
 (2,'object/building/player/construction/shared_construction_player_guildhall_naboo_style_01.iff'),
 (3,'object/building/player/construction/shared_construction_player_guildhall_tatooine_style_01.iff'),
 (4,'object/building/player/construction/shared_construction_player_guildhall_tatooine_style_02.iff'),
 (5,'object/building/player/construction/shared_construction_player_house_corellia_large_style_01.iff'),
 (6,'object/building/player/construction/shared_construction_player_house_corellia_large_style_02.iff'),
 (7,'object/building/player/construction/shared_construction_player_house_corellia_medium_style_01.iff'),
 (8,'object/building/player/construction/shared_construction_player_house_corellia_medium_style_02.iff'),
 (9,'object/building/player/construction/shared_construction_player_house_corellia_small_style_01.iff'),
 (10,'object/building/player/construction/shared_construction_player_house_corellia_small_style_02.iff'),
 (11,'object/building/player/construction/shared_construction_player_house_generic_large_style_01.iff'),
 (12,'object/building/player/construction/shared_construction_player_house_generic_large_style_02.iff'),
 (13,'object/building/player/construction/shared_construction_player_house_generic_medium_style_01.iff'),
 (14,'object/building/player/construction/shared_construction_player_house_generic_medium_style_02.iff'),
 (15,'object/building/player/construction/shared_construction_player_house_generic_small_style_01.iff'),
 (16,'object/building/player/construction/shared_construction_player_house_generic_small_style_02.iff'),
 (17,'object/building/player/construction/shared_construction_player_house_naboo_large_style_01.iff'),
 (18,'object/building/player/construction/shared_construction_player_house_naboo_medium_style_01.iff'),
 (19,'object/building/player/construction/shared_construction_player_house_naboo_small_style_01.iff'),
 (20,'object/building/player/construction/shared_construction_player_house_naboo_small_style_02.iff'),
 (21,'object/building/player/construction/shared_construction_player_house_tatooine_large_style_01.iff'),
 (22,'object/building/player/construction/shared_construction_player_house_tatooine_medium_style_01.iff'),
 (23,'object/building/player/construction/shared_construction_player_house_tatooine_small_style_01.iff'),
 (24,'object/installation/mining_ore/construction/shared_construction_mining_ore_harvester_style_1.iff'),
 (25,'object/installation/mining_ore/construction/shared_construction_mining_ore_harvester_style_2.iff'),
 (26,'object/installation/mining_ore/construction/shared_construction_mining_ore_harvester_style_heavy.iff'),
 (27,'object/installation/base/shared_construction_installation_base.iff');
/*!40000 ALTER TABLE `structure_fence_type` ENABLE KEYS */;




/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
