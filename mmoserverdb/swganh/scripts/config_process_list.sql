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


--
-- Create schema swganh
--

USE swganh;

--
-- Definition of table `config_process_list`
--

DROP TABLE IF EXISTS `config_process_list`;
CREATE TABLE `config_process_list` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `address` char(16) NOT NULL DEFAULT '127.0.0.1',
  `port` int(10) unsigned NOT NULL DEFAULT '0',
  `status` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '0=offline, 1=loading, 2=online, 3=locked',
  `active` int(10) unsigned NOT NULL DEFAULT '0',
  `name` char(32) NOT NULL DEFAULT '',
  `description` char(255) NOT NULL DEFAULT '',
  `serverstartID` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `config_process_list`
--

/*!40000 ALTER TABLE `config_process_list` DISABLE KEYS */;
INSERT INTO `config_process_list` (`id`,`address`,`port`,`status`,`active`,`name`,`description`,`serverstartID`) VALUES 
 (0,'',0,0,0,'login','Handles initial connectivity and authentication of client.',1),
 (1,'',0,0,0,'connection','Handles all client connections.',3),
 (2,'',0,0,0,'unused','Unused server ID.',0),
 (3,'',0,0,0,'unused','Unused server ID.',0),
 (4,'',0,0,0,'unused','Unused server ID.',0),
 (5,'',0,0,0,'unused','Unused server ID.',0),
 (6,'',0,0,1,'chat','Handles chat,tells,mails,tickets',2),
 (7,'',0,0,0,'unused','Unused server ID.',0),
 (8,'',0,0,0,'corellia','Zone server for Corellia',0),
 (9,'',0,0,0,'dantooine','Zone server for Dantooine',0),
 (10,'',0,0,0,'dathomir','Zone server for Dathomir',0),
 (11,'',0,0,0,'endor','Zone server for Endor',0),
 (12,'',0,0,0,'lok','Zone server for Lok',0),
 (13,'',0,0,0,'naboo','Zone server for Naboo',0),
 (14,'',0,0,0,'rori','Zone server for Rori',0),
 (15,'',0,0,0,'talus','Zone server for Talus',0),
 (16,'',0,0,0,'tatooine','Zone server for Tatooine',4),
 (17,'',0,0,0,'yavin4','Zone server for Yavin 4',0),
 (18,'',0,0,0,'space_corellia','Zone server for Corellian Space 1',0),
 (19,'',0,0,0,'space_corellia_2','Zone server for Corellian Space 2',0),
 (20,'',0,0,0,'space_dantooine','Zone server for Dantooine Space',0),
 (21,'',0,0,0,'space_dathomir','Zone server for Dathomir Space',0),
 (22,'',0,0,0,'space_endor','Zone server for Endor Space',0),
 (23,'',0,0,0,'space_env','Zone server for uknown space',0),
 (24,'',0,0,0,'space_halos','Zone server for unknown space',0),
 (25,'',0,0,0,'space_heavy1','Zone server for unknown space',0),
 (26,'',0,0,0,'space_light1','Zone server for unknown space',0),
 (27,'',0,0,0,'space_lok','Zone server for Lok Space',0),
 (28,'',0,0,0,'space_naboo','Zone server for Naboo Space 1',0),
 (29,'',0,0,0,'space_naboo_2','Zone server for Naboo Space 2',0),
 (30,'',0,0,0,'space_tatooine','Zone server for Tatooine Space 1',0),
 (31,'',0,0,0,'space_tatooine_2','Zone server for Tatooine Space 2',0),
 (32,'',0,0,0,'space_yavin4','Zone server to Yavin 4 Space',0),
 (33,'',0,0,0,'09','',0),
 (34,'',0,0,0,'10','',0),
 (35,'',0,0,0,'11','',0),
 (36,'',0,0,0,'character_farm','',0),
 (37,'',0,0,0,'cinco_city_test_m5','',0),
 (38,'',0,0,0,'creature_test','',0),
 (39,'',0,0,0,'dungeon1','',0),
 (40,'',0,0,0,'endor_asommers','',0),
 (41,'',0,0,0,'floratest','',0),
 (42,'',0,0,0,'godclient_test','',0),
 (43,'',0,0,0,'otoh_gunga','',0),
 (44,'',0,0,0,'rivertest','',0),
 (45,'',0,0,0,'runtimerules','',0),
 (46,'',0,0,0,'simple','',0),
 (47,'',0,0,0,'space_09','',0),
 (48,'',0,0,0,'test_wearables','',0),
 (49,'',0,0,0,'tutorial','',0),
 (50,'',0,0,0,'taanab','Zone server for Tanaab',0),
 (51,'',0,0,0,'dagobah','Zone server for Dagobah.',0);
/*!40000 ALTER TABLE `config_process_list` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
