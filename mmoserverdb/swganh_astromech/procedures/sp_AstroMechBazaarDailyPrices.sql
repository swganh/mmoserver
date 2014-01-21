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

use swganh_astromech;

--
-- Definition of procedure `sp_AstroMechBazaarDailyPrices`
--

DELIMITER $$

DROP PROCEDURE IF EXISTS `swganh_astromech`.`sp_AstroMechBazaarDailyPrices` $$
CREATE DEFINER=`root`@`%` PROCEDURE `sp_AstroMechBazaarDailyPrices`()
BEGIN
  DECLARE item_id INT;
  DECLARE item_min INT;
  DECLARE item_max INT;
  DECLARE item_avg INT;
  DECLARE item_count INT;
  DECLARE item_total INT;
  DECLARE loop_counter INT(5);
  DECLARE current_item INT;
  DECLARE item_name CHAR(255);
  DECLARE mtime BIGINT(22);

  SELECT COUNT(*) FROM swganh.item_types INTO item_total;
  SET loop_counter = 1;

	loop1: LOOP
    SELECT
      swganh.items.item_type,
      swganh.item_types.object_string,
      MAX(swganh.commerce_auction.price),
      MIN(swganh.commerce_auction.price),
      AVG(swganh.commerce_auction.price),
      COUNT(swganh.commerce_auction.price),
      UNIX_TIMESTAMP(CURRENT_DATE())
      FROM swganh.commerce_auction
        INNER JOIN swganh.items ON (swganh.commerce_auction.auction_id = swganh.items.id)
        INNER JOIN swganh.item_types ON (swganh.items.item_type = swganh.item_types.id)
      WHERE swganh.item_types.id = loop_counter INTO item_id, item_name, item_min, item_max, item_avg, item_count, mtime;

	
    INSERT INTO `swganh_astromech`.`AstromMechBazaarDailyPrices` VALUES (item_id, item_name, item_min, item_max, item_avg, item_count, mtime);

    IF loop_counter = item_total THEN
       LEAVE loop1;
       ELSE SET loop_counter = loop_counter + 1;
    END IF;

  END LOOP loop1;

END $$

DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
