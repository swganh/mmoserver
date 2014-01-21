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

USE swganh;


--
-- Definition of table `galaxy_account`
--

DROP TABLE IF EXISTS `galaxy_account`;
CREATE TABLE `galaxy_account` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `galaxy_id` bigint(20) unsigned NOT NULL,
  `account_type` bigint(20) unsigned NOT NULL,
  `account_credits` bigint(20) unsigned NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `FK_galaxy_account_1` (`account_type`),
  CONSTRAINT `FK_galaxy_account_1` FOREIGN KEY (`account_type`) REFERENCES `galaxy_account_types` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `galaxy_account`
--

/*!40000 ALTER TABLE `galaxy_account` DISABLE KEYS */;
SET AUTOCOMMIT=0;
INSERT INTO `galaxy_account` (`id`,`galaxy_id`,`account_type`,`account_credits`) VALUES 
 (1,2,1,0),
 (2,2,2,0),
 (3,2,3,0),
 (4,2,4,0),
 (5,2,5,0),
 (6,2,6,0),
 (7,2,7,0),
 (8,2,8,0),
 (9,2,9,0),
 (10,2,10,0),
 (11,2,11,0),
 (12,2,12,0),
 (13,2,13,0),
 (14,2,14,0),
 (15,2,15,0),
 (16,2,16,0),
 (17,2,17,0),
 (18,2,18,0),
 (19,2,19,0),
 (20,2,20,0),
 (21,2,21,0),
 (22,2,22,0),
 (23,2,23,0),
 (24,2,24,0),
 (25,2,25,0),
 (26,2,26,0),
 (27,2,27,0),
 (28,2,28,0),
 (29,2,29,0),
 (30,2,30,0),
 (31,2,31,0),
 (32,2,32,0),
 (33,2,33,0),
 (34,2,34,0),
 (35,2,35,0);
COMMIT;
/*!40000 ALTER TABLE `galaxy_account` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;