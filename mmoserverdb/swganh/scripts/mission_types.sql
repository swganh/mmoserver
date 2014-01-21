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
-- Definition of table `mission_types`
--

DROP TABLE IF EXISTS `mission_types`;
CREATE TABLE `mission_types` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` char(128) NOT NULL,
  `content` int(10) unsigned NOT NULL DEFAULT '0',
  `name` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=144 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `mission_types`
--

/*!40000 ALTER TABLE `mission_types` DISABLE KEYS */;
INSERT INTO `mission_types` (`id`,`type`,`content`,`name`) VALUES 
 (1,'mission_bounty_imperial_easy',25,0),
 (2,'mission_bounty_imperial_hard',25,0),
 (3,'mission_bounty_imperial_hard_jedi',30,0),
 (4,'mission_bounty_imperial_medium',50,0),
 (5,'mission_bounty_informant',0,0),
 (6,'mission_bounty_jedi',6,0),
 (7,'mission_bounty_neutral_easy',25,0),
 (8,'mission_bounty_neutral_hard',25,0),
 (9,'mission_bounty_neutral_medium',50,0),
 (10,'mission_deliver_imperial_easy',25,0),
 (11,'mission_deliver_imperial_easy_non_persistent_from_npc',25,0),
 (12,'mission_deliver_imperial_hard',25,0),
 (13,'mission_deliver_imperial_hard_non_persistent_from_npc',25,0),
 (14,'mission_deliver_imperial_medium',50,0),
 (15,'mission_deliver_imperial_medium_non_persistent_from_npc',25,0),
 (16,'mission_deliver_neutral_easy',30,0),
 (17,'mission_deliver_neutral_easy_non_persistent_from_npc',35,0),
 (18,'mission_deliver_neutral_hard',25,0),
 (19,'mission_deliver_neutral_hard_non_persistent_from_npc',15,0),
 (20,'mission_deliver_neutral_medium',51,0),
 (21,'mission_deliver_neutral_medium_non_persistent_from_npc',35,0),
 (22,'mission_deliver_rebel_easy',25,0),
 (23,'mission_deliver_rebel_easy_non_persistent_from_npc',25,0),
 (24,'mission_deliver_rebel_hard',25,0),
 (25,'mission_deliver_rebel_hard_non_persistent_from_npc',25,0),
 (26,'mission_deliver_rebel_medium',50,0),
 (27,'mission_deliver_rebel_medium_non_persistent_from_npc',25,0),
 (28,'mission_destroy_imperial_military_easy',50,0),
 (29,'mission_destroy_imperial_military_easy_non_persistent_from_npc',25,0),
 (30,'mission_destroy_imperial_military_hard',50,0),
 (31,'mission_destroy_imperial_military_hard_non_persistent_from_npc',25,0),
 (32,'mission_destroy_imperial_military_medium',50,0),
 (33,'mission_destroy_imperial_military_medium_non_persistent_from_npc',25,0),
 (34,'mission_destroy_imperial_non_military_easy',13,0),
 (35,'mission_destroy_imperial_non_military_easy_non_persistent_from_npc',25,0),
 (36,'mission_destroy_imperial_non_military_hard',25,1),
 (37,'mission_destroy_imperial_non_military_hard_non_persistent_from_npc',25,0),
 (38,'mission_destroy_imperial_non_military_medium',50,0),
 (39,'mission_destroy_imperial_non_military_medium_non_persistent_from_npc',25,0),
 (40,'mission_destroy_neutral_easy_creature',35,0),
 (41,'mission_destroy_neutral_easy_creature_corellia',35,0),
 (42,'mission_destroy_neutral_easy_creature_naboo',35,0),
 (43,'mission_destroy_neutral_easy_creature_non_persistent_from_npc',35,0),
 (44,'mission_destroy_neutral_easy_creature_tatooine',35,0),
 (45,'mission_destroy_neutral_easy_npc',35,1),
 (46,'mission_destroy_neutral_easy_npc_corellia',35,0),
 (47,'mission_destroy_neutral_easy_npc_naboo',35,0),
 (48,'mission_destroy_neutral_easy_npc_non_persistent_from_npc',35,0),
 (49,'mission_destroy_neutral_easy_npc_tatooine',35,0),
 (50,'mission_destroy_neutral_hard_creature',35,0),
 (51,'mission_destroy_neutral_hard_creature_corellia',15,0),
 (52,'mission_destroy_neutral_hard_creature_naboo',15,0),
 (53,'mission_destroy_neutral_hard_creature_non_persistent_from_npc',15,0),
 (54,'mission_destroy_neutral_hard_creature_tatooine',18,0),
 (55,'mission_destroy_neutral_hard_npc',35,0),
 (56,'mission_destroy_neutral_hard_npc_corellia',15,0),
 (57,'mission_destroy_neutral_hard_npc_naboo',15,0),
 (58,'mission_destroy_neutral_hard_npc_non_persistent_from_npc',15,0),
 (59,'mission_destroy_neutral_hard_npc_tatooine',15,0),
 (60,'mission_destroy_neutral_medium_creature',50,0),
 (61,'mission_destroy_neutral_medium_creature_corellia',35,0),
 (62,'mission_destroy_neutral_medium_creature_naboo',35,0),
 (63,'mission_destroy_neutral_medium_creature_non_persistent_from_npc',35,0),
 (64,'mission_destroy_neutral_medium_creature_tatooine',45,0),
 (65,'mission_destroy_neutral_medium_npc',50,0),
 (66,'mission_destroy_neutral_medium_npc_corellia',35,0),
 (67,'mission_destroy_neutral_medium_npc_naboo',35,0),
 (68,'mission_destroy_neutral_medium_npc_non_persistent_from_npc',35,0),
 (69,'mission_destroy_neutral_medium_npc_tatooine',35,0),
 (70,'mission_destroy_rebel_military_easy',50,0),
 (71,'mission_destroy_rebel_military_easy_non_persistent_from_npc',25,0),
 (72,'mission_destroy_rebel_military_hard',50,0),
 (73,'mission_destroy_rebel_military_hard_non_persistent_from_npc',25,0),
 (74,'mission_destroy_rebel_military_medium',50,0),
 (75,'mission_destroy_rebel_military_medium_non_persistent_from_npc',25,0),
 (76,'mission_destroy_rebel_non_military_easy',15,0),
 (77,'mission_destroy_rebel_non_military_easy_non_persistent_from_npc',25,0),
 (78,'mission_destroy_rebel_non_military_hard',25,0),
 (79,'mission_destroy_rebel_non_military_hard_non_persistent_from_npc',25,0),
 (80,'mission_destroy_rebel_non_military_medium',50,0),
 (81,'mission_destroy_rebel_non_military_medium_non_persistent_from_npc',25,0),
 (82,'mission_generic',0,0),
 (83,'mission_npc_crafting_imperial_easy',0,0),
 (84,'mission_npc_crafting_neutral_easy',0,0),
 (85,'mission_npc_crafting_rebel_easy',0,0),
 (86,'mission_npc_dancer_imperial_easy',35,0),
 (87,'mission_npc_dancer_neutral_easy',50,0),
 (88,'mission_npc_dancer_rebel_easy',35,0),
 (89,'mission_npc_deliver_imperial_easy',50,0),
 (90,'mission_npc_deliver_imperial_hard',50,0),
 (91,'mission_npc_deliver_imperial_medium',50,0),
 (92,'mission_npc_deliver_neutral_easy',25,0),
 (93,'mission_npc_deliver_neutral_hard',25,0),
 (94,'mission_npc_deliver_neutral_medium',50,0),
 (95,'mission_npc_deliver_rebel_easy',50,0),
 (96,'mission_npc_deliver_rebel_hard',50,0),
 (97,'mission_npc_deliver_rebel_medium',50,0),
 (98,'mission_npc_destroy_imperial_non_military_easy',50,0),
 (99,'mission_npc_destroy_imperial_non_military_hard',25,0),
 (100,'mission_npc_destroy_imperial_non_military_medium',50,0),
 (101,'mission_npc_destroy_neutral_easy_creature',25,0),
 (102,'mission_npc_destroy_neutral_easy_npc',25,0),
 (103,'mission_npc_destroy_neutral_hard_creature',25,0),
 (104,'mission_npc_destroy_neutral_hard_npc',25,0),
 (105,'mission_npc_destroy_neutral_medium_creature',50,0),
 (106,'mission_npc_destroy_neutral_medium_npc',50,0),
 (107,'mission_npc_destroy_rebel_non_military_easy',50,0),
 (108,'mission_npc_destroy_rebel_non_military_hard',25,0),
 (109,'mission_npc_destroy_rebel_non_military_medium',50,0),
 (110,'mission_npc_escort2me_imperial_easy',0,0),
 (111,'mission_npc_escort2me_rebel_easy',0,0),
 (112,'mission_npc_escorttocreator_imperial_easy',50,0),
 (113,'mission_npc_escorttocreator_imperial_hard',50,0),
 (114,'mission_npc_escorttocreator_imperial_medium',50,0),
 (115,'mission_npc_escorttocreator_neutral_easy',25,0),
 (116,'mission_npc_escorttocreator_neutral_hard',25,0),
 (117,'mission_npc_escorttocreator_neutral_medium',50,0),
 (118,'mission_npc_escorttocreator_rebel_easy',50,0),
 (119,'mission_npc_escorttocreator_rebel_hard',50,0),
 (120,'mission_npc_escorttocreator_rebel_medium',50,0),
 (121,'mission_npc_escort_imperial_easy',50,0),
 (122,'mission_npc_escort_neutral_easy',25,0),
 (123,'mission_npc_escort_neutral_hard',25,0),
 (124,'mission_npc_escort_neutral_medium',50,0),
 (125,'mission_npc_escort_rebel_easy',50,0),
 (126,'mission_npc_hunting_neutral_easy',8,0),
 (127,'mission_npc_hunting_neutral_hard',8,0),
 (128,'mission_npc_hunting_neutral_medium',8,0),
 (129,'mission_npc_musician_imperial_easy',35,0),
 (130,'mission_npc_musician_neutral_easy',50,0),
 (131,'mission_npc_musician_rebel_easy',35,0),
 (132,'mission_npc_recon_imperial_easy',50,0),
 (133,'mission_npc_recon_imperial_easy.tab',50,0),
 (134,'mission_npc_recon_neutral_easy',25,0),
 (135,'mission_npc_recon_neutral_hard',25,0),
 (136,'mission_npc_recon_neutral_medium',50,0),
 (137,'mission_npc_recon_rebel_easy',50,0),
 (138,'mission_npc_survey_imperial_easy',35,0),
 (139,'mission_npc_survey_neutral_easy',50,0),
 (140,'mission_npc_survey_rebel_easy',35,0),
 (141,'mission_object',0,0),
 (142,'special',0,0),
 (143,'survey',0,0);
/*!40000 ALTER TABLE `mission_types` ENABLE KEYS */;




/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
