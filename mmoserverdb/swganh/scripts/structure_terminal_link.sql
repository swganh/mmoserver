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
-- Definition of table `structure_terminal_link`
--

DROP TABLE IF EXISTS `structure_terminal_link`;
CREATE TABLE `structure_terminal_link` (
  `ID` int(10) unsigned NOT NULL auto_increment,
  `terminal_type` int(10) unsigned NOT NULL,
  `structure_type` int(10) unsigned NOT NULL,
  `cellId` int(10) unsigned NOT NULL,
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `qx` float NOT NULL,
  `qy` float NOT NULL,
  `qz` float NOT NULL,
  `qw` float NOT NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=64 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `structure_terminal_link`
--

/*!40000 ALTER TABLE `structure_terminal_link` DISABLE KEYS */;
INSERT INTO `structure_terminal_link` (`ID`,`terminal_type`,`structure_type`,`cellId`,`x`,`y`,`z`,`qx`,`qy`,`qz`,`qw`) VALUES 
 (1,29,83,1,0.7,0.3,6.1,0,1,0,0),
 (2,29,86,0,3,0.4,6,0,1,0,0),
 (3,29,87,0,3,0.4,6,0,1,0,0),
 (4,29,63,0,0,0.5,6,0,1,0,0),
 (5,29,64,7,3.5,4.2,-4.4,0,0,0,1),
 (6,29,65,1,-6.5,4.3,-3.9,0,1,0,0),
 (7,29,66,0,5.3,0.2,6.4,0,1,0,0),
 (8,29,67,0,5.3,0.5,6.4,0,1,0,0),
 (9,29,68,0,-5,0.3,-2.7,0,0.71,0,-0.71),
 (10,29,69,0,-5,0.3,-2.7,0,0.71,0,-0.71),
 (11,29,70,0,-6.7,0.2,3.7,0,0.71,0,0.71),
 (12,29,71,0,-6.7,0.2,3.7,0,0.71,0,0.71),
 (13,29,72,7,3.5,4.2,-4.4,0,0,0,1),
 (14,29,73,1,-6.5,4.3,-3.9,0,1,0,0),
 (15,29,74,0,5.3,0.2,6.4,0,1,0,0),
 (16,29,75,0,5.3,0.5,6.0,0,1,0,0),
 (17,29,76,0,-5,0.3,-2.7,0,0.71,0,-0.71),
 (18,29,77,0,-5,0.3,-2.7,0,0.71,0,-0.71),
 (19,29,78,0,-6.7,0.2,3.7,0,0.71,0,0.71),
 (20,29,79,0,-6.7,0.2,3.7,0,0.71,0,0.71),
 (21,29,80,0,-0.1,0.3,-2.4,0,0,0,1),
 (22,29,81,1,14.8,4.3,-1.8,0,0.71,0,-0.71),
 (23,29,82,0,10,0.7,-10.5,0,0,0,1),
 (24,29,84,1,8,0.2,12,0,1,0,0),
 (25,29,85,1,6.7,0.5,8,0,1,0,0),
 (26,29,10,1,-5.1,0.5,5.5,0,0.98,0,0.15),
 (27,29,11,1,-5.1,0.5,5.5,0,0.98,0,0.15),
 (28,29,12,1,-5.1,0.5,5.5,0,0.98,0,0.15),
 (29,29,13,3,-17.1,2.5,8,0,1,0,0),
 (30,29,14,3,-17.1,2.5,8,0,1,0,0),
 (31,29,15,3,-17.1,2.5,8,0,1,0,0),
 (32,36,13,2,0.2,2,-9.7,0,0,0,1),
 (33,36,14,2,0.2,2,-9.7,0,0,0,1),
 (34,36,15,2,0.2,2,-9.7,0,0,0,1),
 (35,34,13,4,17.2,2,-9,0,0,0,1),
 (36,34,14,4,17.2,2,-9,0,0,0,1),
 (37,34,15,4,17.2,2,-9,0,0,0,1),
 (38,17,16,3,6.5,0.7,-6.6,0,0.71,0,-0.71),
 (39,17,17,3,6.5,0.7,-6.6,0,0.71,0,-0.71),
 (40,17,18,3,6.5,0.7,-6.6,0,0.71,0,-0.71),
 (41,18,16,3,-13.1,-2.5,3,0,0.71,0,0.71),
 (42,18,17,3,-13.1,-2.5,3,0,0.71,0,0.71),
 (43,18,18,3,-13.1,-2.5,3,0,0.71,0,0.71),
 (44,29,16,3,-13.5,-0.6,-6.3,0,0.71,0,0.71),
 (45,29,17,3,-13.5,-0.6,-6.3,0,0.71,0,0.71),
 (46,29,18,3,-13.5,-0.6,-6.3,0,0.71,0,0.71),
 (47,29,28,2,7.8,0.6,1.5,0,0.71,0,-0.71),
 (48,29,29,2,7.8,0.6,1.5,0,0.71,0,-0.71),
 (49,29,30,2,7.8,0.6,1.5,0,0.71,0,-0.71),
 (50,32,28,2,7.8,0.2,-0.5,0,0.71,0,-0.71),
 (51,32,29,2,7.8,0.2,-0.5,0,0.71,0,-0.71),
 (52,32,30,2,7.8,0.2,-0.5,0,0.71,0,-0.71),
 (53,29,34,0,-7,5.2,-14.2,0,0.71,0,0.71),
 (54,29,35,0,-7,5.2,-14.2,0,0.71,0,0.71),
 (55,29,36,0,-7,5.2,-14.2,0,0.71,0,0.71),
 (56,29,44,6,17.9,3.3,10.4,0,0.71,0,0.71),
 (57,29,45,6,17.9,3.3,10.4,0,0.71,0,0.71),
 (58,29,46,6,17.9,3.3,10.4,0,0.71,0,0.71),
 (59,29,88,6,17.9,2.8,10.4,0,0.71,0,0.71),
 (60,21,44,5,-16,2.8,19.7,0,1,0,0),
 (61,21,45,5,-16,2.8,19.7,0,1,0,0),
 (62,21,46,5,-16,2.8,19.7,0,1,0,0),
 (63,21,88,5,-16,2.8,19.7,0,1,0,0),
 (64,29,47,1,9.0,2.0,6.3,0,0.71,0,0.71),
 (65,21,47,5,-16,2.8,19.7,0,1,0,0);
/*!40000 ALTER TABLE `structure_terminal_link` ENABLE KEYS */;




/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
