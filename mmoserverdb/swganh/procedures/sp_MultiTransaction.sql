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
-- Definition of procedure `sp_MultiTransaction`
--

DROP PROCEDURE IF EXISTS `sp_MultiTransaction`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='STRICT_TRANS_TABLES,NO_AUTO_CREATE_USER' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_MultiTransaction`(IN input TEXT)
BEGIN
        DECLARE loopEnd INT default 0;
        DECLARE error INT default 0;
        DECLARE subQuery VARCHAR(1000);
        DECLARE cur_1 CURSOR FOR SELECT * FROM SplitValues;
        DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET loopEnd = 1;

        DECLARE EXIT HANDLER FOR NOT FOUND
        BEGIN
                SET error = 1;
                ROLLBACK;
                SELECT error;
        END;

        DECLARE EXIT HANDLER FOR SQLEXCEPTION
        BEGIN
                SET error = 2;
                ROLLBACK;
                SELECT error;
        END;

        DECLARE EXIT HANDLER FOR SQLWARNING
        BEGIN
                SET error = 3;
                ROLLBACK;
                SELECT error;
        END;

        CALL sp_splitString(input,'$$');

        OPEN cur_1;

        START TRANSACTION;

        REPEAT

                FETCH cur_1 INTO subQuery;

                IF NOT loopEnd THEN
                        if(subQuery IS NOT NULL) THEN
                           SET @_subQuery = subQuery;
                           PREPARE stmt FROM @_subQuery;
                           EXECUTE stmt;
                           DEALLOCATE PREPARE stmt;
                        END IF;
                END IF;               

        UNTIL loopEnd END REPEAT;

        CLOSE cur_1;

        COMMIT;

        SELECT error;
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