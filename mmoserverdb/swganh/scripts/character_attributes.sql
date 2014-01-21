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
-- Definition of table `character_attributes`
--
DROP TABLE IF EXISTS `character_attributes`;
CREATE TABLE `character_attributes` (
  `character_id` bigint(20) unsigned NOT NULL default '0' COMMENT 'Character - ID',
  `faction_id` int(8) unsigned NOT NULL default '0' COMMENT 'Character - Faction ID',
  `health_max` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Health Max',
  `strength_max` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Strength Max',
  `constitution_max` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Constitution Max',
  `action_max` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Action Max',
  `quickness_max` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Quickness Max',
  `stamina_max` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Stamina Max',
  `mind_max` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Mind Max',
  `focus_max` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Focus Max',
  `willpower_max` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Willpower Max',
  `health_current` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Health Fill',
  `strength_current` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Strength Fill',
  `constitution_current` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Constitution Fill',
  `action_current` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Action Fill',
  `quickness_current` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Quickness Fill',
  `stamina_current` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Stamina Fill',
  `mind_current` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Mind Fill',
  `focus_current` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Focus Fill',
  `willpower_current` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Willpower Fill',
  `health_wounds` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Health Wounds',
  `strength_wounds` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Strength Wounds',
  `constitution_wounds` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Constitution Wounds',
  `action_wounds` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Action Wounds',
  `quickness_wounds` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Quickness Wounds',
  `stamina_wounds` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Stamina wounds',
  `mind_wounds` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Mind wounds',
  `focus_wounds` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Focus Wounds',
  `willpower_wounds` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Willpower Wounds',
  `health_encum` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Health Encumberance',
  `action_encum` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Action Encumberance',
  `mind_encum` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Mind Encumberance',
  `battlefatigue` int(6) unsigned NOT NULL default '0' COMMENT 'Character - Battle Fatigue',
  `posture` int(3) unsigned NOT NULL default '0',
  `moodId` int(11) unsigned NOT NULL default '0',
  `title` char(255) default NULL,
  `character_flags` int(11) unsigned NOT NULL default '0',
  `states` bigint(20) unsigned NOT NULL default '0',
  `language` int(11) unsigned NOT NULL default '1',
  `group_id` bigint(20) unsigned default '0',
  `force_current` int(11) unsigned NOT NULL default '0',
  `force_max` int(11) unsigned NOT NULL default '0',
  `new_player_exemptions` int(2) unsigned NOT NULL default '3',
  PRIMARY KEY  (`character_id`),
  KEY `fk_charattributes_fac_fac` (`faction_id`),
  KEY `fk_charattributes_mood_mood` (`moodId`),
  CONSTRAINT `fk_charattributes_char_char` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_charattributes_fac_fac` FOREIGN KEY (`faction_id`) REFERENCES `faction` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_charattributes_mood_mood` FOREIGN KEY (`moodId`) REFERENCES `moods` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPRESSED;


--
-- Dumping data for table `character_attributes`
--

/*!40000 ALTER TABLE `character_attributes` DISABLE KEYS */;
/*!40000 ALTER TABLE `character_attributes` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;