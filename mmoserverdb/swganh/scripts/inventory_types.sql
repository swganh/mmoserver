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
-- Definition of table `inventory_types`
--

DROP TABLE IF EXISTS `inventory_types`;
CREATE TABLE `inventory_types` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `object_string` char(255) NOT NULL default 'object/tangible/inventory/shared_character_inventory.iff',
  `slots` int(11) unsigned NOT NULL default '50',
  `name` char(255) NOT NULL default 'inventory',
  `file` char(255) NOT NULL default 'item_n',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `inventory_types`
--

/*!40000 ALTER TABLE `inventory_types` DISABLE KEYS */;
INSERT INTO `inventory_types` (`id`,`object_string`,`slots`,`name`,`file`) VALUES 
 (1,'object/tangible/inventory/shared_character_inventory.iff',80,'inventory','item_n'),
 (2,'object/tangible/inventory/shared_creature_inventory.iff',50,'inventory','item_n'),
 (3,'object/tangible/inventory/shared_creature_inventory_1.iff',50,'inventory','item_n'),
 (4,'object/tangible/inventory/shared_creature_inventory_2.iff',50,'inventory','item_n'),
 (5,'object/tangible/inventory/shared_creature_inventory_3.iff',50,'inventory','item_n'),
 (6,'object/tangible/inventory/shared_creature_inventory_4.iff',50,'inventory','item_n'),
 (7,'object/tangible/inventory/shared_creature_inventory_5.iff',50,'inventory','item_n'),
 (8,'object/tangible/inventory/shared_creature_inventory_6.iff',50,'inventory','item_n'),
 (9,'object/tangible/inventory/shared_vendor_inventory.iff',50,'inventory','item_n'),
 (10,'object/tangible/inventory/shared_lightsaber_inventory_training.iff',50,'inventory','item_n'),
 (11,'object/tangible/inventory/shared_lightsaber_inventory_1.iff',50,'inventory','item_n'),
 (12,'object/tangible/inventory/shared_lightsaber_inventory_2.iff',50,'inventory','item_n'),
 (13,'object/tangible/inventory/shared_lightsaber_inventory_3.iff',50,'inventory','item_n'),
 (14,'object/tangible/inventory/shared_lightsaber_inventory_4.iff',50,'inventory','item_n');
/*!40000 ALTER TABLE `inventory_types` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;