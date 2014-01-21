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


--
-- Create schema swganh
--

CREATE DATABASE IF NOT EXISTS swganh;
USE swganh;

--
-- Definition of table `character_appearance`
--

DROP TABLE IF EXISTS `character_appearance`;
CREATE TABLE `character_appearance` (
  `character_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `00FF` int(10) unsigned DEFAULT NULL COMMENT 'Starting Header',
  `01FF` int(10) unsigned DEFAULT NULL COMMENT 'Eye Color',
  `02FF` int(10) unsigned DEFAULT NULL,
  `03FF` int(10) unsigned DEFAULT NULL,
  `04FF` int(10) unsigned DEFAULT NULL COMMENT 'Muscle / Torso',
  `05FF` int(10) unsigned DEFAULT NULL COMMENT 'Weight',
  `06FF` int(10) unsigned DEFAULT NULL,
  `07FF` int(10) unsigned DEFAULT NULL COMMENT 'Eye Size',
  `08FF` int(10) unsigned DEFAULT NULL COMMENT 'Nose Protrusion',
  `09FF` int(10) unsigned DEFAULT NULL,
  `0AFF` int(10) unsigned DEFAULT NULL,
  `0BFF` int(10) unsigned DEFAULT NULL COMMENT 'Cheek Size',
  `0CFF` int(10) unsigned DEFAULT NULL COMMENT 'Eye Angle',
  `0DFF` int(10) unsigned DEFAULT NULL COMMENT 'Trunk Height',
  `0EFF` int(10) unsigned DEFAULT NULL,
  `0FFF` int(10) unsigned DEFAULT NULL COMMENT 'Nose Width',
  `10FF` int(10) unsigned DEFAULT NULL,
  `11FF` int(10) unsigned DEFAULT NULL,
  `12FF` int(10) unsigned DEFAULT NULL,
  `13FF` int(10) unsigned DEFAULT NULL COMMENT 'Jaw Size',
  `14FF` int(10) unsigned DEFAULT NULL COMMENT 'Skin Colour',
  `15FF` int(10) unsigned DEFAULT NULL COMMENT 'Mouth Size',
  `16FF` int(10) unsigned DEFAULT NULL,
  `17FF` int(10) unsigned DEFAULT NULL COMMENT 'Lip size',
  `18FF` int(10) unsigned DEFAULT NULL,
  `19FF` int(10) unsigned DEFAULT NULL COMMENT 'Ear Size',
  `1AFF` int(10) unsigned DEFAULT NULL,
  `1BFF` int(10) unsigned DEFAULT NULL,
  `1CFF` int(10) unsigned DEFAULT NULL COMMENT 'Chin / Gullet Size',
  `1DFF` int(10) unsigned DEFAULT NULL COMMENT 'Nose Color',
  `1EFF` int(10) unsigned DEFAULT NULL COMMENT 'Eyebrow Type',
  `1FFF` int(10) unsigned DEFAULT NULL COMMENT 'Facial Hair Color',
  `20FF` int(10) unsigned DEFAULT NULL,
  `21FF` int(10) unsigned DEFAULT NULL COMMENT 'Eye Shape',
  `22FF` int(10) unsigned DEFAULT NULL,
  `23FF` int(10) unsigned DEFAULT NULL COMMENT 'Facial Hair type',
  `24FF` int(10) unsigned DEFAULT NULL COMMENT 'Age',
  `25FF` int(10) unsigned DEFAULT NULL COMMENT 'Freckles',
  `26FF` int(10) unsigned DEFAULT NULL COMMENT 'Marking Pattern Type',
  `27FF` int(10) unsigned DEFAULT NULL COMMENT 'Sensor Size / Jowl',
  `28FF` int(10) unsigned DEFAULT NULL,
  `29FF` int(10) unsigned DEFAULT NULL,
  `2AFF` int(10) unsigned DEFAULT NULL,
  `2BFF` int(10) unsigned DEFAULT NULL,
  `2CFF` int(10) unsigned DEFAULT NULL COMMENT 'Eyeshadow Color',
  `2DFF` int(10) unsigned DEFAULT NULL COMMENT 'Eyeshadow',
  `2EFF` int(10) unsigned DEFAULT NULL COMMENT 'Lip Color',
  `2FFF` int(10) unsigned DEFAULT NULL,
  `30FF` int(10) unsigned DEFAULT NULL,
  `31FF` int(10) unsigned DEFAULT NULL,
  `32FF` int(10) unsigned DEFAULT NULL COMMENT 'Center Beard Color',
  `33FF` int(10) unsigned DEFAULT NULL,
  `34FF` int(10) unsigned DEFAULT NULL,
  `35FF` int(10) unsigned DEFAULT NULL,
  `36FF` int(10) unsigned DEFAULT NULL,
  `37FF` int(10) unsigned DEFAULT NULL,
  `38FF` int(10) unsigned DEFAULT NULL,
  `39FF` int(10) unsigned DEFAULT NULL COMMENT 'Head Size',
  `3AFF` int(10) unsigned DEFAULT NULL,
  `3BFF` int(10) unsigned DEFAULT NULL COMMENT 'Marking Pattern Color',
  `3CFF` int(10) unsigned DEFAULT NULL,
  `3DFF` int(10) unsigned DEFAULT NULL,
  `3EFF` int(10) unsigned DEFAULT NULL COMMENT 'Ear Shape',
  `3FFF` int(10) unsigned DEFAULT NULL,
  `40FF` int(10) unsigned DEFAULT NULL,
  `41FF` int(10) unsigned DEFAULT NULL,
  `42FF` int(10) unsigned DEFAULT NULL,
  `43FF` int(10) unsigned DEFAULT NULL COMMENT 'Brow',
  `44FF` int(10) unsigned DEFAULT NULL,
  `45FF` int(10) unsigned DEFAULT NULL,
  `46FF` int(10) unsigned DEFAULT NULL,
  `47FF` int(10) unsigned DEFAULT NULL,
  `48FF` int(10) unsigned DEFAULT NULL COMMENT 'Marking Color',
  `49FF` int(10) unsigned DEFAULT NULL COMMENT 'Fur Color',
  `4AFF` int(10) unsigned DEFAULT NULL,
  `4BFF` int(10) unsigned DEFAULT NULL,
  `4CFF` int(10) unsigned DEFAULT NULL,
  `4DFF` int(10) unsigned DEFAULT NULL,
  `4EFF` int(10) unsigned DEFAULT NULL COMMENT 'Chin Length',
  `4FFF` int(10) unsigned DEFAULT NULL,
  `50FF` int(10) unsigned DEFAULT NULL,
  `51FF` int(10) unsigned DEFAULT NULL,
  `52FF` int(10) unsigned DEFAULT NULL,
  `53FF` int(10) unsigned DEFAULT NULL,
  `54FF` int(10) unsigned DEFAULT NULL,
  `55FF` int(10) unsigned DEFAULT NULL,
  `56FF` int(10) unsigned DEFAULT NULL,
  `57FF` int(10) unsigned DEFAULT NULL,
  `58FF` int(10) unsigned DEFAULT NULL,
  `59FF` int(10) unsigned DEFAULT NULL,
  `5AFF` int(10) unsigned DEFAULT NULL,
  `5BFF` int(10) unsigned DEFAULT NULL,
  `5CFF` int(10) unsigned DEFAULT NULL,
  `5DFF` int(10) unsigned DEFAULT NULL,
  `5EFF` int(10) unsigned DEFAULT NULL,
  `5FFF` int(10) unsigned DEFAULT NULL,
  `60FF` int(10) unsigned DEFAULT NULL,
  `61FF` int(10) unsigned DEFAULT NULL,
  `62FF` int(10) unsigned DEFAULT NULL,
  `63FF` int(10) unsigned DEFAULT NULL COMMENT 'Markings Color (Sullastian)',
  `64FF` int(10) unsigned DEFAULT NULL,
  `65FF` int(10) unsigned DEFAULT NULL COMMENT 'Markings Pattern (Sullastian)',
  `66FF` int(10) unsigned DEFAULT NULL,
  `67FF` int(10) unsigned DEFAULT NULL,
  `68FF` int(10) unsigned DEFAULT NULL,
  `69FF` int(10) unsigned DEFAULT NULL,
  `6AFF` int(10) unsigned DEFAULT NULL,
  `6BFF` int(10) unsigned DEFAULT NULL,
  `6CFF` int(10) unsigned DEFAULT NULL,
  `6DFF` int(10) unsigned DEFAULT NULL,
  `6EFF` int(10) unsigned DEFAULT NULL COMMENT 'Center Beard Type (Ithorian)',
  `6FFF` int(10) unsigned DEFAULT NULL,
  `70FF` int(10) unsigned DEFAULT NULL COMMENT 'Side Beard Type (Ithorinan)',
  `ABFF` int(10) unsigned DEFAULT NULL COMMENT 'BoobJob',
  `AB2FF` int(10) unsigned DEFAULT NULL COMMENT 'BoobJob',
  `hair` char(128) DEFAULT NULL,
  `hair1` int(10) unsigned DEFAULT NULL COMMENT 'hair main color',
  `hair2` int(10) unsigned DEFAULT NULL COMMENT 'hair additional color',
  `base_model_string` char(128) DEFAULT NULL,
  `scale` float NOT NULL DEFAULT '1',
  PRIMARY KEY (`character_id`),
  CONSTRAINT `character_appearance_ibfk_1` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPRESSED;

--
-- Dumping data for table `character_appearance`
--

/*!40000 ALTER TABLE `character_appearance` DISABLE KEYS */;
/*!40000 ALTER TABLE `character_appearance` ENABLE KEYS */;




/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
