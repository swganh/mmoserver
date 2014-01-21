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
-- Definition of table `conversations`
--

DROP TABLE IF EXISTS `conversations`;
CREATE TABLE `conversations` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `description` varchar(1024) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=34 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `conversations`
--

/*!40000 ALTER TABLE `conversations` DISABLE KEYS */;
INSERT INTO `conversations` (`id`,`description`) VALUES 
 (1,'brawler trainer dialog'),
 (2,'entertainer trainer dialog'),
 (3,'scout trainer dialog'),
 (4,'medic trainer dialog'),
 (5,'artisan trainer dialog'),
 (6,'marksman trainer dialog'),
 (7,'rifleman trainer dialog'),
 (8,'pistol trainer dialog'),
 (9,'carbine trainer dialog'),
 (10,'unarmed trainer dialog'),
 (11,'fencer trainer dialog'),
 (12,'swordman trainer dialog'),
 (13,'polearm trainer dialog'),
 (14,'dancer trainer dialog'),
 (15,'musician trainer dialog'),
 (16,'doctor trainer dialog'),
 (17,'ranger trainer dialog'),
 (18,'creature handler dialog'),
 (19,'bioengineer trainer dialog'),
 (20,'armorsmith trainer dialog'),
 (21,'weaponsmith trainer dialog'),
 (22,'chef trainer dialog'),
 (23,'tailor trainer dialog'),
 (24,'architect trainer dialog'),
 (25,'droidengineer trainer dialog'),
 (26,'merchant trainer dialog'),
 (27,'smuggler trainer dialog'),
 (28,'bountyhunter trainer dialog'),
 (29,'commando trainer dialog'),
 (30,'combat medic trainer dialog'),
 (31,'image designer trainer dialog'),
 (32,'squad leader trainer dialog'),
 (33,'politician trainer dialog'),
 (34,'tutorial item room imperial officer dialog'),
 (35,'tutorial bank and bazzar room imperial officer dialog'),
 (36,'tutorial cloning and insurance room droid dialog'), 
 (37,'tutorial covard imperial officer room dialog'),
 (38,'tutorial imperial officer with trainer room dialog'),  
 (39,'tutorial imperial officer in mission terminal room dialog'),
 (40,'tutorial quartermaster room dialog');
/*!40000 ALTER TABLE `conversations` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;