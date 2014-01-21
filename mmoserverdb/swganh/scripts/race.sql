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
-- Definition of table `race`
--

DROP TABLE IF EXISTS `race`;
CREATE TABLE `race` (
  `id` int(8) unsigned NOT NULL auto_increment COMMENT 'Character Starting Attributes Table ID',
  `name` char(24) NOT NULL default '' COMMENT 'Race Name',
  `health_min` int(6) unsigned NOT NULL default '0' COMMENT 'Health Minimum',
  `health_max` int(6) unsigned NOT NULL default '0' COMMENT 'Health Maximum',
  `strength_min` int(6) unsigned NOT NULL default '0' COMMENT 'Strangth Minimum',
  `strength_max` int(6) unsigned NOT NULL default '0' COMMENT 'Strength Maximum',
  `constitution_min` int(6) unsigned NOT NULL default '0' COMMENT 'Constitution Minimum',
  `constitution_max` int(6) unsigned NOT NULL default '0' COMMENT 'Constitution Maximum',
  `action_min` int(6) unsigned NOT NULL default '0' COMMENT 'Action Minimum',
  `action_max` int(6) unsigned NOT NULL default '0' COMMENT 'Action Maximum',
  `quickness_min` int(6) unsigned NOT NULL default '0' COMMENT 'Quickness Minimum',
  `quickness_max` int(6) unsigned NOT NULL default '0' COMMENT 'Quickness Maximum',
  `stamina_min` int(6) unsigned NOT NULL default '0' COMMENT 'Stamina Minimum',
  `stamina_max` int(6) unsigned NOT NULL default '0' COMMENT 'Stamina Maximum',
  `mind_min` int(6) unsigned NOT NULL default '0' COMMENT 'Mind Minimum',
  `mind_max` int(6) unsigned NOT NULL default '0' COMMENT 'Mind Maximum',
  `focus_min` int(6) unsigned NOT NULL default '0' COMMENT 'Focus Minimum',
  `focus_max` int(6) unsigned NOT NULL default '0' COMMENT 'Focus Maximum',
  `willpower_min` int(6) unsigned NOT NULL default '0' COMMENT 'Willpower Minimum',
  `willpower_max` int(6) unsigned NOT NULL default '0' COMMENT 'Willpower Maximum',
  `total` int(6) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=50 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `race`
--

/*!40000 ALTER TABLE `race` DISABLE KEYS */;
INSERT INTO `race` (`id`,`name`,`health_min`,`health_max`,`strength_min`,`strength_max`,`constitution_min`,`constitution_max`,`action_min`,`action_max`,`quickness_min`,`quickness_max`,`stamina_min`,`stamina_max`,`mind_min`,`mind_max`,`focus_min`,`focus_max`,`willpower_min`,`willpower_max`,`total`) VALUES 
 (0,'human',400,1100,400,1100,400,1100,400,1100,400,1100,400,1100,400,1100,400,1100,400,1100,5400),
 (1,'rodian',300,1000,300,500,300,400,300,1200,300,650,450,850,300,1000,300,500,350,550,5400),
 (2,'trandoshan',550,1250,600,800,700,800,300,1000,300,450,300,400,300,1000,300,500,300,600,5550),
 (3,'moncal',300,1000,300,500,300,400,300,1000,300,450,450,550,600,1300,600,800,450,650,5400),
 (4,'wookiee',650,1350,650,850,450,550,500,1200,400,550,400,500,400,1100,450,650,400,600,6100),
 (5,'bothan',300,1000,300,500,300,400,600,1300,600,750,400,500,400,1100,400,600,300,500,5400),
 (6,'twilek',300,1000,300,500,550,650,550,1250,600,750,300,400,400,1100,300,500,300,500,5400),
 (7,'zabrak',500,1200,300,500,300,400,600,1300,300,450,300,400,300,1000,300,500,700,900,5400),
 (33,'ithorian',300,1400,300,600,300,500,600,1100,300,750,300,500,400,1300,400,600,300,500,5400),
 (49,'sullustan',300,1200,300,500,300,400,600,1400,300,750,300,500,400,1200,400,600,300,600,5400);
/*!40000 ALTER TABLE `race` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;