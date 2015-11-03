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
-- Definition of procedure `sp_ReturnUserAccount`
--

DROP PROCEDURE IF EXISTS `sp_ReturnUserAccount`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_ReturnUserAccount`(IN usrName CHAR(255),IN pwrd CHAR(255))
BEGIN

  ##
  ## Stored Procedure
  ##
  ## Use: CALL sp_ReturnUserAccount(username, password);
  ##
  ## Returns: (server global tick)
  
  --
  -- Declare Vars
  --

 DECLARE user CHAR(255);
  
DECLARE id  BIGINT(20);
  
DECLARE shaPwrd  CHAR(255);
  
DECLARE mAccount_id BIGINT(20);
  
DECLARE mStation_id BIGINT(20);
  
DECLARE mAccount_password CHAR(64);
  

SET shaPwrd = SHA1(pwrd);

  
SELECT account_id 
  FROM swganh.account 
  WHERE account_username = usrName 
  INTO id;
  
  
IF id  IS NULL THEN 
	  
SELECT MAX(account_id) + 1 FROM swganh.account INTO mAccount_id FOR UPDATE;

	  

IF mAccount_id IS NULL THEN
		  
SET mAccount_id = 1;
	  
END IF;

	  
SELECT MAX(account_station_id) + 1 FROM swganh.account INTO mStation_id;

		
IF mStation_id IS NULL THEN
		  SET mStation_id = 1000000;
		
END IF;

	   
INSERT INTO swganh.account VALUES (NULL, usrName, shaPwrd, mStation_id, 0, 0, '', NOW(), NOW(), 1, 0, 0, 1, NULL, NOW() - INTERVAL 1 DAY, NULL, NULL);

  END IF;

  
SELECT 
	    account_id, 
		account_username, 
		account_password, 
		account_station_id, 
		account_banned, 
		account_active, 
		account_characters_allowed, 
		account_csr 
  FROM swganh.account 
  WHERE account_banned=0 AND account_authenticated = 0 AND account_loggedin = 0 AND account_username= usrName AND account_password = shaPwrd;

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