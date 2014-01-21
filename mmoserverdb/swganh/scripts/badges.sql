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
-- Definition of table `badges`
--

DROP TABLE IF EXISTS `badges`;
CREATE TABLE `badges` (
  `Id` int(11) unsigned NOT NULL default '0',
  `name` char(255) default NULL,
  `sound` int(11) unsigned NOT NULL default '0',
  `category` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`),
  UNIQUE KEY `pk_badges_name` (`name`),
  KEY `fk_badges_cat_cat` (`category`),
  KEY `fk_badges_sound_sound` (`sound`),
  CONSTRAINT `fk_badges_cat_cat` FOREIGN KEY (`category`) REFERENCES `badge_categories` (`Id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_badges_sound_sound` FOREIGN KEY (`sound`) REFERENCES `sounds` (`Id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPRESSED;

--
-- Dumping data for table `badges`
--

/*!40000 ALTER TABLE `badges` DISABLE KEYS */;
INSERT INTO `badges` (`Id`,`name`,`sound`,`category`) VALUES 
 (0,'count_5',3074,1),
 (1,'count_10',3074,1),
 (2,'count_25',3074,1),
 (3,'count_50',3074,1),
 (4,'count_75',3074,1),
 (5,'count_100',3074,1),
 (6,'count_125',3074,1),
 (7,'poi_rabidbeast',3074,2),
 (8,'poi_prisonbreak',3074,2),
 (9,'poi_twoliars',3074,2),
 (10,'poi_factoryliberation',3074,2),
 (11,'poi_heromark',3074,2),
 (12,'exp_tat_bens_hut',3074,3),
 (13,'exp_tat_tusken_pool',3074,4),
 (14,'exp_tat_krayt_skeleton',3074,4),
 (15,'exp_tat_escape_pod',3074,5),
 (16,'exp_tat_sarlacc_pit',3074,4),
 (17,'exp_tat_lars_homestead',3074,5),
 (18,'exp_tat_krayt_graveyard',3074,4),
 (19,'exp_nab_gungan_sacred_place',3074,5),
 (20,'exp_cor_agrilat_swamp',3074,5),
 (21,'exp_yav_temple_woolamander',3074,5),
 (22,'exp_yav_temple_blueleaf',3074,5),
 (23,'exp_yav_temple_exar_kun',3074,3),
 (24,'exp_lok_volcano',3074,5),
 (25,'exp_dat_tarpit',3074,5),
 (26,'exp_dat_sarlacc',3074,4),
 (27,'exp_dat_escape_pod',3074,5),
 (28,'exp_dat_misty_falls_1',3074,5),
 (29,'exp_dat_misty_falls_2',3074,5),
 (30,'exp_dan_jedi_temple',3074,3),
 (31,'exp_dan_rebel_base',3074,5),
 (32,'event_project_dead_eye_1',3074,6),
 (33,'acc_good_samaritan',3074,6),
 (34,'acc_fascinating_background',3074,6),
 (35,'acc_brave_soldier',3074,6),
 (36,'acc_interesting_personage',3074,6),
 (37,'acc_professional_demeanor',3074,6),
 (38,'warren_compassion',3074,7),
 (39,'warren_hero',3074,7),
 (40,'event_coa2_rebel',3074,6),
 (41,'event_coa2_imperial',3074,6),
 (42,'combat_1hsword_master',3074,8),
 (43,'combat_2hsword_master',3074,8),
 (44,'combat_bountyhunter_master',3074,8),
 (45,'combat_brawler_master',3074,8),
 (46,'combat_carbine_master',3074,8),
 (47,'combat_commando_master',3074,8),
 (48,'combat_marksman_master',3074,8),
 (49,'combat_pistol_master',3074,8),
 (50,'combat_polearm_master',3074,8),
 (51,'combat_rifleman_master',3074,8),
 (52,'combat_smuggler_master',3074,8),
 (53,'combat_unarmed_master',3074,8),
 (54,'crafting_architect_master',3074,8),
 (55,'crafting_armorsmith_master',3074,8),
 (56,'crafting_artisan_master',3074,8),
 (57,'crafting_chef_master',3074,8),
 (58,'crafting_droidengineer_master',3074,8),
 (59,'crafting_merchant_master',3074,8),
 (60,'crafting_tailor_master',3074,8),
 (61,'crafting_weaponsmith_master',3074,8),
 (62,'outdoors_bio_engineer_master',3074,8),
 (63,'outdoors_creaturehandler_master',3074,8),
 (64,'outdoors_ranger_master',3074,8),
 (65,'outdoors_scout_master',3074,8),
 (66,'outdoors_squadleader_master',3074,8),
 (67,'science_combatmedic_master',3074,8),
 (68,'science_doctor_master',3074,8),
 (69,'science_medic_master',3074,8),
 (70,'social_dancer_master',3074,8),
 (71,'social_entertainer_master',3074,8),
 (72,'social_imagedesigner_master',3074,8),
 (73,'social_musician_master',3074,8),
 (74,'social_politician_master',3074,8),
 (75,'bdg_exp_nab_theed_falls_bottom',3074,5),
 (76,'bdg_exp_nab_deeja_falls_top',3074,5),
 (77,'bdg_exp_nab_amidalas_sandy_beach',3074,5),
 (78,'bdg_exp_cor_rebel_hideout',3074,5),
 (79,'bdg_exp_cor_rogue_corsec_base',3074,5),
 (80,'bdg_exp_cor_tyrena_theater',3074,5),
 (81,'bdg_exp_cor_bela_vistal_fountain',3074,5),
 (82,'bdg_exp_dat_crashed_ship',3074,5),
 (83,'bdg_exp_dat_imp_prison',3074,5),
 (84,'bdg_exp_dan_dantari_village1',3074,5),
 (85,'bdg_exp_dan_dantari_village2',3074,5),
 (86,'bdg_exp_end_ewok_tree_village',3074,5),
 (87,'bdg_exp_end_ewok_lake_village',3074,5),
 (88,'bdg_exp_end_dulok_village',3074,5),
 (89,'bdg_exp_end_imp_outpost',3074,5),
 (90,'bdg_exp_tal_creature_village',3074,5),
 (91,'bdg_exp_tal_imp_base',3074,5),
 (92,'bdg_exp_tal_imp_vs_reb_battle',3074,5),
 (93,'bdg_exp_tal_aqualish_cave',3074,5),
 (94,'bdg_exp_ror_kobala_spice_mine',3074,5),
 (95,'bdg_exp_ror_rebel_outpost',3074,5),
 (96,'bdg_exp_ror_imp_camp',3074,5),
 (97,'bdg_exp_ror_imp_hyperdrive_fac',3074,5),
 (98,'bdg_exp_lok_imp_outpost',3074,5),
 (99,'bdg_exp_lok_kimogila_skeleton',3074,5),
 (100,'bdg_exp_10_badges',3074,1),
 (101,'bdg_exp_20_badges',3074,1),
 (102,'bdg_exp_30_badges',3074,1),
 (103,'bdg_exp_40_badges',3074,1),
 (104,'bdg_exp_45_badges',3074,1),
 (105,'bdg_thm_park_jabba_badge',3074,7),
 (106,'bdg_thm_park_imperial_badge',3074,7),
 (107,'bdg_thm_park_rebel_badge',3074,7),
 (108,'bdg_thm_park_nym_badge',3074,7),
 (109,'event_coa3_rebel',3074,6),
 (110,'event_coa3_imperial',3074,6),
 (111,'bdg_library_trivia',3074,6),
 (112,'bdg_corvette_imp_destroy',3074,7),
 (113,'bdg_corvette_imp_rescue',3074,7),
 (114,'bdg_corvette_imp_assassin',3074,7),
 (115,'bdg_corvette_neutral_destroy',3074,7),
 (116,'bdg_corvette_neutral_rescue',3074,7),
 (117,'bdg_corvette_neutral_assassin',3074,7),
 (118,'bdg_corvette_reb_destroy',3074,7),
 (119,'bdg_corvette_reb_rescue',3074,7),
 (120,'bdg_corvette_reb_assassin',3074,7),
 (121,'bdg_racing_agrilat_swamp',3074,6),
 (122,'bdg_racing_keren_city',3074,6),
 (123,'bdg_racing_mos_espa',3074,6),
 (124,'bdg_accolade_live_event',3074,6),
 (125,'bdg_racing_lok_marathon',3074,6),
 (126,'bdg_racing_narmle_memorial',3074,6),
 (127,'bdg_racing_nashal_river',3074,6),
 (128,'destroy_deathstar',3074,6),
 (129,'crafting_shipwright',3074,8),
 (130,'pilot_rebel_navy_naboo',3074,8),
 (131,'pilot_rebel_navy_corellia',3074,8),
 (132,'pilot_rebel_navy_tatooine',3074,8),
 (133,'pilot_imperial_navy_naboo',3074,8),
 (134,'pilot_imperial_navy_corellia',3074,8),
 (135,'pilot_imperial_navy_tatooine',3074,8),
 (136,'pilot_neutral_naboo',3074,8),
 (137,'pilot_neutral_corellia',3074,8),
 (138,'pilot_neutral_tatooine',3074,8),
 (139,'bdg_accolade_home_show',3074,6),
 (140,'taanab_starport',3074,5);
/*!40000 ALTER TABLE `badges` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;