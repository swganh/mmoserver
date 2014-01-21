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
-- Definition of table `ticket_collectors`
--

DROP TABLE IF EXISTS `ticket_collectors`;
CREATE TABLE `ticket_collectors` (
  `id` bigint(20) unsigned NOT NULL,
  `parent_id` bigint(20) unsigned NOT NULL default '0',
  `object_string` char(255) NOT NULL default 'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',
  `oX` float NOT NULL default '0',
  `oY` float NOT NULL default '0',
  `oZ` float NOT NULL default '0',
  `oW` float NOT NULL default '0',
  `x` float NOT NULL default '0',
  `y` float NOT NULL default '0',
  `z` float NOT NULL default '0',
  `planet_id` int(2) NOT NULL default '0',
  `name` char(255) NOT NULL default 'ticket_collector_name',
  `file` char(255) NOT NULL default 'travel',
  `portDescriptor` char(255) NOT NULL COMMENT 'must match the ticket terminal datastr descriptor',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `ticket_collectors`
--

/*!40000 ALTER TABLE `ticket_collectors` DISABLE KEYS */;
INSERT INTO `ticket_collectors` (`id`,`parent_id`,`object_string`,`oX`,`oY`,`oZ`,`oW`,`x`,`y`,`z`,`planet_id`,`name`,`file`,`portDescriptor`) VALUES 
 (4563402753,1692104,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.998785,0,0.0492769,-0.1,8,-12.1,5,'ticket_collector_name','travel','Theed Spaceport'),
 (4563402754,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.103924,0,0.994585,3623.5,5,-4793.5,8,'ticket_collector_name','travel','Mos Eisley Starport'),
 (4563402755,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.485545,0,0.874212,3427.5,5.6,-4644.1,8,'ticket_collector_name','travel','Mos Eisley Shuttleport'),
 (4563402756,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,-0.08,-1382.15,12,-3583.25,8,'ticket_collector_name','travel','Bestine Starport'),
 (4563402757,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.67,0,0.74,-1090.8,12.6,-3554.9,8,'ticket_collector_name','travel','Bestine Shuttleport'),
 (4563402758,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.113814,0,0.993502,-2886.3,5.6,1929.4,8,'ticket_collector_name','travel','Mos Espa Shuttleport A'),
 (4563402759,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.621933,0,0.783071,-3116.3,5.6,2166.1,8,'ticket_collector_name','travel','Mos Espa Shuttleport B'),
 (4563402760,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.123692,0,0.992321,-2793,5.6,2178.4,8,'ticket_collector_name','travel','Mos Espa Shuttleport C'),
 (4563402761,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,38.15,52.6,-5332.2,8,'ticket_collector_name','travel','Anchorhead Shuttleport'),
 (4563402762,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.89,0,0.46,1241.95,7,3053.42,8,'ticket_collector_name','travel','Mos Entha Starport'),
 (4563402763,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,1386.12,7.6,3473.81,8,'ticket_collector_name','travel','Mos Entha Shuttleport A'),
 (4563402764,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,1721.5,7.6,3191.3,8,'ticket_collector_name','travel','Mos Entha Shuttleport B'),
 (4563402765,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.938551,0,0.345142,-5005.2,6.6,4086.9,5,'ticket_collector_name','travel','Theed Shuttleport A'),
 (4563402766,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0,0,1,-5401.6,6.6,4315.8,5,'ticket_collector_name','travel','Theed Shuttleport B'),
 (4563402767,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.710457,0,0.703799,-5862.8,6.6,4162.7,5,'ticket_collector_name','travel','Theed Shuttleport C'),
 (4563402768,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.373413,0,0.927665,5134.3,-191.4,6617.2,5,'ticket_collector_name','travel','Kaadara Shuttleport'),
 (4563402769,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.988256,0,0.152808,1347.8,13,2760.3,5,'ticket_collector_name','travel','Keren Starport'),
 (4563402770,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,1558.3,25.6,2844.8,5,'ticket_collector_name','travel','Keren Shuttleport A'),
 (4563402771,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.710457,0,0.703739,2027.7,19.6,2535.2,5,'ticket_collector_name','travel','Keren Shuttleport B'),
 (4563402772,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.710457,0,0.703739,5338.6,327.6,-1567.2,5,'ticket_collector_name','travel','Dee\'ja Peak Shuttleport'),
 (4563402773,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.710457,0,0.703739,-5487.8,-149.4,-12.5,5,'ticket_collector_name','travel','The Lake Retreat Shuttleport'),
 (4563402774,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,4717.7,4.2,-4650.6,5,'ticket_collector_name','travel','Moenia Starport'),
 (4563402775,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.71,0,-0.71,4968.6,4.4,-4883.3,5,'ticket_collector_name','travel','Moenia Shuttleport'),
 (4563402776,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,598.2,6,3095.4,2,'ticket_collector_name','travel','Trade Outpost'),
 (4563402777,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,-68.9,18,-1578.8,2,'ticket_collector_name','travel','Science Outpost'),
 (4563402778,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.881543,0,0.472105,-6929.3,73,-5707.2,9,'ticket_collector_name','travel','Labor Outpost'),
 (4563402779,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.938551,0,0.345142,-286.6,35,4888.6,9,'ticket_collector_name','travel','Mining Outpost'),
 (4563402780,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.938551,0,0.345142,4033.5,37,-6224.4,9,'ticket_collector_name','travel','an Imperial Outpost'),
 (4563402781,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.938551,0,0.345142,459.4,8.8,5504.3,4,'ticket_collector_name','travel','Nym\'s Stronghold'),
 (4563402782,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,-5381.5,80,-2162.8,6,'ticket_collector_name','travel','Narmle Starport'),
 (4563402783,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.924076,0,0.382209,-5257.2,81.3,-2150.3,6,'ticket_collector_name','travel','Narmle Shuttleport'),
 (4563402784,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.938551,0,0.345142,5354.4,80,5763.6,6,'ticket_collector_name','travel','Restuss Starport'),
 (4563402785,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,5203.9,80.6,5797,6,'ticket_collector_name','travel','Restuss Shuttleport'),
 (4563402786,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.938551,0,0.345142,3672.5,96,-6411.2,6,'ticket_collector_name','travel','Rebel Outpost'),
 (4563402787,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.345563,0,0.938396,3221.1,24,-3492.1,3,'ticket_collector_name','travel','Research Outpost'),
 (4563402788,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,-970,73,1560.7,3,'ticket_collector_name','travel','Smuggler\'s Outpost'),
 (4563402789,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0,0,1,1589,4,-6408,1,'ticket_collector_name','travel','a Pirate Outpost'),
 (4563402790,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,-4228.1,3,-2358,1,'ticket_collector_name','travel','a Imperial Outpost'),
 (4563402791,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.710457,0,0.703739,-643,3,2487.4,1,'ticket_collector_name','travel','a Mining Outpost'),
 (4563402792,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.710457,0,0.703739,4473.6,2,5372.3,7,'ticket_collector_name','travel','Nashal Starport'),
 (4563402793,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.710457,0,0.703739,4327.6,10.5,5422.8,7,'ticket_collector_name','travel','Nashal Shuttleport'),
 (4563402794,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.995612,0,0.0935771,240.2,6,-2939,7,'ticket_collector_name','travel','Dearic Starport'),
 (4563402795,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0,0,1,708.7,6.6,-3047.6,7,'ticket_collector_name','travel','Dearic Shuttleport'),
 (4563402796,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.710457,0,0.703739,-2221.6,20,2302.7,7,'ticket_collector_name','travel','Talus Imperial Outpost'),
 (4563402797,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0,0,1,-45.5,28,-4727.7,0,'ticket_collector_name','travel','Coronet Starport'),
 (4563402798,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,-34.2,28.6,-4402.6,0,'ticket_collector_name','travel','Coronet Shuttleport A'),
 (4563402799,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,-338.2,28.6,-4634,0,'ticket_collector_name','travel','Coronet Shuttleport B'),
 (4563402800,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.710457,0,0.703739,-4977.3,21,-2221,0,'ticket_collector_name','travel','Tyrena Starport'),
 (4563402801,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.710457,0,0.703739,-5607.7,21.6,-2799.2,0,'ticket_collector_name','travel','Tyrena Shuttleport A'),
 (4563402802,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.998547,0,0.0538918,-5013.5,21.6,-2379.6,0,'ticket_collector_name','travel','Tyrena Shuttleport B'),
 (4563402803,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,-5560.4,16.5,-6052.9,0,'ticket_collector_name','travel','Vreni Island Shuttleport'),
 (4563402804,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.710457,0,0.703739,6636.8,330.6,-5930.6,0,'ticket_collector_name','travel','Bela Vistal Shuttleport A'),
 (4563402805,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.629692,0,0.776845,6939.5,330.6,-5529,0,'ticket_collector_name','travel','Bela Vistal Shuttleport B'),
 (4563402806,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.938551,0,0.345142,-3143.6,31,2899.2,0,'ticket_collector_name','travel','Kor Vella Starport'),
 (4563402807,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,1,0,0,-3784.8,86.6,3240.9,0,'ticket_collector_name','travel','Kor Vella Shuttleport'),
 (4563402808,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,-0.710457,0,0.703739,3372.4,308,5612.9,0,'ticket_collector_name','travel','Doaba Guerfel Starport'),
 (4563402809,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.710457,0,0.703739,3079,280.6,4983.5,0,'ticket_collector_name','travel','Doaba Guerfel Shuttleport'),
 (4563402810,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0,0,1,-2820.5,5,2083.71,8,'ticket_collector_name','travel','Mos Espa Starport'),
 (4563402811,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0,0,1,5301.1,-192,6671.5,5,'ticket_collector_name','travel','Kaadara Starport'),
 (4563402812,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.71,0,0.71,114.3,45.6,3909.3,42,'ticket_collector_name','travel','Pandath Shuttleport'),
 (4563402813,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0.49,0,0.86,305,45,3735,42,'ticket_collector_name','travel','Pandath Starport'),
 (4563402814,0,'object/tangible/travel/ticket_collector/shared_ticket_collector.iff',0,0,0,1,3153.4,40.6,-3852,42,'ticket_collector_name','travel','Pendath Shuttleport');
/*!40000 ALTER TABLE `ticket_collectors` ENABLE KEYS */;




/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
