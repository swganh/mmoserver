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
-- Definition of table `datapad_types`
--

DROP TABLE IF EXISTS `datapad_types`;
CREATE TABLE `datapad_types` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `object_string` char(255) NOT NULL default 'object/tangible/datapad/shared_character_datapad.iff',
  `name` char(255) NOT NULL default 'datapad',
  `file` char(255) NOT NULL default 'item_n',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `pk_datapadtypes_objstr` (`object_string`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 ROW_FORMAT=REDUNDANT;

--
-- Dumping data for table `datapad_types`
--

/*!40000 ALTER TABLE `datapad_types` DISABLE KEYS */;
INSERT INTO `datapad_types` (`id`,`object_string`,`name`,`file`) VALUES 
 (1,'object/tangible/datapad/shared_character_datapad.iff','datapad','item_n'),
 (2,'object/tangible/datapad/shared_droid_datapad_1.iff','datapad','item_n'),
 (3,'object/tangible/datapad/shared_droid_datapad_2.iff','datapad','item_n'),
 (4,'object/tangible/datapad/shared_droid_datapad_3.iff','datapad','item_n'),
 (5,'object/tangible/datapad/shared_droid_datapad_4.iff','datapad','item_n'),
 (6,'object/tangible/datapad/shared_droid_datapad_5.iff','datapad','item_n'),
 (7,'object/tangible/datapad/shared_droid_datapad_6.iff','datapad','item_n');
/*!40000 ALTER TABLE `datapad_types` ENABLE KEYS */;



/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;