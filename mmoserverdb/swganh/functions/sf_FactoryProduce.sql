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
-- Definition of function `sf_FactoryProduce`
--

DROP FUNCTION IF EXISTS `sfsf_FactoryProduce`;

DELIMITER $$

DROP FUNCTION IF EXISTS `swganh`.`sf_FactoryProduce` $$
CREATE DEFINER=`root`@`localhost` FUNCTION `sf_FactoryProduce`(fID BIGINT(20)) RETURNS int(11)
BEGIN

--
-- Declare Variables
-- it

  DECLARE rate INTEGER;
  DECLARE complexity INTEGER;
  DECLARE resID BIGINT(20);
  DECLARE hoppersize INTEGER;
  DECLARE quantity INTEGER;
  DECLARE returnValue INTEGER;

  DECLARE active INTEGER;

--
-- set a proper exit handler in case we have a faulty resource ID
--

  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000'
  BEGIN
    UPDATE factories f  SET f.active = 0 WHERE f.ID = fID;
    RETURN 3;
  END;

  SET returnValue = 0;
--
-- get the complexity of the schematic
--
  SELECT ms.complexity FROM manufactureschematic ms INNER JOIN factories f ON (f.ManSchematicID = ms.id) WHERE f.ID = fID INTO complexity;


  SELECT f.rate FROM factories f WHERE (f.ID = fID)  INTO rate;

  IF((rate +1) >= complexity) THEN
    UPDATE factories SET rate = 0 WHERE ID = fID;
    SELECT sf_FactoryCreateItem(fID) INTO returnValue;
    RETURN returnValue;
  END IF;

  UPDATE factories f SET f.rate = rate+1 WHERE ID = fID;



--
-- Return the Harvesters ID
--

  RETURN 0;

--
-- Exit
--

END $$

DELIMITER ;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
