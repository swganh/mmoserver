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
-- Definition of table `resources_spawn_config`
--

DROP TABLE IF EXISTS `resources_spawn_config`;
CREATE TABLE `resources_spawn_config` (
  `resource_id` bigint(20) unsigned NOT NULL default '0',
  `planet_id` int(2) NOT NULL default '0',
  `noiseMapBoundsX1` double NOT NULL default '1',
  `noiseMapBoundsX2` double NOT NULL default '3',
  `noiseMapBoundsY1` double NOT NULL default '1',
  `noiseMapBoundsY2` double NOT NULL default '4',
  `noiseMapOctaves` tinyint(3) unsigned NOT NULL default '4',
  `noiseMapFrequency` double NOT NULL default '4',
  `noiseMapPersistence` double NOT NULL default '0.2',
  `noiseMapScale` double NOT NULL default '1',
  `noiseMapBias` double NOT NULL default '0',
  `unitsTotal` bigint(20) unsigned NOT NULL default '10000000',
  `unitsLeft` bigint(20) unsigned default '10000000',
  PRIMARY KEY  (`resource_id`,`planet_id`),
  KEY `fk_resourcespawnconfig_planet_planet` (`planet_id`),
  CONSTRAINT `fk_resourcespawnconfig_planet_planet` FOREIGN KEY (`planet_id`) REFERENCES `planet` (`planet_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_resourcespawnconfig_resource_resource` FOREIGN KEY (`resource_id`) REFERENCES `resources` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `resources_spawn_config`
--

/*!40000 ALTER TABLE `resources_spawn_config` DISABLE KEYS */;
/*!40000 ALTER TABLE `resources_spawn_config` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;