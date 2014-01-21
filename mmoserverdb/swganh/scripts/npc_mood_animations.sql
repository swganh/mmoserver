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

--
-- Use schema swganh
--

USE swganh;
--
-- Definition of table `npc_mood_animations`
--

DROP TABLE IF EXISTS `npc_mood_animations`;
CREATE TABLE `npc_mood_animations` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `animation_type` varchar(45) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=24 DEFAULT CHARSET=utf8 COMMENT='animations for the npcs';

--
-- Dumping data for table `npc_mood_animations`
--

/*!40000 ALTER TABLE `npc_mood_animations` DISABLE KEYS */;
INSERT INTO `npc_mood_animations` (`id`,`animation_type`) VALUES 
 (1,'bored'),
 (2,'calm'),
 (3,'conversation'),
 (4,'default'),
 (5,'explain'),
 (6,'none'),
 (7,'neutral'),
 (8,'npc_accusing'),
 (9,'npc_angry'),
 (10,'npc_consoling'),
 (11,'npc_dead_01'),
 (12,'npc_dead_02'),
 (13,'npc_dead_03'),
 (14,'npc_imperial'),
 (15,'npc_sad'),
 (16,'npc_sitting_chair'),
 (17,'npc_sitting_ground'),
 (18,'npc_sitting_table'),
 (19,'npc_sitting_table_eating'),
 (20,'npc_standing_drinking'),
 (21,'npc_use_console_ag'),
 (22,'npc_use_terminal_high'),
 (23,'npc_use_terminal_low');
/*!40000 ALTER TABLE `npc_mood_animations` ENABLE KEYS */;




/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
