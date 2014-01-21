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
-- Definition of procedure `sp_GetCharacterAttributes`
--

DROP PROCEDURE IF EXISTS `sp_GetCharacterAttributes`;

DELIMITER $$

CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_GetCharacterAttributes`(IN character_id BIGINT(20))
BEGIN
	SELECT
		characters.id,							-- Character ID
		characters.parent_id,						-- Character Parent ID
		characters.account_id,						-- Character Account ID
		characters.oX,							-- Character Cell Position (x)
		characters.oY,							-- Character Cell Position (y)
		characters.oZ,							-- Character Cell Position (z)
		characters.oW,							-- Character Cell Facing Direction (w)
		characters.x,							-- Character World Position (x)
		characters.y,							-- Character World Position (y)
		characters.z,							-- Character World Position (z)
		character_appearance.base_model_string,				-- Character SWG Model String
		characters.firstname,						-- Character First Name
		characters.lastname,						-- Character Last Name
		character_appearance.hair,					-- Character Hair Model
		character_appearance.hair1,					-- Character Hair Model 1
		character_appearance.hair2,					-- Character Hair Model 2
		race.name,							-- Character Race
		character_appearance.`00FF`,					-- Character Appearance (--)
		character_appearance.`01FF`,					-- Character Appearance (Eye Color)
		character_appearance.`02FF`,					-- Character Appearance (--)
		character_appearance.`03FF`,					-- Character Appearance (--)
		character_appearance.`04FF`,					-- Character Appearance (Muscle / Torso)
		character_appearance.`05FF`,					-- Character Appearance (Weight)
		character_appearance.`06FF`,					-- Character Appearance (--)
		character_appearance.`07FF`,					-- Character Appearance (Eye Size)
		character_appearance.`08FF`,					-- Character Appearance (Nose Protrusion)
		character_appearance.`09FF`,					-- Character Appearance (--)
		character_appearance.`0AFF`,					-- Character Appearance (--)
		character_appearance.`0BFF`,					-- Character Appearance (Cheek Size)
		character_appearance.`0CFF`,					-- Character Appearance (Eye Angle)
		character_appearance.`0DFF`,					-- Character Appearance (Trunk Height)
		character_appearance.`0EFF`,					-- Character Appearance (--)
		character_appearance.`0FFF`,					-- Character Appearance (Nose Width)
		character_appearance.`10FF`,					-- Character Appearance (--)
		character_appearance.`11FF`,					-- Character Appearance (--)
		character_appearance.`12FF`,					-- Character Appearance (--)
		character_appearance.`13FF`,					-- Character Appearance (Jaw Size)
		character_appearance.`14FF`,					-- Character Appearance (Skin Color)
		character_appearance.`15FF`,					-- Character Appearance (Mouth Size)
		character_appearance.`16FF`,					-- Character Appearance (--)
		character_appearance.`17FF`,					-- Character Appearance (Lip Size)
		character_appearance.`18FF`,					-- Character Appearance (--)
		character_appearance.`19FF`,					-- Character Appearance (Ear Size)
		character_appearance.`1AFF`,					-- Character Appearance (--)
		character_appearance.`1BFF`,					-- Character Appearance (--)
		character_appearance.`1CFF`,					-- Character Appearance (Chin / Gullet Size)
		character_appearance.`1DFF`,					-- Character Appearance (Nose Color)
		character_appearance.`1EFF`,					-- Character Appearance (Eyebrow Type)
		character_appearance.`1FFF`,					-- Character Appearance (Facial Hair Color)
		character_appearance.`20FF`,					-- Character Appearance (--)
		character_appearance.`21FF`,					-- Character Appearance (Eye Shape)
		character_appearance.`22FF`,					-- Character Appearance (--)
		character_appearance.`23FF`,					-- Character Appearance (Facial Hair Type)
		character_appearance.`24FF`,					-- Character Appearance (Age
		character_appearance.`25FF`,					-- Character Appearance (Freckles)
		character_appearance.`26FF`,					-- Character Appearance (Marking Pattern Type)
		character_appearance.`27FF`,					-- Character Appearance (Sensor Size / Jowl)
		character_appearance.`28FF`,					-- Character Appearance (--)
		character_appearance.`29FF`,					-- Character Appearance (--)
		character_appearance.`2AFF`,					-- Character Appearance (--)
		character_appearance.`2BFF`,					-- Character Appearance (--)
		character_appearance.`2CFF`,					-- Character Appearance (Eyeshadow Color)
		character_appearance.`2DFF`,					-- Character Appearance (Eyeshadow)
		character_appearance.`2EFF`,					-- Character Appearance (Lip Color)
		character_appearance.`2FFF`,					-- Character Appearance (--)
		character_appearance.`30FF`,					-- Character Appearance (--)
		character_appearance.`31FF`,					-- Character Appearance (--)
		character_appearance.`32FF`,					-- Character Appearance (Center Beard Color)
		character_appearance.`33FF`,					-- Character Appearance (--)
		character_appearance.`34FF`,					-- Character Appearance (--)
		character_appearance.`35FF`,					-- Character Appearance (--)
		character_appearance.`36FF`,					-- Character Appearance (--)
		character_appearance.`37FF`,					-- Character Appearance (--)
		character_appearance.`38FF`,					-- Character Appearance (--)
		character_appearance.`39FF`,					-- Character Appearance (--)
		character_appearance.`3AFF`,					-- Character Appearance (--)
		character_appearance.`3BFF`,					-- Character Appearance (--)
		character_appearance.`3CFF`,					-- Character Appearance (--)
		character_appearance.`3DFF`,					-- Character Appearance (--)
		character_appearance.`3EFF`,					-- Character Appearance (--)
		character_appearance.`3FFF`,					-- Character Appearance (--)
		character_appearance.`40FF`,					-- Character Appearance (--)
		character_appearance.`41FF`,					-- Character Appearance (--)
		character_appearance.`42FF`,					-- Character Appearance (--)
		character_appearance.`43FF`,					-- Character Appearance (--)
		character_appearance.`44FF`,					-- Character Appearance (--)
		character_appearance.`45FF`,					-- Character Appearance (--)
		character_appearance.`46FF`,					-- Character Appearance (--)
		character_appearance.`47FF`,					-- Character Appearance (--)
		character_appearance.`48FF`,					-- Character Appearance (--)
		character_appearance.`49FF`,					-- Character Appearance (--)
		character_appearance.`4AFF`,					-- Character Appearance (--)
		character_appearance.`4BFF`,					-- Character Appearance (--)
		character_appearance.`4CFF`,					-- Character Appearance (--)
		character_appearance.`4DFF`,					-- Character Appearance (--)
		character_appearance.`4EFF`,					-- Character Appearance (--)
		character_appearance.`4FFF`,					-- Character Appearance (--)
		character_appearance.`50FF`,					-- Character Appearance (--)
		character_appearance.`51FF`,					-- Character Appearance (--)
		character_appearance.`52FF`,					-- Character Appearance (--)
		character_appearance.`53FF`,					-- Character Appearance (--)
		character_appearance.`54FF`,					-- Character Appearance (--)
		character_appearance.`55FF`,					-- Character Appearance (--)
		character_appearance.`56FF`,					-- Character Appearance (--)
		character_appearance.`57FF`,					-- Character Appearance (--)
		character_appearance.`58FF`,					-- Character Appearance (--)
		character_appearance.`59FF`,					-- Character Appearance (--)
		character_appearance.`5AFF`,					-- Character Appearance (--)
		character_appearance.`5BFF`,					-- Character Appearance (--)
		character_appearance.`5CFF`,					-- Character Appearance (--)
		character_appearance.`5DFF`,					-- Character Appearance (--)
		character_appearance.`5EFF`,					-- Character Appearance (--)
		character_appearance.`5FFF`,					-- Character Appearance (--)
		character_appearance.`60FF`,					-- Character Appearance (--)
		character_appearance.`61FF`,					-- Character Appearance (--)
		character_appearance.`62FF`,					-- Character Appearance (--)
		character_appearance.`63FF`,					-- Character Appearance (--)
		character_appearance.`64FF`,					-- Character Appearance (--)
		character_appearance.`65FF`,					-- Character Appearance (--)
		character_appearance.`66FF`,					-- Character Appearance (--)
		character_appearance.`67FF`,					-- Character Appearance (--)
		character_appearance.`68FF`,					-- Character Appearance (--)
		character_appearance.`69FF`,					-- Character Appearance (--)
		character_appearance.`6AFF`,					-- Character Appearance (--)
		character_appearance.`6BFF`,					-- Character Appearance (--)
		character_appearance.`6CFF`,					-- Character Appearance (--)
		character_appearance.`6DFF`,					-- Character Appearance (--)
		character_appearance.`6EFF`,					-- Character Appearance (--)
		character_appearance.`6FFF`,					-- Character Appearance (--)
		character_appearance.`70FF`,					-- Character Appearance (--)
		character_appearance.`ABFF`,					-- Character Appearance (--)
		character_appearance.`AB2FF`,					-- Character Appearance (--)
		character_attributes.health_max,				-- Character HAM (Maximum Health)
		character_attributes.strength_max,				-- Character HAM (Maximum Strength)
		character_attributes.constitution_max,				-- Character HAM (Maximum Constitution)
		character_attributes.action_max,				-- Character HAM (Maximum Action)
		character_attributes.quickness_max,				-- Character HAM (Maximum Quickness)
		character_attributes.stamina_max,				-- Character HAM (Maximum Stamina)
		character_attributes.mind_max,					-- Character HAM (Maximum Mind)
		character_attributes.focus_max,					-- Character HAM (Maximum Focus)
		character_attributes.willpower_max,				-- Character HAM (Maximum Willpower)
		character_attributes.health_current,				-- Character HAM (Current Health)
		character_attributes.strength_current,				-- Character HAM (Maximum Strength)
		character_attributes.constitution_current,			-- Character HAM (Maximum Constitution)
		character_attributes.action_current,				-- Character HAM (Maximum Action)
		character_attributes.quickness_current,				-- Character HAM (Maximum Quickness)
		character_attributes.stamina_current,				-- Character HAM (Maximum Stamina)
		character_attributes.mind_current,				-- Character HAM (Maximum Mind)
		character_attributes.focus_current,				-- Character HAM (Maximum Focus)
		character_attributes.willpower_current,				-- Character HAM (Maximum Willpower)
		character_attributes.health_wounds,				-- Character Wounds (Health)
		character_attributes.strength_wounds,				-- Character Wounds (Strength)
		character_attributes.constitution_wounds,			-- Character Wounds (Constitution)
		character_attributes.action_wounds,				-- Character Wounds (Action)
		character_attributes.quickness_wounds,				-- Character Wounds (Quickness)
		character_attributes.stamina_wounds,				-- Character Wounds (Stamina)
		character_attributes.mind_wounds,				-- Character Wounds (Mind)
		character_attributes.focus_wounds,				-- Character Wounds (Focus)
		character_attributes.willpower_wounds,				-- Character Wounds (Willpower)
		character_attributes.health_encum,				-- Character Encumberance (Health)
		character_attributes.action_encum,				-- Character Encumberance (Action)
		character_attributes.mind_encum,				-- Character Encumberance (Mind)
		character_attributes.battlefatigue,				-- Character Battlefatigue
		character_attributes.language,					-- Character Language
		banks.credits,							-- Character Credits
		faction.name,							-- Character Faction
		character_attributes.posture,					-- Character Posture
		character_attributes.moodId,					-- Character Mood
		characters.jedistate,						-- Character Jedi Flag
		character_attributes.title,					-- Character Title
		character_appearance.scale,					-- Character Scale
		character_movement.baseSpeed,					-- Character Attribute (base Speed)
		character_movement.baseAcceleration,				-- Character Attribute (Base Acceleration)
		character_movement.baseTurnrate,				-- Character Attribute (Base Turnrate)
		character_movement.baseTerrainNegotiation,			-- Character Attribute (Terrain Negotiation)
		character_attributes.character_flags,				-- Character 
		character_biography.biography,					-- Character Biography
		character_attributes.states,					-- Character States
		characters.race_id,						-- Character Race
		banks.planet_id,						-- Character Bank Location (Planet)
		account.csr,							-- Character CSR Flag
		character_attributes.group_id,					-- Character Group ID
		characters.bornyear,						-- Character Year of Creation
		character_matchmaking.match_1,					-- Character MatchMaking Attribute 1
		character_matchmaking.match_2,					-- Character MatchMaking Attribute 2
		character_matchmaking.match_3,					-- Character MatchMaking Attribute 3
		character_matchmaking.match_4,					-- Character MatchMaking Attribute 4
		character_attributes.force_current,				-- Character Force (Current)
		character_attributes.force_max,					-- Character Force (Maximum)
		character_attributes.new_player_exemptionsi			-- Character New Player Exemptions
	FROM characters
		INNER JOIN account ON(characters.account_id = account.account_id)
		INNER JOIN banks ON ((characters.id + 4) = banks.id)
		INNER JOIN character_appearance ON (characters.id = character_appearance.character_id)
		INNER JOIN race ON (characters.race_id = race.id)
		INNER JOIN character_attributes ON (characters.id = character_attributes.character_id)
		INNER JOIN character_movement ON (characters.id = character_movement.character_id)
		INNER JOIN faction ON (character_attributes.faction_id = faction.id)
		INNER JOIN character_biography ON (characters.id = character_biography.character_id)
		INNER JOIN character_matchmaking ON (characters.id = character_matchmaking.character_id)
	WHERE (characters.id = character_id);
END $$

DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
