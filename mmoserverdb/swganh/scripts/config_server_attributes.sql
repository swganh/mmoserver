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
-- Definition of table `config_server_attributes`
--

DROP TABLE IF EXISTS `config_server_attributes`;
CREATE TABLE `config_server_attributes` (
  `id` bigint(20) NOT NULL,
  `attribute` char(255) DEFAULT NULL,
  `attribute_description` char(255) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `config_server_attributes`
--

/*!40000 ALTER TABLE `config_server_attributes` DISABLE KEYS */;
INSERT INTO `config_server_attributes` (`id`,`attribute`,`attribute_description`) VALUES 
 (1,'server_si_fill_factor',NULL),
 (2,'server_si_index_cap',NULL),
 (3,'server_si_leaf_cap',NULL),
 (4,'server_si_horizon',NULL),
 (5,'server_db_username',NULL),
 (6,'server_db_password',NULL),
 (7,'server_db_ip',NULL),
 (8,'server_db_port',NULL),
 (9,'server_db_min_threads',NULL),
 (10,'server_db_max_threads',NULL),
 (11,'server_network_bind_address',NULL),
 (12,'server_network_bind_port',NULL),
 (13,'server_network_service_message_heap',NULL),
 (14,'server_network_global_message_heap',NULL),
 (15,'server_resource_write_maps','Server - Resources - Output Resource Maps (Flag On / Off)'),
 (16,'server_zone_name','Server - Zone Name'),
 (17,'motD','message for player on login'),
 (18,'Zone_Player_ViewingRange','the range to check for objects in the si'),
 (19,'Zone_Player_ChatRange',NULL),
 (20,'Zone_Player_Logout','seconds to playerdestroy after logout'),
 (21,'Server_Time_Interval','Interval we update the servertime in'),
 (22,'Server_Time_Speed',NULL),
 (23,'Player_Incapacitation','amount of incaps leading to death'),
 (24,'Server_Bazaar_MaxListing','amount of listings for the public bazaar'),
 (25,'Server_Bazaar_MaxPrice',NULL),
 (26,'Player_Health_RegenDivider',NULL),
 (27,'Player_Action_RegenDivider',NULL),
 (28,'Player_Mind_RegenDivider',NULL),
 (29,'Player_Incap_Time','time a player stays incapped'),
 (30,'Player_Incap_Reset','time after which the player incap counter resets');
/*!40000 ALTER TABLE `config_server_attributes` ENABLE KEYS */;




/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
