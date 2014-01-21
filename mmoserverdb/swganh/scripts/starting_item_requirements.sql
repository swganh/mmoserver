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
-- Definition of table `starting_item_requirements`
--

DROP TABLE IF EXISTS `starting_item_requirements`;
CREATE TABLE `starting_item_requirements` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `species` int(8) unsigned NOT NULL default '0',
  `gender` tinyint(3) unsigned NOT NULL default '0',
  `profession` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `fk_start_items_req_species` (`species`),
  KEY `fk_start_items_req_profession` (`profession`),
  CONSTRAINT `fk_start_items_req_profession` FOREIGN KEY (`profession`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_start_items_req_species` FOREIGN KEY (`species`) REFERENCES `race` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=REDUNDANT;

--
-- Dumping data for table `starting_item_requirements`
--

/*!40000 ALTER TABLE `starting_item_requirements` DISABLE KEYS */;
INSERT INTO `starting_item_requirements` (`id`,`species`,`gender`,`profession`) VALUES 
 (1,0,0,10),
 (2,1,0,10),
 (3,2,0,10),
 (4,3,0,10),
 (5,4,0,10),
 (6,5,0,10),
 (7,6,0,10),
 (8,7,0,10),
 (9,33,0,10),
 (10,49,0,10),
 (11,0,1,10),
 (12,1,1,10),
 (13,2,1,10),
 (14,3,1,10),
 (15,4,1,10),
 (16,5,1,10),
 (17,6,1,10),
 (18,7,1,10),
 (19,33,1,10),
 (20,49,1,10),
 (21,0,0,30),
 (22,1,0,30),
 (23,2,0,30),
 (24,3,0,30),
 (25,4,0,30),
 (26,5,0,30),
 (27,6,0,30),
 (28,7,0,30),
 (29,33,0,30),
 (30,49,0,30),
 (31,0,1,30),
 (32,1,1,30),
 (33,2,1,30),
 (34,3,1,30),
 (35,4,1,30),
 (36,5,1,30),
 (37,6,1,30),
 (38,7,1,30),
 (39,33,1,30),
 (40,49,1,30),
 (41,0,0,50),
 (42,1,0,50),
 (43,2,0,50),
 (44,3,0,50),
 (45,4,0,50),
 (46,5,0,50),
 (47,6,0,50),
 (48,7,0,50),
 (49,33,0,50),
 (50,49,0,50),
 (51,0,1,50),
 (52,1,1,50),
 (53,2,1,50),
 (54,3,1,50),
 (55,4,1,50),
 (56,5,1,50),
 (57,6,1,50),
 (58,7,1,50),
 (59,33,1,50),
 (60,49,1,50),
 (61,0,0,70),
 (62,1,0,70),
 (63,2,0,70),
 (64,3,0,70),
 (65,4,0,70),
 (66,5,0,70),
 (67,6,0,70),
 (68,7,0,70),
 (69,33,0,70),
 (70,49,0,70),
 (71,0,1,70),
 (72,1,1,70),
 (73,2,1,70),
 (74,3,1,70),
 (75,4,1,70),
 (76,5,1,70),
 (77,6,1,70),
 (78,7,1,70),
 (79,33,1,70),
 (80,49,1,70),
 (81,0,0,90),
 (82,1,0,90),
 (83,2,0,90),
 (84,3,0,90),
 (85,4,0,90),
 (86,5,0,90),
 (87,6,0,90),
 (88,7,0,90),
 (89,33,0,90),
 (90,49,0,90),
 (91,0,1,90),
 (92,1,1,90),
 (93,2,1,90),
 (94,3,1,90),
 (95,4,1,90),
 (96,5,1,90),
 (97,6,1,90),
 (98,7,1,90),
 (99,33,1,90),
 (100,49,1,90),
 (101,0,0,109),
 (102,1,0,109),
 (103,2,0,109),
 (104,3,0,109),
 (105,4,0,109),
 (106,5,0,109),
 (107,6,0,109),
 (108,7,0,109),
 (109,33,0,109),
 (110,49,0,109),
 (111,0,1,109),
 (112,1,1,109),
 (113,2,1,109),
 (114,3,1,109),
 (115,4,1,109),
 (116,5,1,109),
 (117,6,1,109),
 (118,7,1,109),
 (119,33,1,109),
 (120,49,1,109),
 (121, 0, 0, 688),
 (122, 1, 0, 688),
 (123, 2, 0, 688),
 (124, 3, 0, 688),
 (125, 4, 0, 688),
 (126, 5, 0, 688),
 (127, 6, 0, 688),
 (128, 7, 0, 688),
 (129, 33, 0, 688),
 (130, 49, 0, 688),
 (131, 0, 1, 688),
 (132, 1, 1, 688),
 (133, 2, 1, 688),
 (134, 3, 1, 688),
 (135, 4, 1, 688),
 (136, 5, 1, 688),
 (137, 6, 1, 688),
 (138, 7, 1, 688),
 (139, 33, 1, 688),
 (140, 49, 1, 688);
/*!40000 ALTER TABLE `starting_item_requirements` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
