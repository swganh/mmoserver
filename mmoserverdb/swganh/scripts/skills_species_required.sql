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
-- Definition of table `skills_species_required`
--

DROP TABLE IF EXISTS `skills_species_required`;
CREATE TABLE `skills_species_required` (
  `skill_id` int(11) unsigned NOT NULL,
  `species_id` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`skill_id`,`species_id`),
  KEY `fk_skills_speciesreq_spec_spec` (`species_id`),
  CONSTRAINT `fk_skills_speciesreq_skill_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_skills_speciesreq_spec_spec` FOREIGN KEY (`species_id`) REFERENCES `race` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `skills_species_required`
--

/*!40000 ALTER TABLE `skills_species_required` DISABLE KEYS */;
INSERT INTO `skills_species_required` (`skill_id`,`species_id`) VALUES 
 (641,0),
 (642,0),
 (643,0),
 (644,0),
 (677,0),
 (641,1),
 (642,1),
 (643,1),
 (645,1),
 (646,1),
 (647,1),
 (679,1),
 (641,2),
 (642,2),
 (643,2),
 (648,2),
 (649,2),
 (650,2),
 (680,2),
 (641,3),
 (642,3),
 (643,3),
 (651,3),
 (652,3),
 (653,3),
 (678,3),
 (641,4),
 (642,4),
 (643,4),
 (654,4),
 (655,4),
 (656,4),
 (682,4),
 (641,5),
 (642,5),
 (643,5),
 (657,5),
 (658,5),
 (659,5),
 (676,5),
 (641,6),
 (642,6),
 (643,6),
 (660,6),
 (661,6),
 (662,6),
 (681,6),
 (641,7),
 (642,7),
 (643,7),
 (663,7),
 (664,7),
 (665,7),
 (683,7),
 (641,33),
 (642,33),
 (643,33),
 (669,33),
 (670,33),
 (671,33),
 (684,33),
 (641,49),
 (642,49),
 (643,49),
 (672,49),
 (673,49),
 (674,49),
 (685,49);
/*!40000 ALTER TABLE `skills_species_required` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;