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
-- Definition of table `planetmapcategory`
--

DROP TABLE IF EXISTS `planetmapcategory`;
CREATE TABLE `planetmapcategory` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `main` int(10) unsigned default NULL,
  `sub` int(10) unsigned default NULL,
  `description` tinytext,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=99 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `planetmapcategory`
--

/*!40000 ALTER TABLE `planetmapcategory` DISABLE KEYS */;
INSERT INTO `planetmapcategory` (`id`,`main`,`sub`,`description`) VALUES 
 (1,0,0,'For Details refer to --> http://wiki.swganh.org/index.php/PKT_Map#Category_Breakdown'),
 (2,1,0,'Unknown'),
 (3,2,0,'Bank'),
 (4,3,0,'Cantina'),
 (5,4,0,'Capitol'),
 (6,5,0,'Cloning Facility'),
 (7,6,0,'Parking Garage'),
 (8,7,0,'Guild Hall'),
 (9,7,8,'Guild Hall (Combat)'),
 (10,7,9,'Guild Hall (Commerce)'),
 (11,7,10,'Guild Hall (Theater)'),
 (12,7,11,'Guild Hall (University)'),
 (13,8,0,'NOT USED'),
 (14,9,0,'NOT USED'),
 (15,10,0,'NOT USED'),
 (16,11,0,'NOT USED'),
 (17,12,0,'Hotel'),
 (18,13,0,'Medical Center'),
 (19,14,0,'Shuttle Port'),
 (20,15,0,'Starport'),
 (21,16,0,'Theme Park'),
 (22,17,0,'City'),
 (23,18,0,'NOT USED'),
 (24,19,20,'Trainer - Brawler'),
 (25,19,21,'Trainer - Artisan'),
 (26,19,22,'Trainer - Scout'),
 (27,19,23,'Trainer - Marksman'),
 (28,19,24,'Trainer - Entertainer'),
 (29,19,25,'Trainer - Medic'),
 (30,19,45,'Rebel Recruiter'),
 (31,19,46,'Imperial Recruiter'),
 (32,19,63,'Trainer - Shipwright'),
 (33,19,64,'Trainer - Privateer Pilot (RSF)'),
 (34,19,65,'Trainer - Rebel Pilot'),
 (35,19,66,'Trainer - Imperial Pilot'),
 (36,20,0,'NOT USED'),
 (37,21,0,'NOT USED'),
 (38,22,0,'NOT USED'),
 (39,23,0,'NOT USED'),
 (40,24,0,'NOT USED'),
 (41,25,0,'NOT USED'),
 (42,26,0,'Watto\'s Junk Shop'),
 (43,27,0,'Tavern'),
 (44,28,0,'NOT USED'),
 (45,29,40,'Junk Dealer'),
 (46,30,0,'NOT USED'),
 (47,31,0,'NOT USED'),
 (48,32,0,'NOT USED'),
 (49,33,0,'NOT USED'),
 (50,34,0,'NOT USED'),
 (51,35,0,'NOT USED'),
 (52,36,0,'NOT USED'),
 (53,37,0,'NOT USED'),
 (54,38,0,'NOT USED'),
 (55,39,0,'NOT USED'),
 (56,40,0,'NOT USED'),
 (57,41,21,'Artisan Mission Terminal'),
 (58,41,22,'Scout Mission Terminal'),
 (59,41,24,'Entertainer Mission Terminal'),
 (60,41,42,'Bank Terminal'),
 (61,41,43,'Bazzar Terminal'),
 (62,41,44,'Mission Terminal'),
 (63,41,45,'Rebel Misson Terminal'),
 (64,41,46,'Imperial Mission Terminal'),
 (65,42,0,'NOT USED'),
 (66,43,0,'NOT USED'),
 (67,44,0,'NOT USED'),
 (68,45,0,'NOT USED'),
 (69,46,0,'NOT USED'),
 (70,47,0,'NOT USED'),
 (71,48,0,'Imperial Headquarters'),
 (72,49,0,'NOT USED'),
 (73,50,0,'NOT USED'),
 (74,51,0,'Theater'),
 (75,52,0,'NOT USED'),
 (76,53,0,'NOT USED'),
 (77,54,0,'NOT USED'),
 (78,55,0,'NOT USED'),
 (79,56,0,'Museum'),
 (80,57,0,'Salon'),
 (81,58,59,'Imperial Pilot Coordinator (Commander Landare)'),
 (82,59,0,'NOT USED'),
 (83,60,0,'NOT USED'),
 (84,61,0,'Starship Chassis Broker'),
 (85,29,31,'Vendor - Clothing'),
 (86,29,32,'Vendor - Components'),
 (87,29,33,'Vendor - Droids'),
 (88,29,34,'Vendor - Equipment'),
 (89,29,35,'Vendor - Food'),
 (90,29,36,'Vendor - Housing'),
 (91,29,37,'Vendor - Resources'),
 (92,29,38,'Vendor - Tools'),
 (93,29,39,'Vendor - Weapons'),
 (94,29,30,'Vendor - Junk Dealer'),
 (95,29,52,'Vendor - Pet/PetSupplies'),
 (96,29,53,'Vendor - Medical'),
 (97,29,67,'Vendor - Shipwright'),
 (98,68,0,'Barracks');
/*!40000 ALTER TABLE `planetmapcategory` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;