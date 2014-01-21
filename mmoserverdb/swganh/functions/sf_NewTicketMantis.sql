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
-- Definition of function `sf_NewTicketMantis`
--
-- Only for use on swganh.org TC since it requires Mantis to be present and installed
--

DROP FUNCTION IF EXISTS `sf_NewTicketMantis`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='' */ $$
CREATE FUNCTION `sf_NewTicketMantis`(playerName TEXT, category INT, subCategory INT, ticketcomment TEXT, info TEXT, harrassing TEXT, lang CHAR(2), bugreport TINYINT(1)) RETURNS int(11)
BEGIN

  DECLARE character_id BIGINT(20);
  DECLARE subcategory_id INT;
  DECLARE inserted INT;
  DECLARE mSeverity CHAR(32);
  DECLARE mRepeatable CHAR(32);
  DECLARE mLastMantisTextID INT;
  DECLARE mMantisCategory TEXT;
  DECLARE mMantisSeverity INT(2);
  DECLARE mMantisReproducability INT(2);

  SELECT id FROM characters WHERE characters.firstname = SUBSTRING_INDEX(playerName, ' ', 1) INTO character_id;
  SELECT csr_subcategories.subcategory_index FROM csr_subcategories WHERE (csr_subcategories.subcategory_id = subCategory) AND (csr_subcategories.category_id = category) INTO subcategory_id;
  INSERT INTO csr_tickets VALUES (NULL, subcategory_id, ticketcomment, info, harrassing, lang, bugreport, character_id, 0, 0, UNIX_TIMESTAMP());

##
## Insert ticket into Mantis
##

  ## Get Ticket Severity

  SELECT sf_GetCSRTicketValues(ticketcomment, 'Severity: ') INTO mSeverity;

  IF mSeverity LIKE '%Crash%' THEN SET mMantisSeverity = 70;
    ELSEIF mSeverity LIKE '%Cannot%' THEN SET mMantisSeverity = 60;
    ELSEIF mSeverity LIKE '%Can be%' THEN SET mMantisSeverity = 50;
    ELSE SET mMantisSeverity = 50;
  END IF;

  ## Get Ticket Reproducability

  SELECT sf_GetCSRTicketValues(ticketcomment, 'Repeatable: ') INTO mRepeatable;

  IF mRepeatable LIKE '%No%' THEN SET mMantisReproducability = 90;
    ELSE SET mMantisReproducability = 10;
  END IF;

  ## Get Ticket Author


  INSERT INTO swganh_bugtracker2.mantis_bug_text_table VALUES (NULL, ticketcomment, '', '');

  SELECT LAST_INSERT_ID() INTO mLastMantisTextID;

  SELECT name FROM swganh.csr_categories WHERE category_id = category INTO mMantisCategory;

  INSERT INTO swganh_bugtracker2.mantis_bug_table VALUES (NULL, 1, character_id, 0, 0, 30, mMantisSeverity, mMantisReproducability, 10, 10, 10, mMantisCategory, NOW(), NOW(), 10, mLastMantisTextID, '', '', '', '', '', '', 0, 10, 'SWGANH - Game Submitted Bug', 0, 0, '');
  SELECT MAX(ticket_id) FROM csr_tickets INTO inserted;
  RETURN (inserted);
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