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
-- Definition of table `galaxy_account_types`
--

DROP TABLE IF EXISTS `galaxy_account_types`;
CREATE TABLE `galaxy_account_types` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `type` char(255) NOT NULL,
  `description` char(255) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=36 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `galaxy_account_types`
--

/*!40000 ALTER TABLE `galaxy_account_types` DISABLE KEYS */;
SET AUTOCOMMIT=0;
INSERT INTO `galaxy_account_types` (`id`,`type`,`description`) VALUES
 (1,'charactercreation','Character Creation'),
 (2,'newbietutorial','Newbie Tutorial'),
 (3,'customerservice','Customer Service'),
 (4,'missionsystemdynamic','Dynamic Mission System'),
 (5,'missionsystemplayer','Player Mission System'),
 (6,'bountysystem','Bounty Mission System'),
 (7,'cloningsystem','Cloning System'),
 (8,'insurancesystem','Insurance System'),
 (9,'travelsystem','Galactic Travel Commission'),
 (10,'shippingsystem','Galactic Shipping Commission'),
 (11,'bazaarsystem','Galactic Trade Commission'),
 (12,'dispensersystem','Dispenser System'),
 (13,'skilltrainingsystem','Skill Training Union'),
 (14,'rebelfaction','Rebellion'),
 (15,'imperialfaction','the Empire'),
 (16,'jabbaspalace','Jabba the Hutt'),
 (17,'poisystem','POI System'),
 (18,'corpseexpiraton','Corpse Expiration Tracking'),
 (19,'testing','Testing'),
 (20,'structuremaintenance','Structure Maintenance'),
 (21,'tipsurcharge','Tip Surcharge'),
 (22,'vendormaintenance','Vendor Wages');
INSERT INTO `galaxy_account_types` (`id`,`type`,`description`) VALUES 
 (23,'npcloot','NPC Loot'),
 (24,'junkdealer','Junk Dealer'),
 (25,'cantinadrink','Cantina Drink'),
 (26,'betatest','Beta Test Fund'),
 (27,'badgegroupcoinsplit','Group Split Error Account'),
 (28,'gamblingslotstandard','Standard Slot Machine Account'),
 (29,'gamblingroulette','Roulette Account'),
 (30,'gamblingcardsabacc','Sabacc Account'),
 (31,'vehiclerepairs','Vehicle Repair System'),
 (32,'relicdealer','Relic Dealer'),
 (33,'newplayerquests','New Player Quests'),
 (34,'fines','Contraband Scanning Fines'),
 (35,'bank','Bank');
COMMIT;
/*!40000 ALTER TABLE `galaxy_account_types` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;