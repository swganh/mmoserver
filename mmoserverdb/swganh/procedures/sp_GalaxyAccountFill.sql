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
-- Definition of procedure `sp_GalaxyAccountFill`
--

DROP PROCEDURE IF EXISTS `sp_GalaxyAccountFill`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_GalaxyAccountFill`(IN galaxy_id INT, credits BIGINT)
BEGIN

  ##
  ## Stored Procedure
  ##
  ## Use: CALL sp_GalaxyAccountFill(galaxy_id, credits);
  ##
  ## Returns:
  ##
  ##  (0) successful
  ##  (1) NOT FOUND
  ##  (2) SQL Exception
  ##  (3) SQL Warning
  ##
  ## Galaxy Account Types Listing
  ##
  ## 01 - Character Creation
  ## 02 - Newbie Tutorial
  ## 03 - Customer Service
  ## 04 - Dynamic Mission System
  ## 05 - Player Mission System
  ## 06 - Bounty Mission System
  ## 07 - Cloning System
  ## 08 - Insurance System
  ## 09 - Galactic Travel Commission
  ## 10 - Galactic Shipping Commission
  ## 11 - Galactic Trade Commission
  ## 12 - Dispenser System
  ## 13 - Skill Training Union
  ## 14 - Rebellion
  ## 15 - Empire
  ## 16 - Jabba The Hutt
  ## 17 - POI System
  ## 18 - Corpse Expiration Tracking
  ## 19 - Testing
  ## 20 - Structure Maintenance
  ## 21 - Tip Surcharge
  ## 22 - Vendor Wages
  ## 23 - NPC Loot
  ## 24 - Junk Dealer
  ## 25 - Cantina Drink
  ## 26 - Beta Test Fund
  ## 27 - Group Split Error Account
  ## 28 - Standard Slot Machine Account
  ## 29 - Roulette Account
  ## 30 - Sabacc Account
  ## 31 - Vehicle Repair System
  ## 32 - Relic Dealer
  ## 33 - New Player Quests
  ## 34 - Contraband Scanning Fines
  ## 35 - Bank
  
	DECLARE account_id INT;
	DECLARE error INT DEFAULT 0;

  ##
  ## Declare Handlers for Transactional Support

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

	##
	## Start Transaction

	START TRANSACTION;
  
	##
	## Add credits to each Galaxy Account

	UPDATE swganh.galaxy_account SET account_credits = account_credits + credits;

	##
	## Update server_logs db

	SET account_id = 1;

	loop1: LOOP

		INSERT INTO swganh_logs.server_accounting VALUES (NULL, NOW(), 0, account_id, credits);

		IF account_id > 35 THEN LEAVE loop1;
		END IF;

		SET account_id = account_id + 1;

	END LOOP loop1;

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