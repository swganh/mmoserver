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
-- Definition of procedure `sp_CreatePlanetMapViews`
--

DROP PROCEDURE IF EXISTS `sp_CreatePlanetMapViews`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_CreatePlanetMapViews`()
BEGIN

DROP TABLE IF EXISTS `v_planetmapcorellia`;
DROP VIEW IF EXISTS `v_planetmapcorellia`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_planetmapcorellia` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 0);

DROP TABLE IF EXISTS `v_planetmapdantooine`;
DROP VIEW IF EXISTS `v_planetmapdantooine`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_planetmapdantooine` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 1);

DROP TABLE IF EXISTS `v_planetmapdathomir`;
DROP VIEW IF EXISTS `v_planetmapdathomir`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_planetmapdathomir` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 2);

DROP TABLE IF EXISTS `v_planetmapdungeon1`;
DROP VIEW IF EXISTS `v_planetmapdungeon1`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`%` SQL SECURITY DEFINER VIEW `v_planetmapdungeon1` AS select `swganh`.`planetmap`.`id` AS `id`,`swganh`.`planetmap`.`name` AS `name`,`swganh`.`planetmap`.`x` AS `x`,`swganh`.`planetmap`.`z` AS `z`,`swganh`.`planetmapcategory`.`main` AS `main`,`swganh`.`planetmapcategory`.`sub` AS `sub`,`swganh`.`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`swganh`.`planetmap`.`category_id` = `swganh`.`planetmapcategory`.`id`))) where (`swganh`.`planetmap`.`planet_id` = 31);

DROP TABLE IF EXISTS `v_planetmapendor`;
DROP VIEW IF EXISTS `v_planetmapendor`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_planetmapendor` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 3);

DROP TABLE IF EXISTS `v_planetmaplok`;
DROP VIEW IF EXISTS `v_planetmaplok`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_planetmaplok` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 4);

DROP TABLE IF EXISTS `v_planetmapnaboo`;
DROP VIEW IF EXISTS `v_planetmapnaboo`;
CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `v_planetmapnaboo` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 5);

DROP TABLE IF EXISTS `v_planetmaprori`;
DROP VIEW IF EXISTS `v_planetmaprori`;
CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `v_planetmaprori` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 6);

DROP TABLE IF EXISTS `v_planetmaptaanab`;
DROP VIEW IF EXISTS `v_planetmaptaanab`;
CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `v_planetmaptaanab` AS select `swganh`.`planetmap`.`id` AS `id`,`swganh`.`planetmap`.`name` AS `name`,`swganh`.`planetmap`.`x` AS `x`,`swganh`.`planetmap`.`z` AS `z`,`swganh`.`planetmapcategory`.`main` AS `main`,`swganh`.`planetmapcategory`.`sub` AS `sub`,`swganh`.`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`swganh`.`planetmap`.`category_id` = `swganh`.`planetmapcategory`.`id`))) where (`swganh`.`planetmap`.`planet_id` = 42);

DROP TABLE IF EXISTS `v_planetmaptalus`;
DROP VIEW IF EXISTS `v_planetmaptalus`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_planetmaptalus` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 7);

DROP TABLE IF EXISTS `v_planetmaptatooine`;
DROP VIEW IF EXISTS `v_planetmaptatooine`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_planetmaptatooine` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 8);


DROP TABLE IF EXISTS `v_planetmaptutorial`;
DROP VIEW IF EXISTS `v_planetmaptutorial`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_planetmaptutorial` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 41);

DROP TABLE IF EXISTS `v_planetmapyavin4`;
DROP VIEW IF EXISTS `v_planetmapyavin4`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_planetmapyavin4` AS select `planetmap`.`id` AS `id`,`planetmap`.`name` AS `name`,`planetmap`.`x` AS `x`,`planetmap`.`z` AS `z`,`planetmapcategory`.`main` AS `main`,`planetmapcategory`.`sub` AS `sub`,`planetmap`.`icon` AS `icon` from (`planetmap` join `planetmapcategory` on((`planetmap`.`category_id` = `planetmapcategory`.`id`))) where (`planetmap`.`planet_id` = 9);


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