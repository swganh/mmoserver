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
-- Definition of table `conversation_animations`
--

DROP TABLE IF EXISTS `conversation_animations`;
CREATE TABLE `conversation_animations` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `name` char(255) NOT NULL default 'none',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=47 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `conversation_animations`
--

/*!40000 ALTER TABLE `conversation_animations` DISABLE KEYS */;
INSERT INTO `conversation_animations` (`id`,`name`) VALUES 
 (1,'bow'),
 (2,'beckon'),
 (3,'adn'),
 (4,'apologize'),
 (5,'ayt'),
 (6,'backhand'),
 (7,'blame'),
 (8,'bow2'),
 (9,'bow3'),
 (10,'bow4'),
 (11,'claw'),
 (12,'cuckoo'),
 (13,'curtsey'),
 (14,'dream'),
 (15,'rose'),
 (16,'giveup'),
 (17,'helpme'),
 (18,'huge'),
 (19,'medium'),
 (20,'small'),
 (21,'tiny'),
 (22,'jam'),
 (23,'loser'),
 (24,'mock'),
 (25,'model'),
 (26,'nod'),
 (27,'poke'),
 (28,'rude'),
 (29,'scare'),
 (30,'scared'),
 (31,'scream'),
 (32,'shiver'),
 (33,'shoo'),
 (34,'snap'),
 (35,'spin'),
 (36,'squirm'),
 (37,'strut'),
 (38,'sweat'),
 (39,'thank'),
 (40,'throwdown'),
 (41,'tiphat'),
 (42,'twitch'),
 (43,'worship'),
 (44,'yawn'),
 (45,'yes'),
 (46,'none');
/*!40000 ALTER TABLE `conversation_animations` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;