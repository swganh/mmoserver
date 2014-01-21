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
-- Definition of function `sf_DiscardResource`
--

DROP FUNCTION IF EXISTS `swganh`.`sf_DiscardResource`;

DELIMITER $$

CREATE FUNCTION `sf_DiscardResource`(structure_id BIGINT(20), resource_id BIGINT(20), amount FLOAT) RETURNS int(11)
BEGIN
        DECLARE RAmount FLOAT;
        DECLARE Rres BIGINT(20);

        SELECT quantity FROM harvester_resources WHERE structure_id = ID AND resource_id = resourceID INTO RAmount;
        SELECT ResourceID FROM harvesters WHERE structure_id = ID INTO Rres;

        IF RAmount > amount THEN
           UPDATE harvester_resources set quantity = RAmount - amount WHERE structure_id = ID AND resource_id = resourceID;
           RETURN(0);
        END IF;

        IF RAmount < amount THEN
           RETURN(1);
        END IF;


        IF Rres = resource_id THEN

           UPDATE harvester_resources set quantity = 0 WHERE structure_id = ID AND resource_id = resourceID;
           RETURN(0);
        END IF;

        DELETE FROM harvester_resources WHERE structure_id = ID AND resource_id = resourceID;

        RETURN(0);

END $$

DELIMITER ;

/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;