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

USE swganh;

--
-- Definition of function `sf_AddPermissionList`
--

DROP FUNCTION IF EXISTS `sf_AddPermissionList`;

DELIMITER $$

DROP FUNCTION IF EXISTS `swganh`.`sf_AddPermissionList` $$
CREATE DEFINER=`root`@`localhost` FUNCTION `sf_AddPermissionList`(structure_id BIGINT(20), name CHAR(255), listname CHAR(255)) RETURNS int(11)
BEGIN
        DECLARE tmpId BIGINT(20);
        DECLARE nameId BIGINT(20);
        DECLARE ownerId BIGINT(20);
        DECLARE listcount INT;

        SELECT id FROM characters WHERE STRCMP(LOWER(firstname),name)=0 INTO nameId;

--
-- do we have a valid character?
--

        IF nameId IS NULL THEN RETURN(1);
        END IF;

--
-- make sure we do net end up several times on the list
--

        SELECT id FROM structure_admin_data WHERE (StructureId = structure_id and PlayerID = nameId and AdminType like listname) INTO tmpId;

        IF tmpId IS NOT NULL THEN RETURN(2);
        END IF;

--
-- make sure we do not get hughe lists - no more entries than 36
--

        SELECT COUNT(*) FROM structure_admin_data WHERE (StructureId = structure_id) INTO listcount;
        IF listcount > 36 THEN RETURN(3);
        END IF;

--
-- make sure we do not add ourselves to the ban list
--

        IF (STRCMP(LOWER(listname),'ban')=0) THEN

          SELECT owner FROM structures WHERE structures.ID = structure_id AND owner = nameId INTO ownerId;
            IF ownerId IS NOT NULL THEN RETURN(4);
          END IF;

        END IF;

        INSERT INTO structure_admin_data VALUES (NULL,structure_id,nameId,listname);

        RETURN(0);
END $$

DELIMITER ;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
