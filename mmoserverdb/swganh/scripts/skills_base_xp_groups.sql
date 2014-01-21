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
-- Definition of table `skills_base_xp_groups`
--

DROP TABLE IF EXISTS `skills_base_xp_groups`;
CREATE TABLE `skills_base_xp_groups` (
  `skill_id` int(11) unsigned NOT NULL,
  `xp_type_id` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`skill_id`,`xp_type_id`),
  KEY `fk_skills_xpgroups_xp_xp` (`xp_type_id`),
  CONSTRAINT `fk_skills_xpgroups_skill_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_skills_xpgroups_xp_xp` FOREIGN KEY (`xp_type_id`) REFERENCES `xp_types` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `skills_base_xp_groups`
--

/*!40000 ALTER TABLE `skills_base_xp_groups` DISABLE KEYS */;
INSERT INTO `skills_base_xp_groups` (`skill_id`,`xp_type_id`) VALUES 
 (357,2),
 (357,3),
 (528,4),
 (31,5),
 (319,5),
 (110,6),
 (129,6),
 (148,6),
 (167,6),
 (186,6),
 (205,6),
 (224,6),
 (243,6),
 (547,6),
 (688,7),
 (707,8),
 (726,9),
 (764,9),
 (688,10),
 (707,10),
 (726,10),
 (764,10),
 (861,10),
 (899,10),
 (918,10),
 (937,10),
 (91,11),
 (205,11),
 (91,12),
 (243,12),
 (91,13),
 (224,13),
 (91,14),
 (186,14),
 (110,15),
 (167,15),
 (528,15),
 (110,16),
 (148,16),
 (509,16),
 (528,16),
 (110,17),
 (129,17),
 (376,18),
 (433,19),
 (471,20),
 (414,21),
 (71,22),
 (51,23),
 (300,23),
 (566,23),
 (509,24),
 (452,25),
 (395,26),
 (338,27),
 (11,28),
 (262,28),
 (11,29),
 (262,29),
 (281,29),
 (11,30),
 (585,30),
 (51,32),
 (300,32),
 (566,32),
 (490,33),
 (11,34),
 (281,34),
 (71,35),
 (31,36),
 (319,36),
 (509,37),
 (604,38),
 (31,39),
 (319,39),
 (528,40),
 (547,40),
 (623,41),
 (957,42),
 (970,42),
 (784,43),
 (822,44),
 (841,45),
 (803,46),
 (992,47),
 (1012,48),
 (1031,48),
 (1050,48);
/*!40000 ALTER TABLE `skills_base_xp_groups` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;