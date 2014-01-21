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
-- Definition of table `chat_channels`
--

DROP TABLE IF EXISTS `chat_channels`;
CREATE TABLE `chat_channels` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `name` char(64) NOT NULL default 'name',
  `private` tinyint(1) unsigned NOT NULL default '0',
  `moderated` tinyint(1) unsigned NOT NULL default '0',
  `creator` char(32) NOT NULL,
  `owner` char(32) NOT NULL,
  `title` char(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=66 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `chat_channels`
--

/*!40000 ALTER TABLE `chat_channels` DISABLE KEYS */;
INSERT INTO `chat_channels` VALUES
 (1,'corellia',0,0,'SYSTEM','SYSTEM',NULL),
 (2,'dantooine',0,0,'SYSTEM','SYSTEM',NULL),
 (3,'dathomir',0,0,'SYSTEM','SYSTEM',NULL),
 (4,'endor',0,0,'SYSTEM','SYSTEM',NULL), (5,'lok',0,0,'SYSTEM','SYSTEM',NULL),
 (6,'naboo',0,0,'SYSTEM','SYSTEM',NULL),
 (7,'rori',0,0,'SYSTEM','SYSTEM',NULL),
 (8,'taanab',0,0,'SYSTEM','SYSTEM',NULL),
 (9,'talus',0,0,'SYSTEM','SYSTEM',NULL), (10,'tatooine',0,0,'SYSTEM','SYSTEM',NULL),
 (11,'yavin4',0,0,'SYSTEM','SYSTEM',NULL),
 (12,'corellia.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (13,'dantooine.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (14,'dathomir.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (15,'endor.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (16,'lok.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (17,'naboo.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (18,'rori.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (19,'taanab.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (20,'talus.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (21,'tatooine.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (22,'yavin4.Chat',0,0,'SYSTEM','SYSTEM','public chat for this planet, can create rooms here'),
 (23,'corellia.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (24,'dantooine.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (25,'dathomir.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (26,'endor.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (27,'lok.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (28,'naboo.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (29,'rori.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (30,'talus.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (31,'tatooine.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (32,'yavin4.Planet',0,0,'SYSTEM','SYSTEM',NULL),
 (65,'taanab.Planet',0,0,'SYSTEM','SYSTEM',NULL);
/*!40000 ALTER TABLE `chat_channels` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;