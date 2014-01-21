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
-- Definition of table `terminal_types`
--

DROP TABLE IF EXISTS `terminal_types`;
CREATE TABLE `terminal_types` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `object_string` char(255) default NULL,
  `name` char(255) NOT NULL default 'terminal_',
  `file` char(255) NOT NULL default 'unknown',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `pk_object_string` (`object_string`)
) ENGINE=InnoDB AUTO_INCREMENT=55 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `terminal_types`
--

/*!40000 ALTER TABLE `terminal_types` DISABLE KEYS */;
INSERT INTO `terminal_types` (`id`,`object_string`,`name`,`file`) VALUES 
 (1,'object/tangible/terminal/shared_terminal_mission.iff','terminal_mission','terminal_detail'),
 (2,'object/tangible/terminal/shared_terminal_bazaar.iff','terminal_bazaar','terminal_detail'),
 (3,'object/tangible/terminal/shared_terminal_bank.iff','terminal_bank','terminal_detail'),
 (4,'object/tangible/terminal/shared_terminal_mission_rebel.iff','terminal_mission_rebel','terminal_detail'),
 (5,'object/tangible/terminal/shared_terminal_hq.iff','terminal_hq','terminal_detail'),
 (6,'object/tangible/terminal/shared_terminal_mission_bounty.iff','terminal_mission_bounty','terminal_detail'),
 (7,'object/tangible/terminal/shared_terminal_space.iff','terminal_space','terminal_detail'),
 (8,'object/tangible/terminal/shared_terminal_mission_imperial.iff','terminal_mission_imperial','terminal_detail'),
 (9,'object/tangible/terminal/shared_terminal_newsnet.iff','terminal_newsnet','terminal_detail'),
 (10,'object/tangible/terminal/shared_terminal_mission_scout.iff','terminal_mission_scout','terminal_detail'),
 (11,'object/tangible/terminal/shared_terminal_mission_entertainer.iff','terminal_mission_entertainer','terminal_detail'),
 (12,'object/tangible/terminal/shared_terminal_mission_artisan.iff','terminal_mission_artisan','terminal_detail'),
 (13,'object/tangible/terminal/shared_terminal_bestine_quests_01.iff','terminal_bestine_01','terminal_detail'),
 (14,'object/tangible/terminal/shared_terminal_bestine_quests_02.iff','terminal_bestine_02','terminal_detail'),
 (15,'object/tangible/terminal/shared_terminal_elevator.iff','terminal_elevator','terminal_name'),
 (16,'object/tangible/terminal/shared_terminal_travel.iff','terminal_travel','terminal_detail'),
 (17,'object/tangible/terminal/shared_terminal_insurance.iff','terminal_insurance','terminal_detail'),
 (18,'object/tangible/terminal/shared_terminal_cloning.iff','terminal_cloning','terminal_detail'),
 (19,'object/tangible/terminal/shared_terminal_ballot_box.iff','terminal_ballot_box','terminal_detail'),
 (20,'object/tangible/terminal/shared_terminal_bounty_droid.iff','terminal_bounty_droid','terminal_detail'),
 (21,'object/tangible/terminal/shared_terminal_guild.iff','terminal_guild','terminal_name'),
 (22,'object/tangible/terminal/shared_terminal_mission_newbie.iff','terminal_mission','terminal_detail'),
 (23,'object/tangible/terminal/shared_terminal_mission_statue.iff','terminal_mission_statue','terminal_detail'),
 (24,'object/tangible/terminal/shared_terminal_newbie_clothing.iff','newbie_clothing_dispenser','terminal_detail'),
 (25,'object/tangible/terminal/shared_terminal_newbie_food.iff','newbie_food_dispenser','terminal_detail'),
 (26,'object/tangible/terminal/shared_terminal_newbie_instrument.iff','newbie_instrument_dispenser','terminal_detail'),
 (27,'object/tangible/terminal/shared_terminal_newbie_medicine.iff','newbie_medicine_dispenser','terminal_detail'),
 (28,'object/tangible/terminal/shared_terminal_newbie_tool.iff','newbie_tool_dispenser','terminal_detail'),
 (29,'object/tangible/terminal/shared_terminal_player_structure.iff','terminal_player_structure','terminal_name'),
 (30,'object/tangible/terminal/shared_terminal_hq_rebel.iff','terminal_hq_rebel','terminal_detail'),
 (31,'object/tangible/terminal/shared_terminal_hq_imperial.iff','terminal_hq_imperial','terminal_detail'),
 (32,'object/tangible/terminal/shared_terminal_pm_register.iff','terminal_pm_register','terminal_detail'),
 (33,'object/tangible/terminal/shared_terminal_skill.iff','unknown','terminal_detail'),
 (34,'object/tangible/terminal/shared_terminal_city.iff','terminal_city','terminal_detail'),
 (35,'object/tangible/terminal/shared_terminal_player_structure_nosnap.iff','unknown','terminal_detail'),
 (36,'object/tangible/terminal/shared_terminal_city_vote.iff','terminal_city_vote','terminal_detail'),
 (37,'object/tangible/terminal/shared_terminal_player_structure_nosnap_mini.iff','unknown','terminal_detail'),
 (38,'object/tangible/terminal/shared_terminal_nym_cave.iff','unknown','terminal_detail'),
 (39,'object/tangible/terminal/shared_terminal_command_console.iff','unknown','terminal_detail'),
 (40,'object/tangible/terminal/shared_terminal_geo_bunker.iff','terminal_geo_bunker','terminal_detail'),
 (41,'object/tangible/terminal/shared_terminal_bestine_quests_03.iff','terminal_bestine_03','terminal_detail'),
 (42,'object/tangible/terminal/shared_terminal_elevator_up.iff','terminal_elevator','terminal_detail'),
 (43,'object/tangible/terminal/shared_terminal_elevator_down.iff','terminal_elevator','terminal_detail'),
 (44,'object/tangible/terminal/shared_terminal_hq_turret_control.iff','terminal_hq_turret_control','terminal_detail'),
 (45,'object/tangible/terminal/shared_terminal_imagedesign.iff','terminal_imagedesign','terminal_detail'),
 (46,'object/tangible/terminal/shared_terminal_water_pressure.iff','terminal_water_pressure','terminal_detail'),
 (47,'object/tangible/terminal/shared_terminal_light_enclave_voting.iff','terminal_light_enclave_voting','terminal_detail'),
 (48,'object/tangible/terminal/shared_terminal_dark_enclave_challenge.iff','terminal_dark_enclave_challenge','terminal_detail'),
 (49,'object/tangible/terminal/shared_terminal_dark_enclave_voting.iff','terminal_dark_enclave_voting','terminal_detail'),
 (50,'object/tangible/terminal/shared_terminal_character_builder.iff','terminal_character_builder','terminal_detail'),
 (51,'object/tangible/terminal/shared_terminal_ship_interior_security_1.iff','unknown','terminal_detail'),
 (52,'object/tangible/terminal/shared_terminal_pob_ship.iff','unknown','terminal_detail'),
 (53,'object/tangible/terminal/shared_terminal_light_enclave_challenge.iff','terminal_light_enclave_challenge','terminal_detail'),
 (54,'object/tangible/terminal/shared_terminal_jukebox.iff','unknown','terminal_detail');
/*!40000 ALTER TABLE `terminal_types` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;