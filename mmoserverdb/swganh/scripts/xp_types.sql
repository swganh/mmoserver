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
-- Definition of table `xp_types`
--

DROP TABLE IF EXISTS `xp_types`;
CREATE TABLE `xp_types` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `def_cap` int(11) unsigned NOT NULL,
  `xp_type` char(255) NOT NULL,
  `xp_type_ex` char(255) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `pk_xptypes_name` (`xp_type`)
) ENGINE=InnoDB AUTO_INCREMENT=50 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `xp_types`
--

/*!40000 ALTER TABLE `xp_types` DISABLE KEYS */;
INSERT INTO `xp_types` (`id`,`def_cap`,`xp_type`,`xp_type_ex`) VALUES
 (1,1240,'apprenticeship','Apprenticeship'),
 (2,20000,'crafting_bio_engineer_creature','Bio-Engineer Crafting'),
 (3,20000,'bio_engineer_dna_harvesting','DNA Sampling'),
 (4,10000,'bountyhunter','Bounty Hunter'),
 (5,2000,'camp','Wilderness Survival'),
 (6,170000,'combat_general','Combat'),
 (7,3000,'combat_meleespecialize_onehandlightsaber','Onehanded Lightsaber'),
 (8,200000,'combat_meleespecialize_twohandlightsaber','Twohanded Lightsaber'),
 (9,200000,'combat_meleespecialize_polearmlightsaber','Polearm Lightsaber'),
 (10,90000,'jedi_general','Jedi'),
 (11,2000,'combat_meleespecialize_onehand','Onehanded Weapons'),
 (12,2000,'combat_meleespecialize_polearm','Polearm Weapons'),
 (13,2000,'combat_meleespecialize_twohand','Twohanded Weapons'),
 (14,2000,'combat_meleespecialize_unarmed','Unarmed Combat'),
 (15,2000,'combat_rangedspecialize_carbine','Carbine Weapons'),
 (16,2000,'combat_rangedspecialize_pistol','Pistol Weapons'),
 (17,2000,'combat_rangedspecialize_rifle','Rifle Weapons'),
 (18,56000,'crafting_clothing_armor','Armor Crafting'),
 (19,56000,'crafting_clothing_general','Tailoring'),
 (20,56000,'crafting_droid_general','Droid Crafting'),
 (21,56000,'crafting_food_general','Food Crafting'),
 (22,1000,'crafting_general','General Crafting'),
 (23,1000,'crafting_medicine_general','Medicine Crafting'),
 (24,45000,'crafting_spice','Spice Crafting'),
 (25,140000,'crafting_structure_general','Structure Crafting'),
 (26,56000,'crafting_weapons_general','Weapon Crafting'),
 (27,20000,'creaturehandler','Creature Handling'),
 (28,2000,'dance','Dancing'),
 (29,2000,'entertainer_healing','Entertainer Healing'),
 (30,2000,'imagedesigner','Image Designer'),
 (31,2000,'jedi','Todo: jedi'),
 (32,2000,'medical','Medical'),
 (33,10000,'merchant','Merchant'),
 (34,2000,'music','Musician'),
 (35,1000,'resource_harvesting_inorganic','Surveying'),
 (36,2000,'scout','Scouting'),
 (37,30000,'slicing','Slicing'),
 (38,500000,'squadleader','Squad Leadership'),
 (39,2000,'trapping','Trapping'),
 (40,300000,'combat_rangedspecialize_heavy','Heavy Weapons'),
 (41,20000,'political','Political'),
 (42,625000,'force_rank_xp','Force Rank'),
 (43,5100000,'fs_combat','Force-sensitive Combat'),
 (44,5100000,'fs_crafting','Force-sensitive Crafting'),
 (45,5100000,'fs_senses','Force-sensitive Senses'),
 (46,5100000,'fs_reflex','Force-sensitive Reflex'),
 (47,60000,'shipwright','Shipwright'),
 (48,10000,'space_combat_general','Starship Combat'),
 (49,0,'none','None');
/*!40000 ALTER TABLE `xp_types` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;