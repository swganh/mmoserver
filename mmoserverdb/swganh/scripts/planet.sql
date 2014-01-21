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
-- Definition of table `planet`
--

DROP TABLE IF EXISTS `planet`;
CREATE TABLE `planet` (
  `planet_id` int(2) NOT NULL COMMENT 'Planet ID',
  `name` char(32) NOT NULL COMMENT 'Planet Name',
  `terrain_file` char(255) NOT NULL default 'terrain/' COMMENT 'Planet Terrain File',
  PRIMARY KEY  (`planet_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `planet`
--

/*!40000 ALTER TABLE `planet` DISABLE KEYS */;
INSERT INTO `planet` (`planet_id`,`name`,`terrain_file`) VALUES 
 (0,'corellia','terrain/corellia.trn'),
 (1,'dantooine','terrain/dantooine.trn'),
 (2,'dathomir','terrain/dathomir.trn'),
 (3,'endor','terrain/endor.trn'),
 (4,'lok','terrain/lok.trn'),
 (5,'naboo','terrain/naboo.trn'),
 (6,'rori','terrain/rori.trn'),
 (7,'talus','terrain/talus.trn'),
 (8,'tatooine','terrain/tatooine.trn'),
 (9,'yavin4','terrain/yavin4.trn'),
 (10,'space_corellia','terrain/space_corellia.trn'),
 (11,'space_corellia_2','terrain/space_corellia_2.trn'),
 (12,'space_dantooine','terrain/space_dantooine.trn'),
 (13,'space_dathomir','terrain/space_dathomir.trn'),
 (14,'space_endor','terrain/space_endor.trn'),
 (15,'space_env','terrain/space_env.trn'),
 (16,'space_halos','terrain/space_halos.trn'),
 (17,'space_heavy1','terrain/space_heavy1.trn'),
 (18,'space_light1','terrain/space_light1.trn'),
 (19,'space_lok','terrain/space_lok.trn'),
 (20,'space_naboo','terrain/space_naboo.trn'),
 (21,'space_naboo_2','terrain/space_naboo_2.trn'),
 (22,'space_tatooine','terrain/space_tatooine.trn'),
 (23,'space_tatooine_2','terrain/space_tatooine_2.trn'),
 (24,'space_yavin4','terrain/space_yavin4.trn'),
 (25,'09','terrain/09.trn'),
 (26,'10','terrain/10.trn'),
 (27,'11','terrain/11.trn'),
 (28,'character_farm','terrain/character_farm.trn'),
 (29,'cinco_city_test_m5','terrain/cinco_city_test_m5.trn'),
 (30,'creature_test','terrain/creature_test.trn'),
 (31,'dungeon1','terrain/dungeon1.trn'),
 (32,'endor_asommers','terrain/endor_asommers.trn'),
 (33,'floratest','terrain/floratest.trn'),
 (34,'godclient_test','terrain/godclient_test.trn'),
 (35,'otoh_gunga','terrain/otoh_gunga.trn'),
 (36,'rivertest','terrain/rivertest.trn'),
 (37,'runtimerules','terrain/runtimerules.trn'),
 (38,'simple','terrain/simple.trn'),
 (39,'space_09','terrain/space_09.trn'),
 (40,'test_wearables','terrain/test_wearables.trn'),
 (41,'tutorial','terrain/tutorial.trn'),
 (42,'taanab','terrain/taanab.trn'),
 (43,'dagobah','terrain/dagobah.trn');
/*!40000 ALTER TABLE `planet` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;