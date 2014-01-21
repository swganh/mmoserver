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
-- Definition of table `lairs`
--

DROP TABLE IF EXISTS `lairs`;
CREATE TABLE `lairs` (
  `id` bigint(20) NOT NULL,
  `creature_spawn_region` bigint(20) NOT NULL,
  `family` int(11) unsigned NOT NULL default '9',
  `faction` int(8) unsigned NOT NULL default '0',
  `lair_template` bigint(20) default NULL,
  `creature_group` int(11) default NULL,
  `count` int(11) default 0,
  `spawn_x` float default NULL,
  `spawn_y` float default NULL,
  `spawn_z` float default NULL,
  `spawn_dir_Y` float NOT NULL default '1.0',
  `spawn_dir_W` float NOT NULL default '0.0',
  PRIMARY KEY  (`id`,`creature_spawn_region`),
  KEY `fk_creature_spawn_region` (`creature_spawn_region`),
  KEY `fk_lair_template` (`lair_template`),
  KEY `fk_lair_creature_group` (`creature_group`),
  CONSTRAINT `fk_lair_creature_group` FOREIGN KEY (`creature_group`) REFERENCES `creature_groups` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_lair_template` FOREIGN KEY (`lair_template`) REFERENCES `lair_templates` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_creature_spawn_region` FOREIGN KEY (`creature_spawn_region`) REFERENCES `spawns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `lairs`
--

/*!40000 ALTER TABLE `lairs` DISABLE KEYS */;
INSERT INTO `lairs` (`id`,`creature_spawn_region`,`family`,`faction`,`lair_template`,`creature_group`, `count`, `spawn_x`, `spawn_y`, `spawn_z`, `spawn_dir_Y`, `spawn_dir_W`) VALUES
 (1,1,9,1,1,1,0,-1631,33,-4031,1.0,0.0),	-- Womps
 (2,1,9,1,2,2,0,-1377,60,-4235,0.0,1.0), -- Rills
 (3,1,9,1,1,1,25,0.0,0.0,0.0,1.0,0.0),	-- Womps
 (4,1,9,1,2,2,25,-1500,0,-3950,0.0,1.0); -- Rills
/*!40000 ALTER TABLE `lairs` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
