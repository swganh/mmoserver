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
-- Definition of procedure `sp_AdminGiveSkillTree`
--

DROP PROCEDURE IF EXISTS `sp_AdminGiveSkillTree`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_AdminGiveSkillTree`(IN character_id BIGINT, IN skill CHAR(16))
BEGIN
	
	DECLARE prof_id int;
	
	IF skill LIKE '%artisan%' THEN SET prof_id = 70;
	END IF;	
	IF skill LIKE '%scout%' THEN SET prof_id = 30;
	END IF;
	IF skill LIKE '%entertainer%' THEN SET prof_id = 10;
	END IF;
	IF skill LIKE '%medic%' THEN SET prof_id = 50;
	END IF;
	IF skill LIKE '%brawler%' THEN SET prof_id = 90;
	END IF;
	IF skill LIKE '%marksman%' THEN SET prof_id = 109;
	END IF;
	IF skill LIKE '%rifleman%' THEN SET prof_id = 128;
	END IF;
	IF skill LIKE '%pistol%' THEN SET prof_id = 147;
	END IF;
	IF skill LIKE '%carbine%' THEN SET prof_id = 166;
	END IF;
	IF skill LIKE '%unarmed%' THEN SET prof_id = 185;
	END IF;
	IF skill LIKE '%1hsword%' THEN SET prof_id = 204;
	END IF;
	IF skill LIKE '%2hsword%' THEN SET prof_id = 223;
	END IF;
	IF skill LIKE '%polearm%' THEN SET prof_id = 242;
	END IF;
	IF skill LIKE '%dancer%' THEN SET prof_id = 261;
	END IF;
	IF skill LIKE '%musician%' THEN SET prof_id = 280;
	END IF;
	IF skill LIKE '%doctor%' THEN SET prof_id = 299;
	END IF;
	IF skill LIKE '%ranger%' THEN SET prof_id = 318;
	END IF;
	IF skill LIKE '%creaturehandler%' THEN SET prof_id = 337;
	END IF;
	IF skill LIKE '%bio_engineer%' THEN SET prof_id = 356;
	END IF;
	IF skill LIKE '%armorsmith%' THEN SET prof_id = 375;
	END IF;
	
	INSERT INTO character_skills VALUES
	(character_id, prof_id + 1),
	(character_id, prof_id + 2),
	(character_id, prof_id + 3),
	(character_id, prof_id + 4),
	(character_id, prof_id + 5),
	(character_id, prof_id + 6),
	(character_id, prof_id + 7),
	(character_id, prof_id + 8),
	(character_id, prof_id + 9),
	(character_id, prof_id + 10),
	(character_id, prof_id + 11),
	(character_id, prof_id + 12),
	(character_id, prof_id + 13),
	(character_id, prof_id + 14),
	(character_id, prof_id + 15),
	(character_id, prof_id + 16),
	(character_id, prof_id + 17),
	(character_id, prof_id + 18);

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