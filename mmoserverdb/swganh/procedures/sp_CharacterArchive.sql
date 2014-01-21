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
-- Definition of procedure `sp_CharacterArchive`
--

DROP PROCEDURE IF EXISTS `sp_CharacterArchive`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_CharacterArchive`(
	IN start_firstname CHAR(32)
	)
BEGIN
	
	DECLARE char_id BIGINT(20);
	DECLARE inventory_id BIGINT;
	DECLARE bank_id BIGINT;
	DECLARE datapad_id BIGINT;
	DECLARE loop_counter INT;
	DECLARE loop_count INT;
	DECLARE done INT;
	DECLARE curCopyWeapons CURSOR FOR SELECT * FROM swganh.weapons;
	DECLARE curCopyWearables CURSOR FOR SELECT * FROM swganh.wearables;
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET done = 1;
	DECLARE CONTINUE HANDLER FOR 1062 BEGIN END;
	
		
	SELECT id from swganh.characters WHERE firstname like start_firstname INTO char_id;
	
	SET inventory_id = char_id + 1;
	SET bank_id = char_id + 4;
	SET datapad_id = char_id + 3;

		
	INSERT INTO swganh_archive.characters
		SELECT * FROM swganh.characters WHERE firstname = start_firstname;
	
	INSERT INTO swganh_archive.banks
		SELECT * FROM swganh.banks WHERE id = bank_id;
		
	INSERT INTO swganh_archive.character_appearance
		SELECT * FROM swganh.character_appearance WHERE character_id = char_id;
		
	INSERT INTO swganh_archive.character_attributes
		SELECT * FROM swganh.character_attributes WHERE character_id = char_id;
	
	INSERT INTO swganh_archive.character_biography
		SELECT * FROM swganh.character_biography WHERE character_id = char_id;
		
	INSERT INTO swganh_archive.character_faction
		SELECT * FROM swganh.character_faction WHERE character_id = char_id;

	INSERT INTO swganh_archive.character_skills
		SELECT * FROM swganh.character_skills WHERE character_id = char_id;

	INSERT INTO swganh_archive.character_waypoints
		SELECT * FROM swganh.character_waypoints WHERE character_id = char_id;
		
	INSERT INTO swganh_archive.character_xp
		SELECT * FROM swganh.character_xp WHERE character_id = char_id;

	INSERT INTO swganh_archive.chat_friendlist
		SELECT * FROM swganh.chat_friendlist WHERE character_id = char_id;
		
	INSERT INTO swganh_archive.chat_ignorelist
		SELECT * FROM swganh.chat_ignorelist WHERE character_id = char_id;
		
	INSERT INTO swganh_archive.datapads
		SELECT * FROM swganh.datapads WHERE id = datapad_id;
		
	INSERT INTO swganh_archive.inventories
		SELECT * FROM swganh.inventories WHERE id = inventory_id;
		
	INSERT INTO swganh_archive.weapons
		SELECT * FROM swganh.weapons WHERE parent_id = inventory_id;
		
	INSERT INTO swganh_archive.wearables
		SELECT * FROM swganh.wearables WHERE parent_id = inventory_id;
		
	END $$
/*!50003 SET SESSION SQL_MODE=@TEMP_SQL_MODE */  $$

DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;