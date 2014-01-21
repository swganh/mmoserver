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
-- Definition of table `config_zones`
--

DROP TABLE IF EXISTS `config_zones`;
CREATE TABLE `config_zones` (
  `id` int(11) NOT NULL default '0',
  `motD` text,
  `player_viewing_range` int(3) unsigned NOT NULL default '128',
  `player_chat_range` int(3) unsigned NOT NULL default '64',
  `player_logged_timeout` int(3) unsigned NOT NULL default '60' COMMENT 'seconds',
  `servertime_update_frequency` int(3) unsigned NOT NULL default '30' COMMENT 'seconds',
  `servertime_speed` int(11) unsigned NOT NULL default '0',
  `weather_update_frequency` int(4) unsigned NOT NULL default '120',
  `weather_max_type_id` int(2) unsigned NOT NULL default '5',
  `health_regen_divider` float(5,2) NOT NULL default '100.00',
  `action_regen_divider` float(5,2) NOT NULL default '100.00',
  `mind_regen_divider` float(5,2) NOT NULL default '100.00',
  `player_max_incaps` int(1) unsigned NOT NULL default '3' COMMENT 'nr of incaps until death',
  `player_base_incap_time` int(11) unsigned NOT NULL default '30' COMMENT 'seconds spend per incap',
  `player_base_incap_reset` int(11) unsigned NOT NULL default '300' COMMENT 'seconds until incap counter resets',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPRESSED;

--
-- Dumping data for table `config_zones`
--

/*!40000 ALTER TABLE `config_zones` DISABLE KEYS */;
INSERT INTO `config_zones` (`id`,`motD`,`player_viewing_range`,`player_chat_range`,`player_logged_timeout`,`servertime_update_frequency`,`servertime_speed`,`weather_update_frequency`,`weather_max_type_id`,`health_regen_divider`,`action_regen_divider`,`mind_regen_divider`,`player_max_incaps`,`player_base_incap_time`,`player_base_incap_reset`) VALUES 
 (0,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (1,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (2,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (3,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (4,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (5,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (6,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (7,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (8,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (9,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (10,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (11,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (12,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (13,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (14,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (15,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (16,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (17,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (18,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (19,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (20,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (21,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (22,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (23,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (24,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (25,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (26,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (27,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (28,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (29,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (30,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (31,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (32,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (33,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (34,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (35,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (36,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (37,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (38,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (39,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (40,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (41,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300),
 (42,'Server SysMessage Goes Here',128,64,10,30,0,120,3,100.00,100.00,100.00,3,30,300);
/*!40000 ALTER TABLE `config_zones` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;