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
-- Definition of procedure `sp_ResourceRotate`
--

DROP PROCEDURE IF EXISTS `sp_ResourceRotate`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */ $$
CREATE PROCEDURE `sp_ResourceRotate`() SQL SECURITY INVOKER
BEGIN

  ##
  ## sp_ResourceRotate()
  ##
  ## Retires every active resource whose spawn_end has passed and spawns a
  ## replacement of the same type on the same planets, with a new name,
  ## stats, lifetime and distribution. Returns the number replaced.
  ##
  ## The zone servers call it on startup and every few minutes, then load
  ## the changes. A named lock keeps concurrent calls from doing the work
  ## twice: a call that finds the lock taken waits for that rotation to
  ## finish (it then finds nothing left), so the caller never loads a
  ## half-done rotation. It returns 0 if the wait times out.

  DECLARE expired_id BIGINT UNSIGNED;
  DECLARE expired_type INT UNSIGNED;
  DECLARE new_id BIGINT UNSIGNED;
  DECLARE new_name VARCHAR(128);
  DECLARE attempts INT;
  DECLARE rotated INT DEFAULT 0;
  DECLARE found INT DEFAULT 1;
  DECLARE CONTINUE HANDLER FOR NOT FOUND SET found = 0;

  IF GET_LOCK('swganh.sp_ResourceRotate', 30) = 1 THEN

    rotate: LOOP
      SET found = 1;
      SELECT id, type_id INTO expired_id, expired_type
        FROM resources
       WHERE active = 1 AND spawn_end <= NOW()
       ORDER BY id LIMIT 1;
      IF found = 0 THEN
        LEAVE rotate;
      END IF;

      -- Names are unique; try a few combinations before giving up on one.
      SET new_name = NULL;
      SET attempts = 0;
      WHILE new_name IS NULL AND attempts < 20 DO
        SELECT CONCAT(
                 (SELECT name FROM resource_name_begin ORDER BY RAND() LIMIT 1),
                 (SELECT name FROM resource_name_end ORDER BY RAND() LIMIT 1))
          INTO new_name;
        IF EXISTS (SELECT 1 FROM resources WHERE name = new_name) THEN
          SET new_name = NULL;
        END IF;
        SET attempts = attempts + 1;
      END WHILE;

      START TRANSACTION;

      UPDATE resources SET active = 0 WHERE id = expired_id;

      IF new_name IS NOT NULL THEN
        SELECT MAX(id) + 1 INTO new_id FROM resources;

        INSERT INTO resources
              (id, name, type_id, er, cr, cd, dr, fl, hr, ma, oq, sr, ut, pe,
               spawn_date, spawn_end, active)
        SELECT new_id, new_name, t.id,
               RAND() * (t.er_max - t.er_min) + t.er_min,
               RAND() * (t.cr_max - t.cr_min) + t.cr_min,
               RAND() * (t.cd_max - t.cd_min) + t.cd_min,
               RAND() * (t.dr_max - t.dr_min) + t.dr_min,
               RAND() * (t.fl_max - t.fl_min) + t.fl_min,
               RAND() * (t.hr_max - t.hr_min) + t.hr_min,
               RAND() * (t.ma_max - t.ma_min) + t.ma_min,
               RAND() * (t.oq_max - t.oq_min) + t.oq_min,
               RAND() * (t.sr_max - t.sr_min) + t.sr_min,
               RAND() * (t.ut_max - t.ut_min) + t.ut_min,
               RAND() * (t.pe_max - t.pe_min) + t.pe_min,
               NOW(),
               -- At least a day, even for templates with time_min = 0.
               DATE_ADD(NOW(), INTERVAL GREATEST(1, ROUND(RAND() * (t.time_max - t.time_min) + t.time_min)) DAY),
               1
          FROM resource_template t
         WHERE t.id = expired_type;

        -- Same planets as the expired resource, each with a new distribution
        -- (the same ranges as the initial spawn).
        INSERT INTO resources_spawn_config
              (resource_id, planet_id, noiseMapBoundsX1, noiseMapBoundsX2,
               noiseMapBoundsY1, noiseMapBoundsY2, noiseMapOctaves,
               noiseMapFrequency, noiseMapPersistence, noiseMapScale,
               noiseMapBias, unitsTotal, unitsLeft)
        SELECT new_id, planet_id, x1, x1 + FLOOR(1 + RAND() * 2),
               y1, y1 + FLOOR(1 + RAND() * 2), FLOOR(1 + RAND() * 2),
               1, RAND(), RAND(), RAND(), units, units
          FROM (SELECT planet_id,
                       FLOOR(2 + RAND() * 10) AS x1,
                       FLOOR(2 + RAND() * 10) AS y1,
                       FLOOR(1 + RAND() * 99999999) AS units
                  FROM resources_spawn_config
                 WHERE resource_id = expired_id) AS planets;

        SET rotated = rotated + 1;
      END IF;

      COMMIT;
    END LOOP;

    DO RELEASE_LOCK('swganh.sp_ResourceRotate');
  END IF;

  SELECT rotated;

END $$
/*!50003 SET SESSION SQL_MODE=@TEMP_SQL_MODE */  $$

DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
