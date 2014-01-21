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
-- Definition of procedure `sp_CharacterStartingItems`
--

DROP PROCEDURE IF EXISTS `sp_CharacterStartingItems`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,NO_AUTO_CREATE_USER' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_CharacterStartingItems`(
	IN inventory_id BIGINT(20),
	IN tutorialcontainer_id BIGINT(20),
	IN privateowner_id BIGINT(20),
	IN race_id INT(8),
	IN profession_id INT(8),
	IN gender_id INT(3))
BEGIN

	DECLARE tmpId BIGINT(20);
        DECLARE familyId,typeId INT(11);
        DECLARE custom_name CHAR(255);
        DECLARE loopEnd INT DEFAULT 0;
        DECLARE cur_1 CURSOR FOR SELECT family_id,type_id,customName FROM starting_items WHERE req_id = (SELECT id FROM starting_item_requirements WHERE species=race_id AND gender=gender_id AND profession=profession_id);
        DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET loopEnd = 1;

	OPEN cur_1;

        REPEAT
                FETCH cur_1 INTO familyId,typeId,custom_name;

                IF NOT loopEnd THEN
						IF tutorialcontainer_id = 0 THEN
						
							-- equip if its a wearable
							IF familyId = 7 THEN
									SELECT sf_DefaultItemCreate(familyId,typeId,inventory_id-1,0,99,0,0,0,custom_name) INTO tmpId;

									UPDATE item_attributes SET value='1' WHERE item_id=tmpId AND attribute_id=11;

							ELSE
									SELECT sf_DefaultItemCreate(familyId,typeId,inventory_id,0,99,0,0,0,custom_name) INTO tmpId;

							END IF;

						ELSE

							IF familyId = 7 THEN
								-- equip if its a wearable, and store in inventory.
								-- Do not mark items stored into inventory as "private".
								SELECT sf_DefaultItemCreate(familyId,typeId,inventory_id-1,0,99,0,0,0,custom_name) INTO tmpId;

								UPDATE item_attributes SET value='1' WHERE item_id=tmpId AND attribute_id=11;
							ELSE
								-- create item in a container, used by tutorial.
								-- Do not add any CDEF-pistol, it will be given to player by a NPC.
								IF familyId <> 10 OR typeId <> 2326 THEN
									SELECT sf_DefaultItemCreate(familyId,typeId,tutorialcontainer_id,privateowner_id,99,0,0,0,custom_name) INTO tmpId;
								END IF;

							END IF;
						
						END IF;
                END IF;

        UNTIL loopEnd END REPEAT;

        CLOSE cur_1;

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