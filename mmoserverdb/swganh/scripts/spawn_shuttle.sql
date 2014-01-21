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
-- Definition of table `spawn_shuttle`
--

DROP TABLE IF EXISTS `spawn_shuttle`;
CREATE TABLE `spawn_shuttle` (
  `id` int(11) unsigned NOT NULL,
  `oX` float NOT NULL default '0',
  `oY` float NOT NULL default '0',
  `oZ` float NOT NULL default '0',
  `oW` float NOT NULL default '0',
  `X` float NOT NULL default '0',
  `Y` float NOT NULL default '0',
  `Z` float NOT NULL default '0',
  `planet_id` int(2) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `spawn_shuttle_planetid` (`planet_id`),
  CONSTRAINT `spawn_shuttle_ibfk_1` FOREIGN KEY (`planet_id`) REFERENCES `planet` (`planet_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spawn_shuttle`
--

/*!40000 ALTER TABLE `spawn_shuttle` DISABLE KEYS */;
INSERT INTO `spawn_shuttle` (`id`,`oX`,`oY`,`oZ`,`oW`,`X`,`Y`,`Z`,`planet_id`) VALUES 
 (1,0,0.00100429,0,0.946813,3598.1,5,-4776,8),
 (2,0,0.740416,0,0.999957,3419.9,5,-4647.1,8),
 (3,0,0.634851,0,0.376626,-1358.8,12,-3600.5,8),
 (4,0,-0.681038,0,-0.174716,-1094.4,12.3,-3561.8,8),
 (5,0,-0.997989,0,0.699624,-2834.7,5,2110.9,8),
 (6,0,-0.191618,0,0.883836,-2893.6,5,1936.5,8),
 (7,0,-0.632075,0,-0.604032,-3113.8,5,2179.5,8),
 (8,0,-0.911067,0,0.997526,-2804.9,5,2185.3,8),
 (9,0,0.84383,0,0.971628,43.1,52,-5341.2,8),
 (10,0,-0.769691,0,0.883836,1267.3,7,3067.7,8),
 (11,0,-0.441517,0,-0.665485,1392.9,7,3467.7,8),
 (12,0,0.942352,0,0.94829,1727,7,3181.7,8),
 (13,0,-0.995538,0,0.999957,-4862.2,6,4163,5),
 (14,0,-0.725013,0,-0.546518,-5002.8,6,4075.7,5),
 (15,0,-0.96716,0,0.997191,-5410,6,4325.3,5),
 (16,0,-0.891295,0,-0.0256717,-5855.9,6,4170.1,5),
 (17,0,-0.958565,0,0.951183,5277.5,-192,6686.7,5),
 (18,0,-0.961174,0,0.888121,5121.2,-192,6614.9,5),
 (19,0,-0.994915,0,0.798509,1371.1,13,2744.5,5),
 (20,0,-0.260402,0,0.98585,1564.4,25,2838.6,5),
 (21,0,0.0364384,0,0.297371,2024.6,19.1,2526.6,5),
 (22,0,0.419494,0,1,5329.8,326.9,-1573.4,5),
 (23,0,-0.994939,0,0.754219,-5491.6,-150,-21,5),
 (24,0,-0.978501,0,0.0538918,4730.8,4.2,-4674.7,5),
 (25,0,-0.205444,0,0.809499,4963.1,3.8,-4897,5),
 (26,0,0.786126,0,0.888121,620.4,6,3090.1,2),
 (27,0,-0.598487,0,-0.402636,-46.8,18,-1586.9,2),
 (28,0,0.34379,0,-0.870273,-6924.9,73,-5727.7,9),
 (29,0,0.274962,0,0.846314,-262.6,35,4897.9,9),
 (30,0,0.987635,0,0.754219,4051.6,37,-6214.7,9),
 (31,0,-0.880705,0,-0.908621,482.8,8.9,5512.4,4),
 (32,0,0.923728,0,0.577519,-5376.6,80,-2185.7,6),
 (33,0,-0.459846,0,-0.256917,-5258,80.7,-2164.9,6),
 (34,0,0.716324,0,-0.904722,5340,80,5726,6),
 (35,0,0.716324,0,1,5215.6,80,5792.2,6),
 (36,0,-0.501677,0,-0.0958773,3695.8,96,-6400.9,6),
 (37,0,-0.967355,0,0.988079,3201,24,-3499,3),
 (38,0,-0.682172,0,-0.183808,-950.6,73,1548.8,3),
 (39,0,-0.999997,0,0.951183,1573.1,4,-6414,1),
 (40,0,0.993095,0,-0.998908,-4211.5,3,-2350,1),
 (41,0,-0.999865,0,0.923634,-638.1,3,2506.2,1),
 (42,0,-0.674044,0,-0.247974,-6789.4,46,5574.7,1),
 (43,0,-0.977276,0,-0.830934,4453.4,2,5351,7),
 (44,0,-0.893845,0,0.97271,4332.5,9.9,5433.1,7),
 (45,0,-0.715777,0,0.372341,267.5,6,-2950.3,7),
 (46,0,-0.108406,0,-0.100476,698.6,6,-3037.9,7),
 (47,0,0.342138,0,-0.546518,-2226,20,2323,7),
 (48,0,0.906347,0,0.0585057,-69.9,28,-4711.3,0),
 (49,0,0.912766,0,0.996834,-27.9,28,-4406.3,0),
 (50,0,0.994138,0,0.506804,-331.2,28,-4639.2,0),
 (51,0,-0.997379,0,-0.992032,-5001,21,-2229.2,0),
 (52,0,-0.895223,0,-0.935729,-5600.9,21,-2788.3,0),
 (53,0,-0.967613,0,0.692991,-5007.5,21,-2391.2,0),
 (54,0,0.961315,0,0.846314,-5553.8,15.2,-6062.3,0),
 (55,0,0.177776,0,0.920052,6645.3,330,-5918.4,0),
 (56,0,-0.370171,0,0.748118,6932.1,330,-5530.7,0),
 (57,0,0.560854,0,0.921853,-3156,31,2873.9,0),
 (58,0,-0.0651836,0,-0.105073,-3772.7,86,3235.8,0),
 (59,0,-0.436682,0,0.970524,3354.5,308,5599.2,0),
 (60,0,0.986357,0,0.999989,3081.2,280.3,4991,0),
 (61,0,-0.247448,0,-0.672355,-6520.3,398,6042.2,0),
 (62,0,0,0,1,320,45,3764,42),
 (63,0,0,0,1,119.2,45,3917.9,42),
 (64,0,0,0,1,3151,40,-3817,42);
/*!40000 ALTER TABLE `spawn_shuttle` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;