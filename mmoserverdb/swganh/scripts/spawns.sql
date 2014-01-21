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
-- Definition of table `spawns`
--

DROP TABLE IF EXISTS `spawns`;
CREATE TABLE `spawns` (
  `id` bigint(20) NOT NULL,
  `spawn_x` float default NULL,
  `spawn_z` float default NULL,
  `spawn_width` float default NULL,
  `spawn_length` float default NULL,
  `spawn_planet` int(2) default NULL,
  `spawn_density` int(11) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spawns`
--

/*!40000 ALTER TABLE `spawns` DISABLE KEYS */;
INSERT INTO `spawns` (`id`,`spawn_x`,`spawn_z`,`spawn_width`,`spawn_length`,`spawn_planet`,`spawn_density`) VALUES 
 (1,-2200,-4700,700,750,8,1),
 (2,-7800,7800,5712,7776,0,1),
 (3,-2112,7800,9904,3776,0,1),
 (4,-1980,3880,3060,5720,0,1),
 (5,1120,3860,2900,5720,0,1),
 (6,4060,3840,3700,9760,0,1),
 (7,-7680,60,5500,5440,0,1),
 (8,-2020,-1940,6020,4020,0,1),
 (9,-380,-6040,8000,1600,0,1),
 (10,-7800,7800,5536,5344,1,1),
 (11,-2256,7800,5040,3616,1,1),
 (12,2832,7800,4826,3616,1,1),
 (13,-2288,4192,3088,2896,1,1),
 (14,832,4192,6880,2896,1,1),
 (15,-7800,2480,2400,6576,1,1),
 (16,-2320,2448,3088,6576,1,1),
 (17,-2256,1216,6224,5328,1,1),
 (18,4016,1200,3680,8784,1,1),
 (19,-7800,-4128,6704,3456,1,1),
 (20,-1136,-4144,5152,3408,1,1),
 (21,-7824,7840,4752,4000,2,1),
 (22,-3040,7840,5200,4016,2,1),
 (23,2208,7840,5520,5040,2,1),
 (24,-7824,3808,6320,4720,2,1),
 (25,-1472,3776,3664,6672,2,1),
 (26,2208,2768,5536,5072,2,1),
 (27,-7824,-3856,6336,2896,2,1),
 (28,-7824,-3904,6368,3792,2,1),
 (29,-1456,-2944,3680,4752,2,1),
 (30,2240,-2304,5504,5408,2,1),
 (31,-8048,8064,3936,6080,3,1),
 (32,-4080,8064,4160,5248,3,1),
 (33,128,8064,3824,3840,3,1),
 (34,3968,8064,3824,7520,3,1),
 (35,-8064,1968,3984,8448,3,1),
 (36,-4080,2784,4160,3904,3,1),
 (37,112,4160,3808,5280,3,1),
 (38,3968,448,3856,5664,3,1),
 (39,-4048,-1184,4016,5312,3,1),
 (40,-16,-1152,3952,4096,3,1),
 (41,2688,-5264,5168,2576,3,1),
 (42,-8016,7984,5776,4128,4,1),
 (43,-2176,7952,4592,4144,4,1),
 (44,2464,7936,5280,4160,4,1),
 (45,-8016,3792,3744,5856,4,1),
 (46,-4224,3760,6416,2768,4,1),
 (47,-4224,-2096,6416,3040,4,1),
 (48,2208,3744,5536,5856,4,1),
 (49,-8016,-2112,3776,5552,4,1),
 (50,-4224,-2128,6576,5552,4,1),
 (51,2352,-2128,5408,5584,4,1),
 (52,-7936,7984,8576,6992,5,1),
 (53,672,7968,7120,6992,5,1),
 (54,-7936,960,9696,2368,5,1),
 (55,1776,960,6016,8896,5,1),
 (56,-7936,-1456,3536,6400,5,1),
 (57,-4352,-1472,6096,6480,5,1),
 (58,-7800,7800,4832,3312,6,1),
 (59,-2960,7800,4864,3312,6,1),
 (60,1936,7800,5904,3312,6,1),
 (61,-7800,4416,4832,2128,6,1),
 (62,-2960,4400,3120,8640,6,1),
 (63,192,4400,7648,4160,6,1),
 (64,-7800,2272,4864,228,6,1),
 (65,-7800,-64,4864,4192,6,1),
 (66,192,192,7664,4416,6,1),
 (67,-7800,-4272,7968,3536,6,1),
 (68,192,-4272,7648,3520,6,1),
 (69,-7800,7800,4432,5456,7,1),
 (70,-3344,7800,5504,5456,7,1),
 (71,2176,7800,5632,5456,7,1),
 (72,-7800,2320,5504,5664,7,1),
 (73,-2288,2256,10112,3568,7,1),
 (74,-7800,-3376,5504,3936,7,1),
 (75,-2273,-1328,5600,5984,7,1),
 (76,3344,-1328,4448,6000,7,1),
 (77,-7800,7800,4144,3616,8,1),
 (78,-3632,7800,11440,2176,8,1),
 (79,-7800,4144,4128,11936,8,1),
 (80,-3632,5568,4528,4672,8,1),
 (81,944,5568,6880,4688,8,1),
 (82,-3616,864,11456,8672,8,1),
 (83,4960,4464,2336,1152,8,1),
 (84,-7800,7800,5840,4736,9,1),
 (85,-1952,7800,3968,4736,9,1),
 (86,2049,7800,5696,4752,9,1),
 (87,-7800,3040,3616,4400,9,1),
 (88,-4128,3024,5408,4368,9,1),
 (89,1344,2992,6368,4352,9,1),
 (90,-7808,-1424,5920,5632,9,1),
 (91,-1792,-1456,5776,5616,9,1),
 (92,4048,-1456,3696,5648,9,1);
/*!40000 ALTER TABLE `spawns` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
