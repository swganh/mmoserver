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
-- Use schema swganh
--

USE swganh;

--
-- Definition of procedure `sp_CharacterCreate`
--

DROP PROCEDURE IF EXISTS `sp_CharacterCreate`;

DELIMITER $$

CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_CharacterCreate`(
	IN start_account_id INT,IN start_galaxy_id INT,IN start_firstname char(32),IN start_lastname char(32),
	IN start_profession char(64),IN start_city char(32),IN start_scale FLOAT,IN start_biography text(2048),
	IN 00FF INT, IN 01FF INT, IN 02FF INT, IN 03FF INT, IN 04FF INT, IN 05FF INT, IN 06FF INT, IN 07FF INT,
	IN 08FF INT, IN 09FF INT, IN 0AFF INT, IN 0BFF INT, IN 0CFF INT, IN 0DFF INT, IN 0EFF INT, IN 0FFF INT,
	IN 10FF INT, IN 11FF INT, IN 12FF INT, IN 13FF INT, IN 14FF INT, IN 15FF INT, IN 16FF INT, IN 17FF INT,
	IN 18FF INT, IN 19FF INT, IN 1AFF INT, IN 1BFF INT, IN 1CFF INT, IN 1DFF INT, IN 1EFF INT, IN 1FFF INT,
	IN 20FF INT, IN 21FF INT, IN 22FF INT, IN 23FF INT, IN 24FF INT, IN 25FF INT, IN 26FF INT, IN 27FF INT,
	IN 28FF INT, IN 29FF INT, IN 2AFF INT, IN 2BFF INT, IN 2CFF INT, IN 2DFF INT, IN 2EFF INT, IN 2FFF INT,
	IN 30FF INT, IN 31FF INT, IN 32FF INT, IN 33FF INT, IN 34FF INT, IN 35FF INT, IN 36FF INT, IN 37FF INT,
	IN 38FF INT, IN 39FF INT, IN 3AFF INT, IN 3BFF INT, IN 3CFF INT, IN 3DFF INT, IN 3EFF INT, IN 3FFF INT,
	IN 40FF INT, IN 41FF INT, IN 42FF INT, IN 43FF INT, IN 44FF INT, IN 45FF INT, IN 46FF INT, IN 47FF INT,
	IN 48FF INT, IN 49FF INT, IN 4AFF INT, IN 4BFF INT, IN 4CFF INT, IN 4DFF INT, IN 4EFF INT, IN 4FFF INT,
	IN 50FF INT, IN 51FF INT, IN 52FF INT, IN 53FF INT, IN 54FF INT, IN 55FF INT, IN 56FF INT, IN 57FF INT,
	IN 58FF INT, IN 59FF INT, IN 5AFF INT, IN 5BFF INT, IN 5CFF INT, IN 5DFF INT, IN 5EFF INT, IN 5FFF INT,
	IN 60FF INT, IN 61FF INT, IN 62FF INT, IN 63FF INT, IN 64FF INT, IN 65FF INT, IN 66FF INT, IN 67FF INT,
	IN 68FF INT, IN 69FF INT, IN 6AFF INT, IN 6BFF INT, IN 6CFF INT, IN 6DFF INT, IN 6EFF INT, IN 6FFF INT,
	IN 70FF INT, IN ABFF INT, IN AB2FF INT,
	IN start_hair_model CHAR(64), IN hair1 INT,IN hair2 INT, IN base_model_string CHAR(64))

charCreate:BEGIN

  --
  -- Declare Vars
  --

  DECLARE oX FLOAT;DECLARE oY FLOAT;DECLARE oZ FLOAT;DECLARE oW FLOAT;
  DECLARE battlefatigue INT;
	DECLARE race_id INT;
	DECLARE character_id BIGINT(20);
  DECLARE character_parent_id BIGINT(20);
  DECLARE inventory_id BIGINT(20);
  DECLARE tutorialcontainer_id BIGINT(20);
  DECLARE privateowner_id BIGINT(20);
	DECLARE bank_id BIGINT(20);
	DECLARE datapad_id BIGINT(20);
	DECLARE planet_name char(32);
	DECLARE profession_id INT;
	DECLARE t_health INT;
	DECLARE t_strength INT;
	DECLARE t_constitution INT;
	DECLARE t_action INT;
	DECLARE t_quickness INT;
	DECLARE t_stamina INT;
	DECLARE t_mind INT;
	DECLARE t_focus INT;
	DECLARE t_willpower INT;
	DECLARE start_planet INT;
	DECLARE start_x FLOAT;DECLARE start_y FLOAT;DECLARE start_z FLOAT;
	DECLARE shortSpecies CHAR(32);
	DECLARE gender INT(3);
  DECLARE base_skill_id INT;
  DECLARE nameCheck INT;
	DECLARE currentTime BIGINT(20);
  DECLARE tool_id BIGINT(20);
	DECLARE melon_id BIGINT(20);
  DECLARE charactersAllowed INT;
  DECLARE charactersCurrent INT;

  --
  -- Transactional Support
  --

  DECLARE EXIT HANDLER FOR NOT FOUND
    BEGIN
      SET character_id = 4;
      ROLLBACK;
      SELECT character_id;
    END;

  DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
      SET character_id = 4;
      ROLLBACK;
      SELECT character_id;
    END;

  DECLARE EXIT HANDLER FOR SQLWARNING
    BEGIN
    SET character_id = 4;
    ROLLBACK;
    SELECT character_id;
  END;
  --
  -- Check the new character name for validity
  --

  SELECT sf_CharacterNameDeveloperCheck(start_firstname) INTO nameCheck;
    IF nameCheck <> 666 THEN
      SELECT(nameCheck);
      LEAVE charCreate;
    END IF;

  SELECT sf_CharacterNameFictionalCheck(start_firstname) INTO nameCheck;
    IF nameCheck <> 666 THEN
      SELECT(nameCheck);
      LEAVE charCreate;
    END IF;

  SELECT sf_CharacterNameInUseCheck(start_firstname) INTO nameCheck;
    IF nameCheck <> 666 THEN
      SELECT(nameCheck);
      LEAVE charCreate;
    END IF;

  --
  -- Set the gender
  --

  IF base_model_string like '%female%' THEN
    SET gender = 0;
  ELSE
    SET gender = 1;
  END IF;

  --
  -- Set defaults (battle fatigue, world orientation)
  --

  SET character_parent_id = 0;
  SET privateowner_id = 0;
  SET battlefatigue = 0;
  SET oX = 0;
  SET oY = 1;
  SET oZ = 0;
  SET oW = 0;
		
	--
	-- Transaction Start
	--

  START TRANSACTION;

  SELECT MAX(id) + 1000 FROM characters INTO character_id FOR UPDATE;

  IF character_id IS NULL THEN
    SET character_id = 8589934593;
  END IF;

  --
  -- Set the initial IDs
  --

  SET inventory_id = character_id + 1;
  SET bank_id = character_id + 4;
  SET datapad_id = character_id + 3;
  SET tutorialcontainer_id = 0;

  SELECT planet_id, x, y, z FROM starting_location WHERE location LIKE start_city INTO start_planet, start_x, start_y, start_z;

  SELECT f_speciesShort(base_model_string) INTO shortSpecies;

  SELECT health, strength, constitution, action, quickness, stamina, mind, focus, willpower FROM starting_attributes WHERE starting_attributes.species like shortSpecies AND starting_attributes.profession like start_profession INTO t_health, t_strength, t_constitution, t_action, t_quickness, t_stamina, t_mind, t_focus, t_willpower;

  SELECT id from race where race.name like shortSpecies into race_id;

  SELECT skill_id from skills where skill_name like start_profession INTO profession_id;

  -- Don't set any default skills or XP when creating player in the Tutorial.

  IF start_city = 'tutorial' THEN
    SET character_parent_id = 2203318222960;
    SET tutorialcontainer_id = 2533274790395904;
    SET privateowner_id = character_id;
  END IF;

  IF start_city = 'default_location' THEN
    SET character_parent_id = 2203318222975; -- 2203318222960;
  END IF;

  INSERT INTO characters VALUES (character_id, start_account_id, start_galaxy_id, start_firstname, start_lastname, race_id, character_parent_id, start_planet, start_x, start_y, start_z, oX, oY, oZ, oW, 0, NULL, 0, CURDATE() + 0);

  INSERT INTO inventories VALUES (inventory_id,1,1000);
  INSERT INTO banks VALUES (bank_id,1000,-1);
  INSERT INTO datapads VALUES (datapad_id,1);
  INSERT INTO character_attributes VALUES (character_id, 1, t_health, t_strength, t_constitution, t_action, t_quickness, t_stamina, t_mind, t_focus, t_willpower, t_health, t_strength, t_constitution, t_action, t_quickness, t_stamina, t_mind, t_focus, t_willpower, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, battlefatigue,0,0,NULL,0,0,1,0,0,0,3);
  INSERT INTO character_appearance VALUES (character_id, 00FF, 01FF, 02FF, 03FF, 04FF, 05FF, 06FF, 07FF, 08FF, 09FF, 0AFF, 0BFF, 0CFF, 0DFF, 0EFF, 0FFF, 10FF, 11FF, 12FF, 13FF, 14FF, 15FF, 16FF, 17FF, 18FF, 19FF, 1AFF, 1BFF, 1CFF, 1DFF, 1EFF, 1FFF, 20FF, 21FF, 22FF, 23FF, 24FF, 25FF, 26FF, 27FF, 28FF, 29FF, 2AFF, 2BFF, 2CFF, 2DFF, 2EFF, 2FFF, 30FF, 31FF, 32FF, 33FF, 34FF, 35FF, 36FF, 37FF, 38FF, 39FF, 3AFF, 3BFF, 3CFF, 3DFF, 3EFF, 3FFF, 40FF, 41FF, 42FF, 43FF, 44FF, 45FF, 46FF, 47FF, 48FF, 49FF, 4AFF, 4BFF, 4CFF, 4DFF, 4EFF, 4FFF, 50FF, 51FF, 52FF, 53FF, 54FF, 55FF, 56FF, 57FF, 58FF, 59FF, 5AFF, 5BFF, 5CFF, 5DFF, 5EFF, 5FFF, 60FF, 61FF, 62FF, 63FF, 64FF, 65FF, 66FF, 67FF, 68FF, 69FF, 6AFF, 6BFF, 6CFF, 6DFF, 6EFF, 6FFF, 70FF, ABFF, AB2FF, start_hair_model, hair1,hair2, base_model_string,start_scale);
  INSERT INTO character_movement VALUES(character_id,5.75,1.50,1.00,0.0125);
  INSERT INTO character_tutorial VALUES(character_id,1,1,start_profession);
        
  IF start_city <> 'tutorial' THEN
    SET base_skill_id = profession_id + 1;
    CALL sp_CharacterSkillsCreate(character_id,base_skill_id,race_id);
    CALL sp_CharacterXpCreate(character_id,base_skill_id);
  END IF;
		
  IF start_biography IS NULL THEN SET start_biography = '';
  END IF;
		
  INSERT INTO character_biography VALUES (character_id, start_biography);
  INSERT INTO character_matchmaking VALUES (character_id,0,0,0,0,0);
		
  CALL sp_CharacterCreateFactions(character_id);
  CALL sp_CharacterStartingItems(inventory_id,tutorialcontainer_id,privateowner_id,race_id,profession_id,gender);
		
	--
	-- Fix Melon to have 5 stacks
	-- 

	-- Wen running the Tutorial there is no melon in the inventory. And it will make it impossible to create a character at the Tutorial zone.
	-- SELECT id FROM items WHERE parent_id = inventory_id AND item_type = 89 INTO melon_id;
	-- INSERT INTO item_attributes VALUES (melon_id, 23, 5, 3, NULL);

  IF start_city = 'tutorial' THEN
    SELECT id FROM items WHERE items.privateowner_id = character_id AND items.item_type = 89 INTO melon_id;
    INSERT INTO item_attributes VALUES (melon_id, 23, 5, 3, NULL);
	ELSE
    SELECT id FROM items WHERE items.parent_id = inventory_id AND items.item_type = 89 INTO melon_id;
    INSERT INTO item_attributes VALUES (melon_id, 23, 5, 3, NULL);
	END IF;
	

  --
  -- Fix tools to have effectivness of 0
  --

  IF start_profession LIKE '%crafting%' OR start_profession LIKE '%scout%' THEN
    IF start_city = 'tutorial' THEN
      SELECT id FROM items where items.privateowner_id = character_id AND items.item_family = 3 INTO tool_id;
      UPDATE item_attributes SET item_attributes.value = '0' WHERE attribute_id = 15 and item_id = tool_id;
    ELSE
      SELECT id FROM items where items.parent_id = inventory_id AND items.item_family = 3 INTO tool_id;
      UPDATE item_attributes SET item_attributes.value = '0' WHERE attribute_id = 15 and item_id = tool_id;
    END IF;
  END IF;

	--
	-- Commit Transaction
	--
	
  COMMIT;
   
	--
	-- Return new character ID
	--
	
  SELECT(character_id);

END $$

DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;