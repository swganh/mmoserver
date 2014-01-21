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
-- Definition of procedure `sp_CharacterCreateClone`
--

DROP PROCEDURE IF EXISTS `sp_CharacterCreateClone`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_CharacterCreateClone`(
	IN mcharacter_id BIGINT(22),
	IN CloneFacility_ID BIGINT(11)
	)
BEGIN


--
-- Declare Vars
--
  DECLARE mCloneFacilityID BIGINT(11);
  DECLARE mX FLOAT;
  DECLARE mY FLOAT;
  DECLARE mZ FLOAT;
  DECLARE mPlanetId INT(2);
  DECLARE mHealthWounds INT(8);
  DECLARE mStrengthWounds INT(8);
  DECLARE mConstitutionWounds INT(8);
  DECLARE mActionWounds INT(8);
  DECLARE mQuicknessWounds INT(8);
  DECLARE mStaminaWounds INT(8);
  DECLARE mMindWounds INT(8);
  DECLARE mFocusWounds INT(8);
  DECLARE mWillpowerWounds INT(8);
  DECLARE stat INT(2);

--
-- Get current character stats
--

  SELECT character_attributes.health_wounds, character_attributes.strength_wounds, character_attributes.constitution_wounds, 
         character_attributes.action_wounds, character_attributes.quickness_wounds, character_attributes.stamina_wounds, 
         character_attributes.mind_wounds, character_attributes.focus_wounds, character_attributes.willpower_wounds,
         buildings.x, buildings.y, buildings.z, buildings.planet_id
         FROM character_attributes 
         INNER JOIN buildings ON (id = CloneFacility_ID)
         WHERE character_id = mcharacter_id INTO mHealthWounds, mStrengthWounds, mconstitutionwounds, mActionWounds, mQuicknessWounds, mStaminaWounds, mMindWounds, mFocusWounds, mWillpowerWounds,
         mX, mY, mZ, mPlanetId;
  
--
-- Get Clone Facility for actual clone
--

  SET mCloneFacilityID = CloneFacility_ID;

--
-- Generate Clone
--

  SET stat = 0;

  SELECT COUNT(*) FROM character_clone WHERE character_id = mcharacter_id INTO stat;

  IF stat > 0 THEN
    UPDATE character_clone SET
	character_id = mcharacter_id,
	spawn_facility_id = mCloneFacilityID,
    x = mX,
    y = mY,
    z = mZ,
    planet_id =mPlanetId,
	health_wounds = mHealthWounds,
	strength_wounds = mStrengthWounds,
	constitution_wounds = mConstitutionWounds,
	action_wounds = mActionWounds,
	quickness_wounds = mQuicknessWounds,
	stamina_wounds = mStaminaWounds,
	mind_wounds = mMindWounds,
	focus_wounds = mFocusWounds,
	willpower_wounds = mWillpowerWounds WHERE character_id = mcharacter_id;
  ELSE
	INSERT INTO character_clone VALUES (
    mcharacter_id,
    mCloneFacilityID,
    mX,
    mY,
    mZ,
    mPlanetId,
    mHealthWounds,
    mStrengthWounds,
    mconstitutionwounds,
    mActionWounds,
    mQuicknessWounds,
    mStaminaWounds,
    mMindWounds,
    mFocusWounds,
    mWillpowerWounds);

  END IF;

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