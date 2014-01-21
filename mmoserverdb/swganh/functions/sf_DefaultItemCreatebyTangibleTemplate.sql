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
-- Definition of function `sf_DefaultItemCreatebyTangibleTemplate`
--

DROP FUNCTION IF EXISTS `sf_DefaultItemCreatebyTangibleTemplate`;

DELIMITER $$

DROP FUNCTION IF EXISTS `swganh`.`sf_DefaultItemCreatebyTangibleTemplate` $$
CREATE DEFINER=`root`@`localhost` FUNCTION `sf_DefaultItemCreatebyTangibleTemplate`(parent_id BIGINT(20),template_id BIGINT(20)) RETURNS bigint(20)
BEGIN
        DECLARE tmpId BIGINT(20);
        DECLARE att_id,att_order INT;
        DECLARE att_value CHAR(255);
        DECLARE loopEnd INT DEFAULT 0;

        DECLARE cur_1 CURSOR FOR SELECT ia.attribute_id,ia.value,ia.order FROM item_attributes ia WHERE ia.item_id = template_id;

        DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET loopEnd = 1;

        INSERT INTO items VALUES (NULL,parent_id,21,1166,0,0,0,0,1,0,0,0,99,"",100,0,0);

        SET tmpId = LAST_INSERT_ID();

        UPDATE items i JOIN items ii ON (ii.id = template_id) SET i.item_family = ii.item_family, i.item_type = ii.item_type, i.customName = ii.customName WHERE i.id = tmpId;

        OPEN cur_1;
        REPEAT
                FETCH cur_1 INTO att_id,att_value,att_order;
                IF NOT loopEnd THEN
                        INSERT INTO item_attributes VALUES(tmpId,att_id,att_value,att_order,NULL);
                END IF;
        UNTIL loopEnd END REPEAT;

        CLOSE cur_1;
        RETURN(tmpId);
END $$

DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
