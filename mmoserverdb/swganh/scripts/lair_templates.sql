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
-- Definition of table `lair_templates`
--

DROP TABLE IF EXISTS `lair_templates`;
CREATE TABLE `lair_templates` (
  `id` bigint(20) NOT NULL,
  `lair_object_string` char(255) default NULL,
  `stf_name` char(255) default NULL,
  `stf_file` char(255) default NULL,
  `stf_detail_name` char(255) default NULL,
  `stf_detail_file` char(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `lair_templates`
--

/*!40000 ALTER TABLE `lair_templates` DISABLE KEYS */;
INSERT INTO `lair_templates` (`id`,`lair_object_string`,`stf_name`,`stf_file`,`stf_detail_name`,`stf_detail_file`) VALUES
 (1,'object/tangible/lair/base/shared_poi_all_lair_rocks_small.iff','rocks','lair_n','',''),
 (2,'object/tangible/lair/base/shared_poi_all_lair_nest_small.iff','nest','lair_n','',''),
 (3,'/object/tangible/lair/base/shared_poi_all_lair_rock_shelter_small.iff','all_lair_rock_shelter_small','lair_n','all_lair_rock_shelter_small','lair_d'),
 (4,'/object/tangible/lair/base/shared_poi_all_lair_rock_shelter_large.iff','all_lair_rock_shelter_large','lair_n','all_lair_rock_shelter_large','lair_d');
/*!40000 ALTER TABLE `lair_templates` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
