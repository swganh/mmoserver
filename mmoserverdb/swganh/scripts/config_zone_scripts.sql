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
-- Definition of table `config_zone_scripts`
--

DROP TABLE IF EXISTS `config_zone_scripts`;
CREATE TABLE `config_zone_scripts` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `planet_id` int(2) NOT NULL default '0',
  `file` char(255) NOT NULL default 'script/',
  `priority` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `FK_config_zone_scripts_planet` (`planet_id`),
  CONSTRAINT `FK_config_zone_scripts_planet` FOREIGN KEY (`planet_id`) REFERENCES `planet` (`planet_id`)
) ENGINE=InnoDB AUTO_INCREMENT=23 DEFAULT CHARSET=utf8 ROW_FORMAT=REDUNDANT;

--
-- Dumping data for table `config_zone_scripts`
--

/*!40000 ALTER TABLE `config_zone_scripts` DISABLE KEYS */;
INSERT INTO `config_zone_scripts` (`id`,`planet_id`,`file`,`priority`) VALUES 
 (1,0,'script/WeatherController.lua',0),
 (2,1,'script/WeatherController.lua',0),
 (3,2,'script/WeatherController.lua',0),
 (4,3,'script/WeatherController.lua',0),
 (5,4,'script/WeatherController.lua',0),
 (6,5,'script/WeatherController.lua',0),
 (7,6,'script/WeatherController.lua',0),
 (8,7,'script/WeatherController.lua',0),
 (9,8,'script/WeatherController.lua',0),
 (10,9,'script/WeatherController.lua',0),
 (11,42,'script/WeatherController.lua',0),
 (12,8,'script/PlayerLoginReporter.lua',0),
 (13,0,'script/PlayerLoginReporter.lua',0),
 (14,1,'script/PlayerLoginReporter.lua',0),
 (15,2,'script/PlayerLoginReporter.lua',0),
 (16,3,'script/PlayerLoginReporter.lua',0),
 (17,4,'script/PlayerLoginReporter.lua',0),
 (18,5,'script/PlayerLoginReporter.lua',0),
 (19,6,'script/PlayerLoginReporter.lua',0),
 (20,7,'script/PlayerLoginReporter.lua',0),
 (21,9,'script/PlayerLoginReporter.lua',0),
 (22,42,'script/PlayerLoginReporter.lua',0),
 (23,8,'script/TatooineNpcTest.lua',0),
 (24,8,'script/TatooineNpcTest2.lua',0),
 (25,8,'script/TatooineNpcTest3.lua',0),
 (26,8,'script/TatooineNpcTest4.lua',0),
 (27,0,'script/CorelliaInstanceTest.lua',0),
 (28,8,'script/TatooineNpcTest5.lua',0),
 (29,8,'script/TatooineNpcTest6.lua',0),
 (30,8,'script/TatooineNpcTest7.lua',0);
/*!40000 ALTER TABLE `config_zone_scripts` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;