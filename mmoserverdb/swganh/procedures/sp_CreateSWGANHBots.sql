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

use swganh;

--
-- Definition of procedure `sp_CreateSWGANHBots`
--

DROP PROCEDURE IF EXISTS `sp_CreateSWGANHBots`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,NO_AUTO_CREATE_USER' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_CreateSWGANHBots`(IN bots int, IN firstId int, IN start_city char(32))
BEGIN
	DECLARE new_account_id BIGINT(20);
	DECLARE new_username CHAR(16);
	DECLARE lastName CHAR(16);
	DECLARE new_userpass CHAR(128);
	DECLARE new_character_id BIGINT(22);
	DECLARE mod_username CHAR(32);
	DECLARE loop_counter int;
	DECLARE bot_id BIGINT(20);
	DECLARE loopEnd INT default 0;
	DECLARE first_bot_name CHAR(32);

	DECLARE gender_id INT(11);
	DECLARE profession_id INT(11);
	DECLARE species_id INT(11);
	DECLARE species CHAR(24);
	 
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET loopEnd = 1;

	SET new_username = 'bot#';
	SET bot_id = firstId;

	SET loop_counter = 0;

	IF start_city = '' THEN
		SET start_city = 'bestine';
	END IF;

START TRANSACTION;
	loop1: LOOP

	-- This will ensure a number of 0 bots will never be executed.
	IF loop_counter = bots THEN
		LEAVE loop1;
	END IF;

	SET mod_username = CONCAT(new_username, bot_id);
	SET new_userpass = CONCAT(mod_username, 'swganh');

	Call sp_AdminAddAccount(mod_username, new_userpass, 'bot@swganh.org');

  SELECT MAX(account_id) FROM account INTO new_account_id;
	
	-- To obtain a random integer R in the range i <= R < j, use the expression FLOOR(i + RAND() * (j  i)). 
	SELECT FLOOR(RAND() * (2)) INTO gender_id;
	SELECT FLOOR(RAND() * (6)) INTO profession_id;
	SELECT FLOOR(RAND() * (10)) INTO species_id;

	if species_id = 8 then
		SET species_id = 33;
	elseif species_id = 9 then
		SET species_id = 49;
  elseif species_id = 4 then
		SET species_id = 1;
	end if;
	
	SELECT name FROM race WHERE id=species_id INTO species;
	SELECT sf_GenerateLastName(species, gender_id) INTO lastName;
	
	if species_id = 0 then
		-- human
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 1.040251,NULL, 0, 25, 0, 511, 155, 176, 511, 150, 91, 83, 511, 511, 511, 511, 170, 210, 511, 95, 244, 511, 8, 511, 32, 14, 511, 154, 511, 151, 511, 0, 8, 13, 511, 37, 0, 0, 511, 1, 0, 0, 0, 0, 0, 0, 511, 1, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 511,'object/tangible/hair/human/hair_human_female_s01.iff',205,9,'object/creature/player/human_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 1.022160,NULL, 0, 28, 0, 511, 37, 207, 172, 234, 511, 8, 511, 511, 190, 511, 511, 511, 253, 153, 234, 511, 9, 219, 511, 174, 511, 511, 63, 511, 181, 0, 4, 4, 511, 49, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 511, 1, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511,'object/tangible/hair/human/hair_human_female_s19.iff',0,17,'object/creature/player/human_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.889012,NULL, 0, 11, 0, 511, 124, 182, 511, 511, 110, 29, 31, 190, 121, 511, 511, 511, 17, 511, 511, 196, 9, 208, 511, 50, 511, 511, 128, 511, 162, 0, 1, 6, 211, 511, 0, 0, 511, 2, 0, 0, 0, 0, 0, 0, 511, 1, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 244, 767,'object/tangible/hair/human/hair_human_female_s38.iff',0,3,'object/creature/player/human_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.922206,NULL, 0, 18, 0, 511, 213, 253, 511, 57, 140, 146, 511, 226, 21, 511, 511, 511, 132, 511, 511, 13, 19, 213, 511, 511, 32, 511, 34, 511, 129, 0, 6, 9, 43, 511, 0, 0, 511, 4, 0, 0, 0, 0, 0, 0, 511, 1, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 253, 767,'object/tangible/hair/human/hair_human_female_s21.iff',0,11,'object/creature/player/human_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.928459,NULL, 0, 30, 0, 511, 171, 66, 511, 141, 131, 511, 511, 251, 511, 13, 40, 511, 171, 511, 511, 40, 28, 511, 20, 511, 54, 9, 511, 159, 511, 0, 4, 1, 511, 219, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 511, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 132, 767,'object/tangible/hair/human/hair_human_female_s40.iff',0,13,'object/creature/player/human_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 1.041132,NULL, 0, 22, 0, 511, 96, 198, 511, 50, 214, 511, 511, 511, 221, 152, 511, 511, 232, 141, 511, 191, 46, 223, 511, 254, 511, 511, 16, 511, 197, 0, 8, 16, 511, 60, 0, 0, 511, 2, 0, 0, 0, 0, 0, 0, 511, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 139, 767,'object/tangible/hair/human/hair_human_female_s12.iff',511,1,'object/creature/player/human_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 1.041625,NULL, 0, 6, 0, 76, 139, 511, 137, 511, 511, 511, 132, 158, 511, 147, 63, 101, 511, 511, 228, 511, 19, 511, 100, 511, 150, 511, 134, 511, 111, 511, 1, 5, 511, 104, 0, 4, 511, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/human/hair_human_male_s07.iff',0,6,'object/creature/player/human_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.918876,NULL, 0, 25, 0, 511, 103, 35, 28, 208, 511, 142, 511, 20, 511, 511, 198, 511, 22, 511, 70, 511, 17, 511, 66, 58, 511, 137, 511, 511, 153, 511, 6, 2, 217, 511, 0, 17, 511, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/human/hair_human_male_s26.iff',0,18,'object/creature/player/human_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.077231,NULL, 0, 10, 0, 24, 111, 511, 220, 511, 511, 511, 81, 53, 511, 84, 178, 511, 139, 511, 511, 207, 10, 511, 53, 511, 207, 511, 52, 229, 511, 511, 5, 3, 511, 84, 0, 13, 511, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/human/hair_human_male_s11.iff',0,1,'object/creature/player/human_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.914940,NULL, 0, 24, 0, 511, 236, 58, 511, 511, 54, 152, 174, 63, 511, 511, 232, 62, 511, 511, 84, 511, 6, 231, 511, 205, 511, 10, 511, 197, 511, 511, 5, 1, 185, 511, 0, 15, 511, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/human/hair_human_male_s20.iff',0,6,'object/creature/player/human_male.iff');				
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 1.097356,NULL, 0, 8, 0, 511, 161, 200, 173, 16, 511, 511, 511, 221, 40, 209, 511, 103, 511, 511, 133, 511, 28, 511, 51, 176, 511, 511, 200, 511, 150, 511, 1, 19, 128, 511, 0, 4, 511, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/human/hair_human_male_s06.iff',0,17,'object/creature/player/human_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.920306,NULL, 0, 6, 0, 511, 52, 150, 511, 511, 28, 511, 76, 137, 69, 146, 511, 511, 136, 511, 511, 84, 6, 158, 511, 511, 49, 114, 511, 227, 511, 511, 6, 15, 511, 156, 0, 14, 511, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/human/hair_human_male_s08.iff',0,19,'object/creature/player/human_male.iff');
			end if;
		end if;	
		
	elseif species_id = 1 then
		-- rodian
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.802342,NULL, 0, 2, 0, 175, 16, 511, 124, 511, 511, 511, 22, 242, 511, 247, 36, 511, 152, 511, 210, 511, 19, 511, 58, 70, 511, 84, 511, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 160, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 95, 511,'object/tangible/hair/rodian/hair_rodian_female_s02.iff',0,2,'object/creature/player/rodian_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.807295,NULL, 0, 10, 0, 43, 13, 511, 128, 511, 511, 511, 171, 511, 511, 145, 244, 511, 190, 29, 115, 511, 7, 511, 201, 158, 511, 30, 511, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 87, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 511,'object/tangible/hair/rodian/hair_rodian_female_s07.iff',88,15,'object/creature/player/rodian_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.812929,NULL, 0, 1, 0, 511, 245, 96, 511, 511, 240, 151, 237, 143, 511, 511, 240, 16, 511, 511, 173, 511, 5, 511, 250, 511, 151, 171, 511, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 242, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 511,NULL,0,0,'object/creature/player/rodian_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.902751,NULL, 0, 511, 0, 41, 84, 511, 511, 511, 109, 41, 181, 511, 114, 511, 511, 511, 67, 238, 511, 120, 8, 511, 129, 511, 213, 511, 203, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 86, 511,'object/tangible/hair/rodian/hair_rodian_female_s02.iff',0,10,'object/creature/player/rodian_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.846534,NULL, 0, 11, 0, 48, 142, 511, 511, 511, 185, 511, 73, 511, 511, 62, 136, 511, 85, 219, 511, 158, 14, 34, 511, 511, 96, 511, 53, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 245, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 120, 767,'object/tangible/hair/rodian/hair_rodian_female_s09.iff',44,11,'object/creature/player/rodian_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.888265,NULL, 0, 8, 0, 511, 54, 26, 511, 50, 146, 511, 511, 511, 511, 6, 12, 511, 206, 145, 511, 23, 22, 511, 98, 96, 511, 17, 511, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 220, 767,'object/tangible/hair/rodian/hair_rodian_female_s16.iff',0,9,'object/creature/player/rodian_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.859198,NULL, 0, 1, 0, 28, 66, 511, 511, 511, 118, 34, 194, 210, 511, 511, 76, 511, 147, 511, 215, 511, 20, 242, 511, 511, 194, 46, 511, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 199, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/rodian/hair_rodian_male_s06.iff',15,0,'object/creature/player/rodian_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.939088,NULL, 0, 511, 0, 177, 218, 511, 231, 511, 511, 196, 118, 511, 65, 511, 511, 511, 73, 194, 98, 511, 511, 511, 127, 511, 128, 511, 10, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 78, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/rodian/hair_rodian_male_s16.iff',9,0,'object/creature/player/rodian_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.858261,NULL, 0, 4, 0, 511, 83, 214, 511, 511, 122, 511, 161, 511, 170, 50, 511, 511, 119, 169, 767, 511, 20, 33, 511, 97, 511, 108, 511, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 177, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/rodian_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.845958,NULL, 0, 11, 0, 196, 157, 511, 224, 511, 511, 511, 122, 140, 511, 165, 199, 38, 511, 511, 511, 248, 16, 194, 511, 48, 511, 511, 246, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/rodian/hair_rodian_male_s13.iff',6,0,'object/creature/player/rodian_male.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.813023,NULL, 0, 10, 0, 249, 147, 511, 9, 511, 511, 126, 242, 511, 511, 511, 208, 213, 511, 86, 25, 511, 8, 73, 511, 185, 511, 218, 511, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 154, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/rodian/hair_rodian_male_s11.iff',11,0,'object/creature/player/rodian_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.893823,NULL, 0, 10, 0, 62, 167, 511, 137, 71, 511, 2, 511, 511, 123, 511, 511, 511, 53, 241, 511, 122, 2, 156, 511, 74, 511, 511, 73, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 166, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/rodian/hair_rodian_male_s06.iff',7,0,'object/creature/player/rodian_male.iff');
			end if;
		end if;	

	elseif species_id = 2 then
		-- trandoshan
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 1.019599,NULL, 0, 21, 0, 178, 199, 511, 209, 155, 511, 213, 511, 511, 147, 511, 511, 511, 113, 28, 511, 65, 20, 0, 0, 0, 0, 0, 0, 511, 78, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 511,'object/tangible/hair/trandoshan/hair_trandoshan_female_s10.iff',511,0,'object/creature/player/trandoshan_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 1.033544,NULL, 0, 10, 0, 511, 48, 110, 511, 107, 183, 511, 511, 511, 511, 93, 205, 227, 511, 138, 511, 82, 9, 0, 0, 0, 0, 0, 0, 511, 171, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 511,NULL,0,0,'object/creature/player/trandoshan_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.162889,NULL, 0, 1, 0, 511, 75, 90, 241, 511, 511, 511, 66, 49, 147, 6, 511, 511, 151, 511, 511, 69, 8, 0, 0, 0, 0, 0, 0, 511, 176, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 511,NULL,0,0,'object/creature/player/trandoshan_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 1.148447,NULL, 0, 20, 0, 29, 175, 511, 153, 223, 511, 253, 511, 511, 511, 511, 207, 511, 157, 218, 511, 188, 4, 0, 0, 0, 0, 0, 0, 43, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 511,'object/tangible/hair/trandoshan/hair_trandoshan_female_s10.iff',511,0,'object/creature/player/trandoshan_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 1.041980,NULL, 0, 6, 0, 57, 181, 511, 68, 44, 511, 511, 511, 198, 191, 227, 511, 511, 196, 511, 221, 511, 5, 0, 0, 0, 0, 0, 0, 511, 163, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 511,NULL,0,0,'object/creature/player/trandoshan_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 1.210685,NULL, 0, 1, 0, 511, 15, 78, 5, 156, 511, 33, 511, 511, 44, 511, 511, 511, 27, 222, 197, 511, 14, 0, 0, 0, 0, 0, 0, 57, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 511,NULL,0,0,'object/creature/player/trandoshan_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 1.199556,NULL, 0, 23, 0, 511, 197, 126, 223, 237, 511, 511, 511, 511, 254, 248, 511, 194, 511, 11, 145, 511, 8, 0, 0, 0, 0, 0, 0, 511, 162, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/trandoshan/hair_trandoshan_male_s12.iff',511,0,'object/creature/player/trandoshan_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 1.047521,NULL, 0, 18, 0, 511, 93, 37, 511, 142, 111, 511, 511, 511, 219, 49, 511, 511, 109, 132, 63, 511, 12, 0, 0, 0, 0, 0, 0, 204, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/trandoshan/hair_trandoshan_male_s08.iff',511,0,'object/creature/player/trandoshan_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.061860,NULL, 0, 23, 0, 511, 167, 217, 177, 240, 511, 191, 511, 14, 86, 511, 511, 511, 169, 511, 154, 511, 6, 0, 0, 0, 0, 0, 0, 511, 141, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/trandoshan/hair_trandoshan_male_s01.iff',511,0,'object/creature/player/trandoshan_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 1.196794,NULL, 0, 20, 0, 114, 171, 511, 511, 511, 143, 106, 113, 66, 511, 511, 69, 190, 511, 511, 165, 511, 16, 0, 0, 0, 0, 0, 0, 2, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/trandoshan/hair_trandoshan_male_s07.iff',511,0,'object/creature/player/trandoshan_male.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 1.053550,NULL, 0, 13, 0, 32, 158, 511, 21, 511, 511, 249, 126, 511, 511, 511, 86, 511, 100, 80, 511, 206, 11, 0, 0, 0, 0, 0, 0, 511, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/trandoshan/hair_trandoshan_male_s08.iff',511,0,'object/creature/player/trandoshan_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 1.095517,NULL, 0, 8, 0, 511, 100, 111, 511, 102, 13, 209, 511, 115, 223, 511, 511, 93, 511, 511, 511, 84, 19, 0, 0, 0, 0, 0, 0, 32, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/trandoshan/hair_trandoshan_male_s12.iff',511,0,'object/creature/player/trandoshan_male.iff');
			end if;
		end if;	

	elseif species_id = 3 then
		-- moncal
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.903453,NULL, 0, 11, 0, 101, 90, 511, 225, 511, 511, 511, 32, 76, 132, 125, 511, 10, 511, 511, 132, 511, 22, 112, 511, 22, 511, 0, 0, 136, 511, 8, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 0,NULL,0,0,'object/creature/player/moncal_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.937025,NULL, 0, 1, 0, 511, 142, 171, 511, 511, 173, 104, 209, 511, 82, 511, 511, 511, 134, 101, 238, 511, 19, 80, 511, 101, 511, 0, 0, 83, 511, 17, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 189, 0, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 0,NULL,0,0,'object/creature/player/moncal_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.870102,NULL, 0, 18, 0, 511, 87, 248, 231, 511, 511, 237, 168, 511, 511, 511, 212, 243, 511, 225, 511, 151, 1, 511, 109, 119, 511, 0, 0, 511, 224, 24, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 174, 0, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 0,NULL,0,0,'object/creature/player/moncal_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.867234,NULL, 0, 11, 0, 511, 8, 182, 166, 511, 511, 511, 232, 511, 511, 17, 30, 511, 174, 212, 511, 166, 25, 511, 135, 511, 78, 0, 0, 69, 511, 17, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 29, 0, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 0,NULL,0,0,'object/creature/player/moncal_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.887533,NULL, 0, 7, 0, 511, 46, 220, 511, 44, 241, 511, 511, 85, 210, 198, 511, 172, 511, 511, 214, 511, 23, 10, 511, 511, 222, 0, 0, 5, 511, 13, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 148, 0, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 0,NULL,0,0,'object/creature/player/moncal_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.911303,NULL, 0, 2, 0, 207, 82, 511, 5, 511, 511, 511, 17, 511, 511, 132, 161, 127, 511, 77, 511, 219, 25, 511, 194, 76, 511, 0, 0, 511, 229, 15, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 33, 511, 0, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 767, 0,NULL,0,0,'object/creature/player/moncal_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.949440,NULL, 0, 1, 0, 511, 34, 149, 173, 511, 511, 511, 167, 511, 81, 223, 511, 32, 511, 68, 511, 12, 23, 248, 511, 125, 511, 511, 171, 511, 171, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/moncal_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.980095,NULL, 0, 511, 0, 511, 16, 64, 151, 246, 511, 511, 511, 14, 511, 176, 55, 184, 511, 511, 174, 511, 20, 14, 511, 511, 120, 83, 511, 511, 178, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/moncal_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.897769,NULL, 0, 16, 0, 511, 52, 167, 511, 511, 180, 175, 248, 511, 208, 511, 511, 511, 42, 29, 511, 97, 16, 22, 511, 67, 511, 169, 511, 162, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/moncal_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.977357,NULL, 0, 3, 0, 215, 61, 511, 511, 130, 230, 511, 511, 26, 102, 23, 511, 87, 511, 511, 511, 143, 25, 81, 511, 511, 116, 511, 229, 42, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/moncal_male.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.939732,NULL, 0, 511, 0, 511, 34, 47, 511, 175, 104, 185, 511, 72, 20, 511, 511, 511, 77, 511, 200, 511, 2, 33, 511, 511, 182, 511, 13, 89, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/moncal_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.988387,NULL, 0, 9, 0, 237, 98, 511, 151, 511, 511, 511, 85, 511, 511, 1, 145, 511, 126, 225, 210, 511, 16, 246, 511, 511, 2, 511, 93, 175, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/moncal_male.iff');
			end if;
		end if;	

	elseif species_id = 4 then
		-- wookiee
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 1.230734,NULL, 0, 11, 0, 110, 250, 511, 511, 202, 511, 511, 511, 150, 511, 9, 238, 151, 511, 511, 138, 511, 0, 511, 233, 0, 0, 0, 0, 53, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 511, 39, 511, 0, 0, 0, 0, 0, 0, 0, 231, 511, 0, 0, 0, 0, 17, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 1.106635,NULL, 0, 21, 0, 511, 16, 139, 511, 226, 511, 511, 511, 241, 204, 9, 511, 23, 511, 511, 511, 49, 0, 511, 215, 0, 0, 0, 0, 511, 197, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 194, 0, 0, 0, 0, 0, 0, 0, 219, 511, 0, 0, 0, 0, 13, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.130666,NULL, 0, 25, 0, 211, 65, 511, 511, 511, 511, 120, 75, 511, 48, 511, 511, 511, 88, 211, 511, 204, 0, 511, 118, 0, 0, 0, 0, 511, 223, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 148, 0, 0, 0, 0, 0, 0, 0, 160, 511, 0, 0, 0, 0, 10, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 1.199944,NULL, 0, 13, 0, 511, 223, 234, 511, 202, 511, 511, 511, 511, 206, 157, 511, 106, 511, 49, 130, 511, 0, 511, 35, 0, 0, 0, 0, 108, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 81, 0, 0, 0, 0, 0, 0, 0, 511, 246, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 1.152774,NULL, 0, 8, 0, 124, 160, 511, 511, 511, 511, 511, 80, 511, 228, 208, 511, 101, 511, 38, 511, 240, 0, 511, 138, 0, 0, 0, 0, 162, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 511, 68, 511, 0, 0, 0, 0, 0, 0, 0, 511, 192, 0, 0, 0, 0, 10, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 1.154094,NULL, 0, 14, 0, 28, 174, 511, 511, 511, 511, 511, 215, 511, 511, 1, 68, 511, 82, 182, 511, 112, 0, 511, 73, 0, 0, 0, 0, 511, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 511, 22, 511, 0, 0, 0, 0, 0, 0, 0, 511, 126, 0, 0, 0, 0, 19, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 1.172125,NULL, 0, 4, 0, 511, 174, 36, 511, 100, 511, 511, 511, 511, 86, 55, 511, 232, 511, 243, 162, 511, 0, 511, 174, 0, 0, 0, 0, 229, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 511, 193, 511, 0, 0, 0, 0, 0, 0, 0, 511, 85, 0, 0, 0, 0, 9, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 1.217961,NULL, 0, 24, 0, 511, 4, 169, 511, 13, 511, 511, 511, 221, 231, 63, 511, 252, 511, 511, 56, 511, 0, 511, 51, 0, 0, 0, 0, 238, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 511, 247, 511, 0, 0, 0, 0, 0, 0, 0, 511, 197, 0, 0, 0, 0, 14, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.228479,NULL, 0, 5, 0, 511, 92, 20, 511, 115, 511, 150, 511, 511, 511, 511, 111, 106, 511, 55, 511, 152, 0, 511, 67, 0, 0, 0, 0, 157, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 24, 0, 0, 0, 0, 0, 0, 0, 511, 63, 0, 0, 0, 0, 15, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 1.202808,NULL, 0, 19, 0, 65, 767, 511, 511, 511, 511, 511, 239, 511, 134, 230, 511, 511, 242, 159, 511, 96, 0, 86, 511, 0, 0, 0, 0, 511, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 511, 116, 511, 0, 0, 0, 0, 0, 0, 0, 511, 120, 0, 0, 0, 0, 511, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_male.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 1.146574,NULL, 0, 2, 0, 511, 160, 27, 511, 511, 511, 511, 60, 511, 511, 72, 47, 71, 511, 228, 511, 71, 0, 511, 149, 0, 0, 0, 0, 149, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 209, 0, 0, 0, 0, 0, 0, 0, 60, 511, 0, 0, 0, 0, 5, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 1.118991,NULL, 0, 20, 0, 511, 240, 8, 511, 63, 511, 511, 511, 511, 119, 84, 511, 511, 63, 125, 511, 79, 0, 178, 511, 0, 0, 0, 0, 46, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 511, 156, 511, 0, 0, 0, 0, 0, 0, 0, 511, 38, 0, 0, 0, 0, 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/wookiee_male.iff');
			end if;
		end if;	
		
	elseif species_id = 5 then
		-- bothan
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.725962,NULL, 0, 29, 0, 511, 248, 196, 233, 128, 511, 191, 511, 179, 511, 511, 121, 38, 511, 511, 62, 511, 15, 104, 511, 511, 30, 115, 511, 511, 133, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 105, 767,NULL,0,0,'object/creature/player/bothan_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.735649,NULL, 0, 27, 0, 511, 249, 150, 251, 49, 511, 511, 511, 511, 511, 131, 239, 74, 511, 11, 511, 175, 18, 511, 245, 48, 511, 511, 75, 246, 511, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 75, 511,'object/tangible/hair/bothan/hair_bothan_female_s08.iff',0,14,'object/creature/player/bothan_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.738729,NULL, 0, 33, 0, 511, 170, 87, 511, 231, 97, 116, 511, 190, 95, 511, 511, 511, 227, 511, 38, 511, 35, 511, 140, 44, 511, 511, 46, 511, 66, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 161, 767,'object/tangible/hair/bothan/hair_bothan_female_s04.iff',511,48,'object/creature/player/bothan_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.781165,NULL, 0, 14, 0, 511, 4, 118, 511, 511, 91, 511, 79, 134, 183, 511, 511, 193, 511, 511, 216, 511, 44, 511, 78, 511, 159, 101, 511, 511, 176, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 180, 767,'object/tangible/hair/bothan/hair_bothan_female_s06.iff',9,51,'object/creature/player/bothan_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.776407,NULL, 0, 19, 0, 154, 179, 511, 48, 185, 511, 207, 511, 511, 511, 511, 92, 103, 511, 231, 511, 11, 46, 107, 511, 511, 37, 12, 511, 511, 216, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 112, 767,'object/tangible/hair/bothan/hair_bothan_female_s11.iff',511,36,'object/creature/player/bothan_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.747412,NULL, 0, 3, 0, 151, 43, 511, 213, 511, 511, 511, 203, 33, 13, 230, 511, 135, 511, 511, 511, 249, 62, 126, 511, 511, 216, 159, 511, 253, 511, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 137, 767,'object/tangible/hair/bothan/hair_bothan_female_s12.iff',38,21,'object/creature/player/bothan_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.753880,NULL, 0, 12, 0, 28, 77, 511, 239, 511, 511, 226, 215, 133, 511, 511, 109, 511, 224, 511, 247, 511, 24, 129, 511, 511, 28, 47, 511, 160, 511, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/bothan/hair_bothan_male_s01.iff',0,32,'object/creature/player/bothan_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.753064,NULL, 0, 1, 0, 202, 193, 511, 511, 248, 149, 511, 511, 511, 511, 135, 48, 511, 106, 229, 241, 511, 13, 79, 511, 511, 71, 511, 196, 511, 179, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/bothan/hair_bothan_male_s02.iff',0,13,'object/creature/player/bothan_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.812928,NULL, 0, 19, 0, 149, 12, 511, 63, 34, 511, 511, 511, 511, 214, 228, 511, 511, 124, 248, 511, 206, 60, 511, 246, 511, 80, 511, 223, 511, 91, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/bothan/hair_bothan_male_s02.iff',0,43,'object/creature/player/bothan_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.799411,NULL, 0, 28, 0, 75, 121, 511, 197, 198, 511, 511, 511, 511, 2, 44, 511, 241, 511, 63, 511, 68, 31, 132, 511, 75, 511, 511, 84, 511, 170, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/bothan/hair_bothan_male_s08.iff',0,24,'object/creature/player/bothan_male.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.799821,NULL, 0, 32, 0, 511, 51, 216, 511, 511, 191, 511, 42, 511, 133, 15, 511, 511, 211, 154, 85, 511, 22, 228, 511, 511, 122, 511, 133, 205, 511, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/bothan/hair_bothan_male_s20.iff',90,16,'object/creature/player/bothan_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.822122,NULL, 0, 511, 0, 511, 57, 38, 511, 100, 89, 87, 511, 511, 511, 511, 111, 150, 511, 58, 511, 114, 4, 79, 511, 511, 76, 215, 511, 511, 135, 0, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/bothan/hair_bothan_male_s20.iff',53,21,'object/creature/player/bothan_male.iff');
			end if;
		end if;	
		
	elseif species_id = 6 then
		-- twilek
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 1.001847,NULL, 0, 11, 0, 511, 107, 102, 511, 511, 130, 109, 195, 511, 214, 511, 511, 99, 511, 69, 206, 511, 26, 117, 511, 511, 178, 511, 45, 511, 122, 0, 4, 0, 511, 106, 511, 0, 511, 4, 0, 0, 0, 0, 0, 0, 511, 511, 511, 18, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 118, 511,'object/tangible/hair/twilek/hair_twilek_female_s05.iff',511,0,'object/creature/player/twilek_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.990952,NULL, 0, 12, 0, 511, 19, 246, 511, 5, 217, 511, 511, 225, 511, 204, 181, 511, 31, 511, 182, 511, 35, 511, 249, 29, 511, 511, 145, 157, 511, 0, 5, 0, 511, 218, 511, 0, 511, 511, 0, 0, 0, 0, 0, 0, 511, 511, 511, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 161, 767,NULL,0,0,'object/creature/player/twilek_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.048207,NULL, 0, 23, 0, 30, 62, 511, 70, 511, 511, 133, 30, 251, 511, 511, 219, 511, 96, 511, 149, 511, 23, 511, 99, 511, 249, 131, 511, 89, 511, 0, 6, 0, 61, 511, 511, 0, 511, 3, 0, 0, 0, 0, 0, 0, 511, 511, 511, 19, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 137, 767,'object/tangible/hair/twilek/hair_twilek_female_s05.iff',511,0,'object/creature/player/twilek_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.961671,NULL, 0, 14, 0, 128, 41, 511, 511, 511, 249, 39, 235, 511, 161, 511, 511, 221, 511, 247, 139, 511, 2, 511, 63, 511, 138, 511, 49, 38, 511, 0, 6, 0, 511, 141, 511, 0, 511, 1, 0, 0, 0, 0, 0, 0, 511, 1, 511, 1, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 251, 767,'object/tangible/hair/twilek/hair_twilek_female_s07.iff',511,0,'object/creature/player/twilek_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.905970,NULL, 0, 511, 0, 72, 158, 511, 12, 511, 511, 511, 147, 130, 511, 47, 240, 38, 511, 511, 208, 511, 6, 126, 511, 511, 221, 111, 511, 511, 12, 0, 2, 0, 198, 511, 511, 0, 511, 4, 0, 0, 0, 0, 0, 0, 511, 1, 511, 2, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 511,'object/tangible/hair/twilek/hair_twilek_female_s05.iff',511,0,'object/creature/player/twilek_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.994034,NULL, 0, 26, 0, 511, 184, 6, 511, 511, 79, 113, 180, 148, 511, 511, 211, 511, 212, 511, 225, 511, 15, 67, 511, 511, 163, 140, 511, 511, 767, 0, 8, 0, 132, 511, 511, 0, 511, 4, 0, 0, 0, 0, 0, 0, 511, 1, 511, 15, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 511,'object/tangible/hair/twilek/hair_twilek_female_s02.iff',511,0,'object/creature/player/twilek_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.997421,NULL, 0, 5, 0, 511, 73, 113, 171, 143, 511, 511, 511, 511, 511, 250, 93, 219, 511, 56, 186, 511, 19, 511, 38, 511, 217, 19, 511, 140, 511, 0, 0, 0, 165, 511, 511, 0, 511, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/twilek/hair_twilek_male_s05.iff',511,0,'object/creature/player/twilek_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 1.074934,NULL, 0, 18, 0, 130, 104, 511, 511, 511, 93, 80, 28, 511, 511, 511, 174, 168, 511, 192, 511, 225, 7, 511, 235, 59, 511, 511, 168, 511, 10, 0, 0, 0, 15, 511, 511, 0, 511, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/twilek/hair_twilek_male_s02.iff',511,0,'object/creature/player/twilek_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.014987,NULL, 0, 3, 0, 59, 17, 511, 511, 511, 39, 23, 131, 511, 48, 511, 511, 138, 511, 232, 15, 511, 14, 47, 511, 18, 511, 511, 185, 511, 230, 0, 0, 0, 511, 31, 511, 0, 511, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/twilek_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 1.108782,NULL, 0, 23, 0, 120, 45, 511, 511, 511, 122, 511, 104, 103, 122, 134, 511, 511, 24, 511, 511, 128, 5, 511, 131, 511, 242, 139, 511, 125, 511, 0, 0, 0, 234, 511, 511, 0, 511, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/twilek/hair_twilek_male_s06.iff',511,0,'object/creature/player/twilek_male.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 1.103056,NULL, 0, 26, 0, 511, 249, 80, 155, 511, 511, 511, 148, 35, 65, 44, 511, 511, 86, 511, 511, 135, 5, 211, 511, 511, 18, 28, 511, 43, 511, 0, 0, 0, 511, 209, 511, 0, 511, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/twilek/hair_twilek_male_s03.iff',511,0,'object/creature/player/twilek_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 1.090009,NULL, 0, 23, 0, 156, 127, 511, 511, 233, 58, 162, 511, 83, 511, 511, 249, 511, 40, 511, 9, 511, 31, 511, 174, 245, 511, 223, 511, 104, 511, 0, 0, 0, 25, 511, 511, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/twilek/hair_twilek_male_s05.iff',511,0,'object/creature/player/twilek_male.iff');
			end if;
		end if;	
		
	elseif species_id = 7 then
		-- zabrak
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.916444,NULL, 0, 4, 0, 131, 42, 511, 93, 220, 511, 168, 511, 244, 61, 511, 511, 511, 219, 511, 511, 194, 26, 77, 511, 136, 511, 511, 226, 22, 511, 0, 0, 0, 511, 239, 511, 0, 511, 4, 0, 0, 0, 0, 511, 0, 511, 511, 511, 0, 0, 0, 0, 0, 0, 17, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 199, 767,'object/tangible/hair/zabrak/hair_zabrak_female_s12.iff',511,0,'object/creature/player/zabrak_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.922356,NULL, 0, 4, 0, 511, 767, 44, 167, 196, 511, 48, 511, 181, 511, 511, 112, 511, 159, 511, 194, 511, 37, 511, 30, 122, 511, 67, 511, 221, 511, 0, 0, 0, 198, 511, 511, 0, 511, 3, 0, 0, 0, 0, 511, 0, 511, 511, 511, 0, 0, 0, 0, 0, 0, 7, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 105, 511,'object/tangible/hair/zabrak/hair_zabrak_female_s10.iff',2,0,'object/creature/player/zabrak_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.897140,NULL, 0, 14, 0, 8, 223, 511, 511, 4, 195, 227, 511, 511, 39, 511, 511, 511, 44, 178, 511, 140, 45, 19, 511, 511, 217, 59, 511, 511, 49, 0, 0, 0, 130, 511, 511, 0, 511, 2, 0, 0, 0, 0, 511, 0, 511, 511, 511, 0, 0, 0, 0, 0, 0, 15, 62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 146, 767,'object/tangible/hair/zabrak/hair_zabrak_female_s05.iff',1,0,'object/creature/player/zabrak_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.990041,NULL, 0, 2, 0, 121, 172, 511, 248, 104, 511, 511, 511, 15, 511, 67, 104, 210, 511, 511, 59, 511, 37, 184, 511, 79, 511, 132, 511, 511, 156, 0, 0, 0, 511, 208, 511, 0, 511, 511, 0, 0, 0, 0, 511, 0, 511, 511, 511, 0, 0, 0, 0, 0, 0, 14, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 105, 511,'object/tangible/hair/zabrak/hair_zabrak_female_s06.iff',3,0,'object/creature/player/zabrak_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.978565,NULL, 0, 15, 0, 511, 16, 88, 511, 48, 180, 34, 511, 511, 149, 511, 511, 167, 511, 247, 511, 96, 47, 511, 54, 511, 91, 83, 511, 114, 511, 0, 0, 0, 181, 511, 511, 0, 511, 3, 0, 0, 0, 0, 511, 0, 511, 511, 511, 0, 0, 0, 0, 0, 0, 14, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 511,'object/tangible/hair/zabrak/hair_zabrak_female_s07.iff',511,0,'object/creature/player/zabrak_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.976492,NULL, 0, 5, 0, 151, 141, 511, 239, 110, 511, 243, 511, 511, 511, 511, 126, 222, 511, 53, 43, 511, 6, 511, 138, 511, 167, 511, 71, 511, 64, 0, 0, 0, 511, 61, 511, 0, 511, 2, 0, 0, 0, 0, 511, 0, 511, 1, 511, 0, 0, 0, 0, 0, 0, 15, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 134, 767,'object/tangible/hair/zabrak/hair_zabrak_female_s06.iff',3,0,'object/creature/player/zabrak_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.955305,NULL, 0, 13, 0, 511, 245, 111, 101, 238, 511, 511, 511, 511, 511, 129, 54, 181, 511, 165, 511, 229, 20, 128, 511, 14, 511, 511, 139, 511, 251, 0, 0, 0, 195, 511, 511, 0, 511, 1, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/zabrak/hair_zabrak_male_s01.iff',3,0,'object/creature/player/zabrak_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 1.025404,NULL, 0, 3, 0, 511, 34, 63, 511, 511, 56, 121, 137, 511, 132, 511, 511, 511, 156, 66, 187, 511, 32, 42, 511, 511, 161, 511, 74, 217, 511, 0, 0, 0, 511, 128, 511, 0, 511, 1, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/zabrak/hair_zabrak_male_s09.iff',511,0,'object/creature/player/zabrak_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.030398,NULL, 0, 13, 0, 511, 236, 46, 511, 511, 78, 511, 216, 511, 235, 78, 511, 511, 228, 36, 511, 122, 17, 511, 112, 511, 98, 511, 203, 511, 119, 0, 0, 0, 511, 151, 511, 0, 511, 511, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/zabrak/hair_zabrak_male_s02.iff',3,0,'object/creature/player/zabrak_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 1.004164,NULL, 0, 10, 0, 139, 232, 511, 511, 511, 33, 149, 110, 511, 511, 511, 85, 192, 511, 14, 511, 18, 5, 185, 511, 83, 511, 78, 511, 114, 511, 0, 0, 0, 51, 511, 511, 0, 511, 2, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/zabrak/hair_zabrak_male_s05.iff',2,0,'object/creature/player/zabrak_male.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 1.049624,NULL, 0, 3, 0, 88, 44, 511, 511, 511, 247, 105, 137, 511, 200, 511, 511, 36, 511, 166, 30, 511, 25, 198, 511, 511, 40, 511, 13, 126, 511, 0, 0, 0, 511, 158, 511, 0, 511, 2, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/zabrak/hair_zabrak_male_s01.iff',2,0,'object/creature/player/zabrak_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.987509,NULL, 0, 4, 0, 157, 124, 511, 511, 511, 193, 63, 199, 511, 143, 511, 511, 198, 511, 158, 161, 511, 40, 511, 52, 511, 218, 81, 511, 63, 511, 0, 0, 0, 511, 43, 511, 0, 511, 2, 0, 0, 0, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'object/tangible/hair/zabrak/hair_zabrak_male_s09.iff',3,0,'object/creature/player/zabrak_male.iff');
			end if;
		end if;	
		
	elseif species_id = 33 then
		-- ithorian
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.932384,NULL, 0, 0, 0, 23, 143, 511, 511, 230, 180, 169, 511, 511, 122, 511, 511, 0, 0, 239, 222, 511, 11, 511, 149, 511, 150, 0, 0, 8, 511, 18, 0, 0, 138, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 26, 31, 511, 0, 0, 0, 0, 84, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 3, 4, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/ithorian_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.914971,NULL, 0, 0, 0, 90, 41, 511, 511, 19, 126, 198, 511, 511, 176, 511, 511, 0, 0, 146, 511, 44, 5, 238, 511, 511, 28, 0, 0, 7, 511, 25, 0, 0, 511, 126, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 35, 196, 511, 0, 0, 0, 0, 231, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 4, 6, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/ithorian_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.923745,NULL, 0, 0, 0, 511, 7, 5, 68, 225, 511, 511, 511, 61, 60, 175, 511, 0, 0, 511, 249, 511, 21, 188, 511, 12, 511, 0, 0, 3, 511, 511, 0, 0, 247, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 18, 54, 511, 0, 0, 0, 0, 511, 225, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 11, 1, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/ithorian_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.976468,NULL, 0, 0, 0, 179, 16, 511, 137, 511, 511, 511, 63, 511, 68, 108, 511, 0, 0, 229, 78, 511, 511, 511, 171, 96, 511, 0, 0, 11, 511, 14, 0, 0, 511, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 0, 6, 26, 511, 0, 0, 0, 0, 511, 154, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 11, 3, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/ithorian_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.988579,NULL, 0, 0, 0, 511, 36, 74, 511, 74, 10, 511, 511, 136, 216, 76, 511, 0, 0, 511, 213, 511, 29, 511, 244, 511, 203, 0, 0, 511, 97, 25, 0, 0, 511, 117, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 26, 511, 235, 0, 0, 0, 0, 511, 178, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 11, 5, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/ithorian_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 1.012185,NULL, 0, 0, 0, 511, 90, 136, 511, 225, 115, 511, 511, 74, 121, 78, 511, 0, 0, 511, 243, 511, 26, 511, 36, 44, 511, 0, 0, 37, 511, 29, 0, 0, 511, 132, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 15, 511, 41, 0, 0, 0, 0, 511, 203, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 5, 6, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/ithorian_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 1.042303,NULL, 0, 0, 0, 71, 41, 511, 511, 511, 192, 66, 202, 115, 511, 511, 32, 0, 0, 511, 511, 109, 21, 179, 511, 188, 511, 0, 0, 511, 115, 16, 0, 0, 201, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 18, 141, 511, 0, 0, 0, 0, 511, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 511, 0, 1, 0, 0,NULL,0,0,'object/creature/player/ithorian_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.931314,NULL, 0, 0, 0, 169, 243, 511, 239, 511, 511, 70, 189, 206, 60, 511, 511, 0, 0, 511, 72, 511, 16, 147, 511, 511, 30, 0, 0, 250, 511, 29, 0, 0, 95, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 8, 511, 119, 0, 0, 0, 0, 134, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 3, 0, 1, 0, 0,NULL,0,0,'object/creature/player/ithorian_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.026153,NULL, 0, 0, 0, 511, 111, 121, 511, 511, 106, 254, 94, 168, 87, 511, 511, 0, 0, 511, 218, 511, 32, 511, 69, 511, 125, 0, 0, 119, 511, 3, 0, 0, 511, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 37, 0, 14, 30, 511, 0, 0, 0, 0, 511, 171, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 4, 0, 7, 0, 0,NULL,0,0,'object/creature/player/ithorian_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 1.004691,NULL, 0, 0, 0, 246, 123, 511, 511, 246, 74, 511, 511, 54, 138, 69, 511, 0, 0, 511, 148, 511, 27, 25, 511, 138, 511, 0, 0, 92, 511, 9, 0, 0, 511, 202, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 13, 139, 511, 0, 0, 0, 0, 78, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 0, 2, 0, 7, 0, 0,NULL,0,0,'object/creature/player/ithorian_male.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.934483,NULL, 0, 0, 0, 511, 154, 60, 511, 511, 118, 100, 112, 173, 75, 511, 511, 0, 0, 511, 112, 511, 12, 511, 114, 511, 222, 0, 0, 139, 511, 16, 0, 0, 511, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 24, 511, 230, 0, 0, 0, 0, 511, 157, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 4, 0, 511, 0, 0,NULL,0,0,'object/creature/player/ithorian_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 1.059221,NULL, 0, 0, 0, 511, 20, 70, 167, 157, 511, 80, 511, 511, 511, 511, 20, 0, 0, 85, 511, 198, 25, 97, 511, 511, 36, 0, 0, 250, 511, 18, 0, 0, 89, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 0, 31, 511, 165, 0, 0, 0, 0, 511, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0,NULL,0,0,'object/creature/player/ithorian_male.iff');
			end if;
		end if;	
		
	elseif species_id = 49 then
		-- sullustan
		if gender_id = 0 then
			-- female
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 0.946259,NULL, 0, 0, 0, 511, 144, 196, 0, 511, 0, 511, 192, 511, 0, 231, 0, 58, 511, 239, 97, 511, 3, 199, 511, 41, 511, 511, 78, 242, 511, 0, 0, 0, 6, 511, 0, 0, 511, 1, 511, 72, 511, 0, 0, 0, 511, 4, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 42, 0, 511, 159, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 132, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 18, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 26, 0,NULL,0,0,'object/creature/player/sullustan_female.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 0.989145,NULL, 0, 0, 0, 81, 122, 511, 0, 511, 0, 6, 69, 511, 0, 511, 0, 146, 511, 150, 202, 511, 41, 27, 511, 511, 177, 228, 511, 154, 511, 0, 0, 0, 250, 511, 0, 0, 511, 2, 511, 511, 242, 0, 0, 0, 511, 2, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 20, 0, 511, 1, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 4, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 210, 0,NULL,0,0,'object/creature/player/sullustan_female.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 0.928251,NULL, 0, 0, 0, 511, 28, 123, 0, 10, 0, 511, 511, 109, 0, 171, 0, 511, 159, 511, 511, 87, 21, 511, 94, 76, 511, 83, 511, 511, 24, 0, 0, 0, 105, 511, 0, 0, 511, 4, 511, 18, 511, 0, 0, 0, 511, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 29, 0, 31, 511, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 78, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41, 0, 18, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 149, 0,NULL,0,0,'object/creature/player/sullustan_female.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 0.990826,NULL, 0, 0, 0, 110, 188, 511, 0, 186, 0, 58, 511, 65, 0, 511, 0, 64, 511, 511, 511, 92, 13, 48, 511, 511, 50, 174, 511, 56, 511, 0, 0, 0, 511, 15, 0, 0, 511, 511, 511, 511, 115, 0, 0, 0, 511, 1, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 39, 0, 35, 511, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 213, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55, 0, 8, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 246, 0,NULL,0,0,'object/creature/player/sullustan_female.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.954236,NULL, 0, 0, 0, 163, 188, 511, 0, 224, 0, 511, 511, 109, 0, 196, 0, 511, 90, 511, 229, 511, 33, 511, 171, 104, 511, 17, 511, 511, 30, 0, 0, 0, 87, 511, 0, 0, 511, 2, 511, 72, 511, 0, 0, 0, 511, 3, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 59, 0, 51, 511, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 152, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 0, 11, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 151, 0,NULL,0,0,'object/creature/player/sullustan_female.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 0.926948,NULL, 0, 0, 0, 98, 173, 511, 0, 511, 0, 167, 220, 511, 0, 511, 0, 511, 220, 129, 511, 97, 61, 11, 511, 511, 200, 511, 178, 235, 511, 0, 0, 0, 511, 30, 0, 0, 511, 5, 511, 100, 511, 0, 0, 0, 511, 4, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 43, 0, 511, 199, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 0, 9, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 62, 0,NULL,0,0,'object/creature/player/sullustan_female.iff');
			end if;
		else
			-- male
			if profession_id = 0 then
				-- artisan
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'crafting_artisan', start_city, 1.052737,NULL, 0, 0, 0, 511, 130, 121, 0, 511, 0, 511, 66, 511, 0, 72, 0, 511, 252, 165, 511, 168, 23, 511, 134, 182, 511, 511, 140, 511, 190, 0, 0, 0, 153, 511, 0, 0, 511, 0, 0, 71, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 38, 0, 64, 511, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 228, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/sullustan_male.iff');
			elseif profession_id = 1 then				
				-- brawler
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_brawler', start_city, 1.014336,NULL, 0, 0, 0, 511, 225, 167, 0, 511, 0, 169, 50, 511, 0, 511, 0, 511, 192, 159, 511, 31, 55, 511, 11, 511, 55, 511, 234, 511, 184, 0, 0, 0, 511, 60, 0, 0, 511, 0, 0, 157, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 4, 0, 511, 186, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 44, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/sullustan_male.iff');
			elseif profession_id = 2 then				
				-- entertainer
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'social_entertainer', start_city, 1.007075,NULL, 0, 0, 0, 511, 44, 165, 0, 185, 0, 511, 511, 511, 0, 215, 0, 174, 511, 198, 116, 511, 21, 511, 95, 175, 511, 511, 162, 157, 511, 0, 0, 0, 511, 206, 0, 0, 511, 0, 0, 511, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 34, 0, 511, 182, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 53, 0, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/sullustan_male.iff');
			elseif profession_id = 3 then				
				-- marksman
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'combat_marksman', start_city, 1.055619,NULL, 0, 0, 0, 142, 97, 511, 0, 511, 0, 16, 158, 511, 0, 511, 0, 238, 511, 243, 511, 236, 14, 44, 511, 172, 511, 511, 96, 511, 122, 0, 0, 0, 511, 35, 0, 0, 511, 0, 0, 99, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 9, 0, 511, 132, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 0, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/sullustan_male.iff');
			elseif profession_id = 4 then				
				-- medic
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'science_medic', start_city, 0.948675,NULL, 0, 0, 0, 511, 122, 37, 0, 138, 0, 122, 511, 511, 0, 511, 0, 511, 62, 229, 511, 209, 10, 244, 511, 129, 511, 165, 511, 511, 189, 0, 0, 0, 8, 511, 0, 0, 511, 0, 0, 26, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 12, 0, 94, 511, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/sullustan_male.iff');
			elseif profession_id = 5 then				
				-- scout
				CALL sp_CharacterCreate(new_account_id, 2, mod_username, lastName, 'outdoors_scout', start_city, 1.019355,NULL, 0, 0, 0, 511, 53, 22, 0, 1, 0, 511, 511, 73, 0, 4, 0, 38, 511, 511, 96, 511, 2, 511, 244, 511, 63, 511, 110, 96, 511, 0, 0, 0, 511, 156, 0, 0, 511, 0, 0, 50, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 511, 511, 0, 21, 0, 189, 511, 0, 0, 511, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 190, 0, 511, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,NULL,0,0,'object/creature/player/sullustan_male.iff');
			end if;
		end if;
		
	end if;	
		
	SET bot_id = bot_id + 1;
	SET loop_counter = loop_counter + 1;

	END LOOP loop1;

COMMIT;

END $$

DELIMITER ;

/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;

