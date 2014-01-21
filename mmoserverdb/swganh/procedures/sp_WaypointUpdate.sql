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
-- Definition of procedure `sp_WaypointUpdate`
--

DROP PROCEDURE IF EXISTS `sp_WaypointUpdate`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='' */ $$
CREATE PROCEDURE `sp_WaypointUpdate`(IN wp_name CHAR(255), wp_id BIGINT(20), wp_x FLOAT, wp_y FLOAT, wp_z FLOAT, wp_planet INT(3), wp_state TINYINT(1))
BEGIN

	##
	## Stored Procedure
	##
	## sp_WaypointUpdate
	##
	## Usage:
	##
	## CALL sp_WaypointUpdate (wp_id, wp_name, wp_x, wp_y, wp_z, wp_planet, wp_state);
	##
	## Returns:
	##
	##  (0) successful
	##  (1) NOT FOUND
	##  (2) SQL Exception
	##  (3) SQL Warning

	##
	## Declare Vars

	DECLARE w_name CHAR(255);
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
	## Get out waypoint name

	SELECT name FROM swganh.waypoints WHERE waypoint_id = wp_id INTO w_name;

	##
	## Check the waypoint name

	IF wp_name IS NULL OR wp_name NOT LIKE w_name THEN
		SET wp_name = w_name;
	END IF;

	##
	## Update our waypoint

	UPDATE swganh.waypoints SET
		name = wp_name,
		x = wp_x,
		y = wp_y,
		z = wp_z,
		planet_id = wp_planet,
		active = wp_state
	WHERE
		waypoint_id = wp_id;

	##
	## Commit

	COMMIT;

	##
	## Return & Exit

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