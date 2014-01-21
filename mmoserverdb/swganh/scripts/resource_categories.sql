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
-- Definition of table `resource_categories`
--

DROP TABLE IF EXISTS `resource_categories`;
CREATE TABLE `resource_categories` (
  `id` int(11) unsigned NOT NULL default '0',
  `name` char(255) default NULL,
  `typename` char(255) NOT NULL,
  `parent_id` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `pk_resourcecategories_typename` (`typename`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `resource_categories`
--

/*!40000 ALTER TABLE `resource_categories` DISABLE KEYS */;
INSERT INTO `resource_categories` (`id`,`name`,`typename`,`parent_id`) VALUES 
 (2,'Organic','organic',1),
 (3,'Creature Resource','creature_resource',2),
 (4,'Creature Food','creature_food',3),
 (5,'Milk','milk',4),
 (6,'Domesticated Milk','milk_domesticated',5),
 (17,'Wild Milk','milk_wild',5),
 (28,'Meat','meat',4),
 (29,'Domesticated Meat','meat_domesticated',28),
 (40,'Wild Meat','meat_wild',28),
 (51,'Herbivore Meat','meat_herbivore',28),
 (62,'Carnivore Meat','meat_carnivore',28),
 (73,'Reptilian Meat','meat_reptilian',28),
 (84,'Avian Meat','meat_avian',28),
 (95,'Egg','meat_egg',28),
 (106,'Insect Meat','meat_insect',28),
 (117,'Seafood','seafood',28),
 (149,'Creature Structural','creature_structural',3),
 (150,'Bone','bone',149),
 (161,'Horn','bone_horn',149),
 (172,'Avian Bone','bone_avian',150),
 (183,'Hide','hide',149),
 (184,'Wooly Hide','hide_wooly',183),
 (195,'Bristley Hide','hide_bristley',183),
 (206,'Leathery Hide','hide_leathery',183),
 (217,'Scaley Hide','hide_scaley',183),
 (228,'Flora Resources','flora_resources',2),
 (229,'Flora Food','flora_food',228),
 (230,'Cereal','cereal',229),
 (231,'Corn','corn',230),
 (232,'Domesticated Corn','corn_domesticated',231),
 (243,'Wild Corn','corn_wild',231),
 (254,'Rice','rice',230),
 (255,'Domesticated Rice','rice_domesticated',254),
 (266,'Wild Rice','rice_wild',254),
 (277,'Oats','oats',230),
 (278,'Domesticated Oats','oats_domesticated',277),
 (289,'Wild Oats','oats_wild',277),
 (300,'Wheat','wheat',230),
 (301,'Domesticated Wheat','wheat_domesticated',300),
 (312,'Wild Wheat','wheat_wild',300),
 (323,'Seeds','seeds',229),
 (324,'Vegetables','vegetable',323),
 (325,'Greens','vegetable_greens',324),
 (336,'Beans','vegetable_beans',324),
 (347,'Tubers','vegetable_tubers',324),
 (358,'Fungi','vegetable_fungi',324),
 (369,'Fruit','fruit',229),
 (370,'Fruits','fruit_fruits',369),
 (381,'Berries','fruit_berries',369),
 (392,'Flowers','fruit_flowers',369),
 (403,'Flora Structural','flora_structural',228),
 (404,'Wood','wood',403),
 (405,'Hard Wood','wood_deciduous',404),
 (416,'Soft Wood','softwood',404),
 (427,'Evergreen Soft Wood','softwood_evergreen',416),
 (438,'Inorganic','inorganic',1),
 (439,'Chemical','chemical',438),
 (440,'Liquid Petrochem Fuel','fuel_petrochem_liquid',439),
 (441,'Unknown Liquid Petrochem Fuel','fuel_petrochem_liquid_unknown',440),
 (442,'Known Liquid Petrochem Fuel','fuel_petrochem_liquid_known',440),
 (450,'Inert Petrochemical','petrochem_inert',439),
 (451,'Lubricating Oil','petrochem_inert_lubricating_oil',450),
 (452,'Polymer','petrochem_inert_polymer',450),
 (453,'Water','water',438),
 (464,'Mineral','mineral',438),
 (465,'Solid Petrochem Fuel','fuel_petrochem_solid',464),
 (466,'Unknown Solid Petrochem Fuel','fuel_petrochem_solid_unknown',465),
 (467,'Known Solid Petrochem Fuel','fuel_petrochem_solid_known',465),
 (475,'Radioactive','radioactive',464),
 (476,'Unknown Radioactive','radioactive_unknown',475),
 (477,'Known Radioactive','radioactive_known',475),
 (485,'Metal','metal',464),
 (486,'Ferrous Metal','metal_ferrous',485),
 (487,'Unknown Ferrous Metal','metal_ferrous_unknown',486),
 (488,'Steel','steel',486),
 (499,'Iron','iron',486),
 (508,'Non-Ferrous Metal','metal_nonferrous',485),
 (509,'Unknown Non-Ferrous Metal','metal_nonferrous_unknown',508),
 (510,'Aluminum','aluminum',508),
 (517,'Copper','copper',508),
 (527,'Low-Grade Ore','ore',464),
 (528,'Igneous Ore','ore_igneous',527),
 (529,'Unknown Igneous Ore','ore_igneous_unknown',528),
 (530,'Extrusive Ore','ore_extrusive',528),
 (541,'Intrusive Ore','ore_intrusive',528),
 (551,'Sedimentary Ore','ore_sedimentary',527),
 (552,'Unknown Sedimentary Ore','ore_sedimentary_unknown',551),
 (553,'Carbonate Ore','ore_carbonate',551),
 (562,'Siliclastic Ore','ore_siliclastic',551),
 (569,'Gemstone','gemstone',464),
 (570,'Unknown Gem Type','gemstone_unknown',569),
 (571,'Amorphous Gemstone','gemstone_amorphous',569),
 (582,'Crystalline Gemstone','gemstone_crystalline',569),
 (591,'Gas','gas',438),
 (592,'Reactive Gas','gas_reactive',591),
 (593,'Unknown Reactive Gas','gas_reactive_unknown',592),
 (594,'Known Reactive Gas','gas_reactive_known',592),
 (602,'Inert Gas','gas_inert',438),
 (603,'Unknown Inert Gas','gas_inert_unknown',602),
 (604,'Known Inert Gas','gas_inert_known',602),
 (618,'Energy','energy',1),
 (619,'Renewable Energy','energy_renewable',618),
 (620,'Site-Restricted Renewable Energy','energy_renewable_site_limited',619),
 (651,'Non Site-Restricted Renewable Energy','energy_renewable_unlimited',619),
 (671,'Fiberplast','fiberplast',439),
 (900,'Fish','seafood_fish',117),
 (901,'Crustacean','seafood_crustacean',117),
 (902,'Mollusk','seafood_mollusk',117),
 (903,'Wind Energy','energy_renewable_unlimited_wind',651),
 (904,'Solar Energy','energy_renewable_unlimited_solar',651),
 (905,'Homogenized Milk','homogenized_milk',5),
 (906,'Processed Meat','processed_meat',28),
 (914,'Mixed Vegetables','mixed_vegetables',324),
 (915,'Mixed Fruits','mixed_fruits',369),
 (916,'Chemical Compond','chemical_compond',439),
 (934,'Blended Wood','processed_wood',404),
 (950,'Processed Cereal','processed_cereal',230);
/*!40000 ALTER TABLE `resource_categories` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;