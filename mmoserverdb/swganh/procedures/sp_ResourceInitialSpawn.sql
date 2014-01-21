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
-- Definition of procedure `sp_ResourceInitialSpawn`
--

DROP PROCEDURE IF EXISTS `sp_ResourceInitialSpawn`;

DELIMITER $$

/*!50003 SET @TEMP_SQL_MODE=@@SQL_MODE, SQL_MODE='' */ $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_ResourceInitialSpawn`()
BEGIN

--
-- Declare SP Variables
--

  DECLARE loop_counter INT(5);
  DECLARE loop_end INT(5);
  DECLARE generated_resource_id BIGINT(22);
  DECLARE begin_name char(8);
  DECLARE end_name char(16);
  DECLARE planet_check INT(2);
--
-- Declare Generated Resource Variables
--

  DECLARE generated_type_id int(11);
  DECLARE generated_id int(11);
  DECLARE generated_category_id int(11);
  DEcLARE generated_namefile_name char(255);
  DECLARE generated_type_name varchar(128);
  DECLARE generated_type_swg varchar(128);
  DECLARE generated_er_min int(4);
  DECLARE generated_er_max int(4);
  DECLARE generated_cr_min int(4);
  DECLARE generated_cr_max int(4);
  DECLARE generated_cd_min int(4);
  DECLARE generated_cd_max int(4);
  DECLARE generated_dr_min int(4);
  DECLARE generated_dr_max int(4);
  DECLARE generated_fl_min int(4);
  DECLARE generated_fl_max int(4);
  DECLARE generated_hr_min int(4);
  DECLARE generated_hr_max int(4);
  DECLARE generated_ma_min int(4);
  DECLARE generated_ma_max int(4);
  DECLARE generated_oq_min int(4);
  DECLARE generated_oq_max int(4);
  DECLARE generated_sr_min int(4);
  DECLARE generated_sr_max int(4);
  DECLARE generated_ut_min int(4);
  DECLARE generated_ut_max int(4);
  DECLARE generated_pe_min int(4);
  DECLARE generated_pe_max int(4);
  DECLARE generated_tang varchar(128);
  DECLARE generated_type varchar(32);
  DECLARE generated_spawn_min int(3);
  DECLARE generated_spawn_max int(3);
  DECLARE generated_spawn_end int(3);
  DECLARE generated_time_min int(11);
  DECLARE generated_time_max int(11);
  DECLARE generated_weight int(1);
  DECLARE generated_planet_id int(1);
  DECLARE generated_bazaar_catid int(11);

--
-- Declare Final Resource Variables
--
		
	DECLARE final_name char(45);
	DECLARE final_er int;
	DECLARE final_cr int;
	DECLARE final_cd int;
	DECLARE final_dr int;
	DECLARE final_fl int;
	DECLARE final_hr int;
	DECLARE final_ma int;
	DECLARE final_oq int;
	DECLARE final_sr int;
	DECLARE final_ut int;
	DECLARE final_pe int;
	DECLARE final_spawn_end DATETIME;
	
--
-- Declare Resource NoiseMap Variables
--
	
	DECLARE gen_noiseMapBoundsX1 double;
	DECLARE gen_noiseMapBoundsX2 double;
	DECLARE gen_noiseMapBoundsY1 double;
	DECLARE gen_noiseMapBoundsY2 double;
	DECLARE gen_noiseMapOctaves tinyint;
	DECLARE gen_noiseMapFrequency double;
	DECLARE gen_noiseMapPersistence double;
	DECLARE gen_unitsTotal BIGINT;
	DECLARE gen_noiseMapScale double;
	DECLARE gen_noiseMapBias double;
	
--
-- Declare Resource Cursor Variables
--
	
	DECLARE done INT DEFAULT 0;
	DECLARE curResourceNative CURSOR FOR SELECT * FROM MemNativePool;
	DECLARE curResourceFixed CURSOR FOR SELECT * FROM MemFixedPool;
	DECLARE curResourceRandom CURSOR FOR SELECT * FROM MemRandomPool;
	DECLARE curResourceMinimum CURSOR FOR SELECT * FROM MemMinimumPool;
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET done = 1;
	DECLARE CONTINUE HANDLER FOR 1062 BEGIN END;

--
-- Create the various resource Pools
--

--
-- Native Pool:
--
-- One Fiberplast for each planet
-- One Water for each planet
-- One Wind for each planet
-- One Solar for each planet
-- All 38 Organics for each planet
--

DROP TABLE IF EXISTS `MemNativePool`;
CREATE TABLE `MemNativePool` (
  `id` int(11) unsigned NOT NULL,
  `category_id` int(11) unsigned NOT NULL default '1',
  `namefile_name` char(255) default NULL,
  `type_name` varchar(128) NOT NULL,
  `type_swg` varchar(128) NOT NULL,
  `er_min` int(4) default NULL,
  `er_max` int(4) default NULL,
  `cr_min` int(4) default NULL,
  `cr_max` int(4) default NULL,
  `cd_min` int(4) default NULL,
  `cd_max` int(4) default NULL,
  `dr_min` int(4) default NULL,
  `dr_max` int(4) default NULL,
  `fl_min` int(4) default NULL,
  `fl_max` int(4) default NULL,
  `hr_min` int(4) default NULL,
  `hr_max` int(4) default NULL,
  `ma_min` int(4) default NULL,
  `ma_max` int(4) default NULL,
  `oq_min` int(4) default NULL,
  `oq_max` int(4) default NULL,
  `sr_min` int(4) default NULL,
  `sr_max` int(4) default NULL,
  `ut_min` int(4) default NULL,
  `ut_max` int(4) default NULL,
  `pe_min` int(4) default NULL,
  `pe_max` int(4) default NULL,
  `tang` varchar(128) default NULL,
  `type` varchar(32) default NULL,
  `spawn_min` int(3) NOT NULL,
  `spawn_max` int(3) NOT NULL,
  `time_min` int(11) NOT NULL,
  `time_max` int(11) NOT NULL,
  `weight` int(1) NOT NULL,
	`planet_id` int(11) default NULL,
  `bazaar_catid` INT(11) NOT NULL,
  KEY (`category_id`)
) ENGINE=Memory;

--
-- Populate Native Pool
--

	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Fiberplast%' AND type_swg NOT LIKE 'fiberplast_grav%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Water%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Wind%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Solar%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Milk%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Meat%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Egg%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Bones%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Hide%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Corn%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Rice%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Oats%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Wheat%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Greens%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Beans%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Tubers%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Fungus%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Fruit%' AND type_swg NOT LIKE 'mixed_fruits';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Flowers%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Wood%';
	INSERT INTO MemNativePool SELECT * FROM resource_template WHERE type_name LIKE '%Horn%';

--
-- Fixed Pool:
--
-- 8 JTL Resources
-- 14 Iron Resouces
--

DROP TABLE IF EXISTS `MemFixedPool`;
CREATE TABLE `MemFixedPool` (
  `id` int(11) unsigned NOT NULL,
  `category_id` int(11) unsigned NOT NULL default '1',
  `namefile_name` char(255) default NULL,
  `type_name` varchar(128) NOT NULL,
  `type_swg` varchar(128) NOT NULL,
  `er_min` int(4) default NULL,
  `er_max` int(4) default NULL,
  `cr_min` int(4) default NULL,
  `cr_max` int(4) default NULL,
  `cd_min` int(4) default NULL,
  `cd_max` int(4) default NULL,
  `dr_min` int(4) default NULL,
  `dr_max` int(4) default NULL,
  `fl_min` int(4) default NULL,
  `fl_max` int(4) default NULL,
  `hr_min` int(4) default NULL,
  `hr_max` int(4) default NULL,
  `ma_min` int(4) default NULL,
  `ma_max` int(4) default NULL,
  `oq_min` int(4) default NULL,
  `oq_max` int(4) default NULL,
  `sr_min` int(4) default NULL,
  `sr_max` int(4) default NULL,
  `ut_min` int(4) default NULL,
  `ut_max` int(4) default NULL,
  `pe_min` int(4) default NULL,
  `pe_max` int(4) default NULL,
  `tang` varchar(128) default NULL,
  `type` varchar(32) default NULL,
  `spawn_min` int(3) NOT NULL,
  `spawn_max` int(3) NOT NULL,
  `time_min` int(11) NOT NULL,
  `time_max` int(11) NOT NULL,
  `weight` int(1) NOT NULL,
	`planet_id` int(11) default NULL,
  `bazaar_catid` INT(11) NOT NULL,
  KEY (`category_id`)
) ENGINE=Memory;

--
-- Populate Fixed Pool
--

	SET loop_counter = 0;

	loop1: LOOP
	INSERT INTO MemFixedPool
  SELECT * from resource_template where type_name like '%Iron%' AND type_swg NOT LIKE '%smelted%' ORDER BY RAND() LIMIT 1;
	IF loop_counter = 13 THEN
	LEAVE loop1;
	ELSE SET loop_counter = loop_counter + 1;
	END IF;
	END LOOP loop1;
	
--
-- Populate JTL InOrganics
--

	INSERT INTO MemFixedPool SELECT * FROM resource_template WHERE id = 682;
	INSERT INTO MemFixedPool SELECT * FROM resource_template WHERE id = 683;
	INSERT INTO MemFixedPool SELECT * FROM resource_template WHERE id = 684;
	INSERT INTO MemFixedPool SELECT * FROM resource_template WHERE id = 685;
	INSERT INTO MemFixedPool SELECT * FROM resource_template WHERE id = 686;
	INSERT INTO MemFixedPool SELECT * FROM resource_template WHERE id = 687;
	INSERT INTO MemFixedPool SELECT * FROM resource_template WHERE id = 688;
	INSERT INTO MemFixedPool SELECT * FROM resource_template WHERE id = 689;
	
	UPDATE MemFixedPool SET planet_id = FLOOR(RAND()*10);
	
--
-- Random Pool (24 Resources):
--
-- Any Mineral except Iron & JTL
-- Any Gas except JTL
-- Any Chemical except Fiberplast
-- Extra Water
--

DROP TABLE IF EXISTS `MemRandomPool`;
CREATE TABLE `MemRandomPool` (
  `id` int(11) unsigned NOT NULL,
  `category_id` int(11) unsigned NOT NULL default '1',
  `namefile_name` char(255) default NULL,
  `type_name` varchar(128) NOT NULL,
  `type_swg` varchar(128) NOT NULL,
  `er_min` int(4) default NULL,
  `er_max` int(4) default NULL,
  `cr_min` int(4) default NULL,
  `cr_max` int(4) default NULL,
  `cd_min` int(4) default NULL,
  `cd_max` int(4) default NULL,
  `dr_min` int(4) default NULL,
  `dr_max` int(4) default NULL,
  `fl_min` int(4) default NULL,
  `fl_max` int(4) default NULL,
  `hr_min` int(4) default NULL,
  `hr_max` int(4) default NULL,
  `ma_min` int(4) default NULL,
  `ma_max` int(4) default NULL,
  `oq_min` int(4) default NULL,
  `oq_max` int(4) default NULL,
  `sr_min` int(4) default NULL,
  `sr_max` int(4) default NULL,
  `ut_min` int(4) default NULL,
  `ut_max` int(4) default NULL,
  `pe_min` int(4) default NULL,
  `pe_max` int(4) default NULL,
  `tang` varchar(128) default NULL,
  `type` varchar(32) default NULL,
  `spawn_min` int(3) NOT NULL,
  `spawn_max` int(3) NOT NULL,
  `time_min` int(11) NOT NULL,
  `time_max` int(11) NOT NULL,
  `weight` int(1) NOT NULL,
	`planet_id` int(11) default NULL,
  `bazaar_catid` INT(11) NOT NULL,
  KEY (`category_id`)
) ENGINE=Memory;

--
-- Populate Random Pool
--
	
	SET loop_counter = 0;

	loop1: LOOP
	INSERT INTO MemRandomPool
  	SELECT * from resource_template WHERE TYPE LIKE 'mineral_resource' OR namefile_name LIKE 'reactive' OR namefile_name LIKE 'inert' AND type_swg NOT LIKE '%smelted%' AND type_swg NOT LIKE '%fiberplast%' AND type_swg NOT LIKE '%iron%' AND id != 682 AND id != 683 AND id != 683 AND id != 684 AND id != 685 AND id != 686 AND id != 687 AND id != 688 AND id != 689 ORDER BY RAND() LIMIT 1;
	IF loop_counter = 22 THEN
	LEAVE loop1;
	ELSE SET loop_counter = loop_counter + 1;
	END IF;
	END LOOP loop1;

--
-- Minimum Pool (15 Resources):
--
-- 1 Steel not including Hardened and Crystallized from JTL
-- 1 Copper not including Conductive from JTL
-- 1 Aluminium not including Perovskitic from JTL
-- 1 Extrusive Ore
-- 1 Intrusive Ore
-- 1 Carbonate Ore
-- 1 Crystalline Gemstone
-- 1 Armophous Gemstone
-- 1 Known Radioactive not including High Grade from JTL
-- 1 Known Solid Petrochemicals
-- 1 Known Liquid Petrochemicals
-- 2 Polymer
-- 2 Lubricating Oil
--

DROP TABLE IF EXISTS `MemMinimumPool`;
CREATE TABLE `MemMinimumPool` (
  `id` int(11) unsigned NOT NULL,
  `category_id` int(11) unsigned NOT NULL default '1',
  `namefile_name` char(255) default NULL,
  `type_name` varchar(128) NOT NULL,
  `type_swg` varchar(128) NOT NULL,
  `er_min` int(4) default NULL,
  `er_max` int(4) default NULL,
  `cr_min` int(4) default NULL,
  `cr_max` int(4) default NULL,
  `cd_min` int(4) default NULL,
  `cd_max` int(4) default NULL,
  `dr_min` int(4) default NULL,
  `dr_max` int(4) default NULL,
  `fl_min` int(4) default NULL,
  `fl_max` int(4) default NULL,
  `hr_min` int(4) default NULL,
  `hr_max` int(4) default NULL,
  `ma_min` int(4) default NULL,
  `ma_max` int(4) default NULL,
  `oq_min` int(4) default NULL,
  `oq_max` int(4) default NULL,
  `sr_min` int(4) default NULL,
  `sr_max` int(4) default NULL,
  `ut_min` int(4) default NULL,
  `ut_max` int(4) default NULL,
  `pe_min` int(4) default NULL,
  `pe_max` int(4) default NULL,
  `tang` varchar(128) default NULL,
  `type` varchar(32) default NULL,
  `spawn_min` int(3) NOT NULL,
  `spawn_max` int(3) NOT NULL,
  `time_min` int(11) NOT NULL,
  `time_max` int(11) NOT NULL,
  `weight` int(1) NOT NULL,
	`planet_id` int(11) default NULL,
  `bazaar_catid` INT(11) NOT NULL,
  KEY (`category_id`)
) ENGINE=Memory;

--
-- Populate Minimum Pool
--
	
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Steel%' AND type_swg NOT LIKE '%smelted%' AND type_swg NOT LIKE '%fiberplast%' AND type_swg NOT LIKE '%iron%' AND id != 682 AND id != 683 AND id != 515 ORDER BY RAND() LIMIT 2;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Copper%' AND type_swg NOT LIKE '%smelted%' AND id != 684 ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Aluminum%' AND type_swg NOT LIKE '%smelted%' AND id != 686 ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Extrusive%' ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Intrusive%' ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Carbonate%' ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Crystalline%' ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Armophous%' ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Radioactive%' AND id != 689 ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Solid%' ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Liquid%' ORDER BY RAND() LIMIT 1;
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Polymer%';
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Polymer%';
	INSERT INTO MemMinimumPool SELECT * FROM resource_template WHERE type_name LIKE '%Lubricating%' ORDER BY RAND() LIMIT 2;

--
-- Update the planet IDs
--

	UPDATE MemFixedPool SET planet_id = FLOOR(RAND()*10);
	UPDATE MemRandomPool SET planet_id = FLOOR(RAND()*10) WHERE type_name NOT LIKE '%Water%';
	UPDATE MemMinimumPool SET planet_id = FLOOR(RAND()*10);

--
-- Assign Proper ID range for resources
--

	SELECT MAX(id) from swganh.resources INTO generated_resource_id;

	IF generated_resource_id < 352135131311 THEN SET generated_resource_id = 352135131311;
	END IF;

	IF generated_resource_id IS NULL THEN SET generated_resource_id = 352135131311;
	END IF;


--
-- Copy Resources from temp tables to main table & generate valid stats
--

--
-- Generate Native Resource Pool (Final)
--

OPEN curResourceNative;

REPEAT

	FETCH curResourceNative INTO
		generated_id,
		generated_category_id,
		generated_namefile_name,
		generated_type_name,
		generated_type_swg,
		generated_er_min,
		generated_er_max,
		generated_cr_min,
		generated_cr_max,
		generated_cd_min,
		generated_cd_max,
		generated_dr_min,
		generated_dr_max,
		generated_fl_min,
		generated_fl_max,
		generated_hr_min,
		generated_hr_max,
		generated_ma_min,
		generated_ma_max,
		generated_oq_min,
		generated_oq_max,
		generated_sr_min,
		generated_sr_max,
		generated_ut_min,
		generated_ut_max,
		generated_pe_min,
		generated_pe_max,
		generated_tang,
		generated_type,
		generated_spawn_min,
		generated_spawn_max,
		generated_time_min,
		generated_time_max,
		generated_weight,
		generated_planet_id,
    		generated_bazaar_catid;

	SELECT name FROM resource_name_begin ORDER BY RAND() limit 1 INTO begin_name;
	SELECT name FROM resource_name_end ORDER BY RAND() limit 1 INTO end_name;

	SET final_name = CONCAT(begin_name,end_name);
	SET final_er = rand() * (generated_er_max - generated_er_min) + generated_er_min;
	SET final_cr = rand() * (generated_cr_max - generated_cr_min) + generated_cr_min;
	SET final_cd = rand() * (generated_cd_max - generated_cd_min) + generated_cd_min;
	SET final_dr = rand() * (generated_dr_max - generated_dr_min) + generated_dr_min;
	SET final_fl = rand() * (generated_fl_max - generated_fl_min) + generated_fl_min;
	SET final_hr = rand() * (generated_hr_max - generated_hr_min) + generated_hr_min;
	SET final_ma = rand() * (generated_ma_max - generated_ma_min) + generated_ma_min;
	SET final_oq = rand() * (generated_oq_max - generated_oq_min) + generated_oq_min;
	SET final_sr = rand() * (generated_sr_max - generated_sr_min) + generated_sr_min;
	SET final_ut = rand() * (generated_ut_max - generated_ut_min) + generated_ut_min;
	SET final_pe = rand() * (generated_pe_max - generated_pe_min) + generated_pe_min;
	SET generated_spawn_end = (RAND() * (generated_time_min - generated_time_max) + generated_time_min);
	SET final_spawn_end = DATE_ADD(NOW(), INTERVAL generated_spawn_end DAY);
	
	INSERT INTO swganh.resources VALUES (
		generated_resource_id,
		final_name,
		generated_id,
		final_er,
		final_cr,
		final_cd,
		final_dr,
		final_fl,
		final_hr,
		final_ma,
		final_oq,
		final_sr,
		final_ut,
		final_pe,
		now(),
		final_spawn_end,
		1);

	SET gen_noiseMapBoundsX1 = FLOOR(2 + RAND() * 10);
	SET gen_noiseMapBoundsX2 = gen_noiseMapBoundsX1 + FLOOR(1 + RAND() * 2);
	SET gen_noiseMapBoundsY1 = FLOOR(2 + RAND() * 10);
	SET gen_noiseMapBoundsY2 = gen_noiseMapBoundsY1 + FLOOR(1 + RAND() * 2);
	SET gen_noiseMapOctaves = FLOOR(1 + RAND() * 2);
	SET gen_noiseMapFrequency = 1;
	SET gen_noiseMapPersistence = RAND();
	SET gen_unitsTotal = FLOOR(1 + RAND() * 99999999);
	SET gen_noiseMapScale = RAND();
	SET gen_noiseMapBias = RAND();

	INSERT INTO swganh.resources_spawn_config VALUES (
		generated_resource_id,
		generated_planet_id,
		gen_noiseMapBoundsX1,
		gen_noiseMapBoundsX2,
		gen_noiseMapBoundsY1,
		gen_noiseMapBoundsY2,
		gen_noiseMapOctaves,
		gen_noiseMapFrequency,
		gen_noiseMapPersistence,
		gen_noiseMapScale,
		gen_noiseMapBias,
		gen_unitsTotal,
		gen_unitsTotal);

SET generated_resource_id = generated_resource_id + 1;

UNTIL done END REPEAT;
CLOSE curResourceNative;
SET done = 0;

--
-- Generate Fixed Resource Pool (Final)
--

OPEN curResourceFixed;

REPEAT

	FETCH curResourceFixed INTO
		generated_id,
		generated_category_id,
		generated_namefile_name,
		generated_type_name,
		generated_type_swg,
		generated_er_min,
		generated_er_max,
		generated_cr_min,
		generated_cr_max,
		generated_cd_min,
		generated_cd_max,
		generated_dr_min,
		generated_dr_max,
		generated_fl_min,
		generated_fl_max,
		generated_hr_min,
		generated_hr_max,
		generated_ma_min,
		generated_ma_max,
		generated_oq_min,
		generated_oq_max,
		generated_sr_min,
		generated_sr_max,
		generated_ut_min,
		generated_ut_max,
		generated_pe_min,
		generated_pe_max,
		generated_tang,
		generated_type,
		generated_spawn_min,
		generated_spawn_max,
		generated_time_min,
		generated_time_max,
		generated_weight,
		generated_planet_id,
    generated_bazaar_catid;

		
	SELECT name FROM resource_name_begin ORDER BY RAND() limit 1 INTO begin_name;
	SELECT name FROM resource_name_end ORDER BY RAND() limit 1 INTO end_name;

	SET final_name = CONCAT(begin_name,end_name);
	SET final_er = rand() * (generated_er_max - generated_er_min) + generated_er_min;
	SET final_cr = rand() * (generated_cr_max - generated_cr_min) + generated_cr_min;
	SET final_cd = rand() * (generated_cd_max - generated_cd_min) + generated_cd_min;
	SET final_dr = rand() * (generated_dr_max - generated_dr_min) + generated_dr_min;
	SET final_fl = rand() * (generated_fl_max - generated_fl_min) + generated_fl_min;
	SET final_hr = rand() * (generated_hr_max - generated_hr_min) + generated_hr_min;
	SET final_ma = rand() * (generated_ma_max - generated_ma_min) + generated_ma_min;
	SET final_oq = rand() * (generated_oq_max - generated_oq_min) + generated_oq_min;
	SET final_sr = rand() * (generated_sr_max - generated_sr_min) + generated_sr_min;
	SET final_ut = rand() * (generated_ut_max - generated_ut_min) + generated_ut_min;
	SET final_pe = rand() * (generated_pe_max - generated_pe_min) + generated_pe_min;
	SET generated_spawn_end = (RAND() * (generated_time_min - generated_time_max) + generated_time_min);
	SET final_spawn_end = DATE_ADD(NOW(), INTERVAL generated_spawn_end DAY);


	INSERT INTO swganh.resources VALUES (
		generated_resource_id,
		final_name,
		generated_id,
		final_er,
		final_cr,
		final_cd,
		final_dr,
		final_fl,
		final_hr,
		final_ma,
		final_oq,
		final_sr,
		final_ut,
		final_pe,
		now(),
		final_spawn_end,
		1);

--
-- Generate Muliple Planets for the same resource
--

  	SET loop_counter = 0;
  	SET loop_end = FLOOR(RAND()*10);

  	IF loop_end = 0 THEN SET loop_end = 2;
  	END IF;

	loop1: LOOP

    		loop2: LOOP

    			SET generated_planet_id = FLOOR(RAND()*10);

    			SELECT COUNT(*) FROM swganh.resources_spawn_config WHERE generated_resource_id = resource_id AND planet_id = generated_planet_id INTO planet_check;

    			IF planet_check = 0 THEN LEAVE loop2;
    			END IF;

    		END LOOP loop2;

  SET gen_noiseMapBoundsX1 = FLOOR(2 + RAND() * 10);
	SET gen_noiseMapBoundsX2 = gen_noiseMapBoundsX1 + FLOOR(1 + RAND() * 2);
	SET gen_noiseMapBoundsY1 = FLOOR(2 + RAND() * 10);
	SET gen_noiseMapBoundsY2 = gen_noiseMapBoundsY1 + FLOOR(1 + RAND() * 2);
	SET gen_noiseMapOctaves = FLOOR(1 + RAND() * 2);
	SET gen_noiseMapFrequency = 1;
	SET gen_noiseMapPersistence = RAND();
	SET gen_unitsTotal = FLOOR(1 + RAND() * 99999999);
	SET gen_noiseMapScale = RAND();
	SET gen_noiseMapBias = RAND();

	INSERT INTO swganh.resources_spawn_config VALUES (
		generated_resource_id,
		generated_planet_id,
		gen_noiseMapBoundsX1,
		gen_noiseMapBoundsX2,
		gen_noiseMapBoundsY1,
		gen_noiseMapBoundsY2,
		gen_noiseMapOctaves,
		gen_noiseMapFrequency,
		gen_noiseMapPersistence,
		gen_noiseMapScale,
		gen_noiseMapBias,
		gen_unitsTotal,
		gen_unitsTotal);

  IF loop_counter = loop_end THEN LEAVE loop1;
	  ELSE SET loop_counter = loop_counter + 1;
	END IF;

	END LOOP loop1;

  SET generated_resource_id = generated_resource_id + 1;

  UNTIL done END REPEAT;
	CLOSE curResourceFixed;

SET done = 0;

--
-- Generate Random Resource Pool (Final)
--

OPEN curResourceRandom;

REPEAT

	FETCH curResourceRandom INTO
		generated_id,
		generated_category_id,
		generated_namefile_name,
		generated_type_name,
		generated_type_swg,
		generated_er_min,
		generated_er_max,
		generated_cr_min,
		generated_cr_max,
		generated_cd_min,
		generated_cd_max,
		generated_dr_min,
		generated_dr_max,
		generated_fl_min,
		generated_fl_max,
		generated_hr_min,
		generated_hr_max,
		generated_ma_min,
		generated_ma_max,
		generated_oq_min,
		generated_oq_max,
		generated_sr_min,
		generated_sr_max,
		generated_ut_min,
		generated_ut_max,
		generated_pe_min,
		generated_pe_max,
		generated_tang,
		generated_type,
		generated_spawn_min,
		generated_spawn_max,
		generated_time_min,
		generated_time_max,
		generated_weight,
		generated_planet_id,
    generated_bazaar_catid;


	SELECT name FROM resource_name_begin ORDER BY RAND() limit 1 INTO begin_name;
	SELECT name FROM resource_name_end ORDER BY RAND() limit 1 INTO end_name;

	SET final_name = CONCAT(begin_name,end_name);
	SET final_er = rand() * (generated_er_max - generated_er_min) + generated_er_min;
	SET final_cr = rand() * (generated_cr_max - generated_cr_min) + generated_cr_min;
	SET final_cd = rand() * (generated_cd_max - generated_cd_min) + generated_cd_min;
	SET final_dr = rand() * (generated_dr_max - generated_dr_min) + generated_dr_min;
	SET final_fl = rand() * (generated_fl_max - generated_fl_min) + generated_fl_min;
	SET final_hr = rand() * (generated_hr_max - generated_hr_min) + generated_hr_min;
	SET final_ma = rand() * (generated_ma_max - generated_ma_min) + generated_ma_min;
	SET final_oq = rand() * (generated_oq_max - generated_oq_min) + generated_oq_min;
	SET final_sr = rand() * (generated_sr_max - generated_sr_min) + generated_sr_min;
	SET final_ut = rand() * (generated_ut_max - generated_ut_min) + generated_ut_min;
	SET final_pe = rand() * (generated_pe_max - generated_pe_min) + generated_pe_min;
	SET generated_spawn_end = (RAND() * (generated_time_min - generated_time_max) + generated_time_min);
	SET final_spawn_end = DATE_ADD(NOW(), INTERVAL generated_spawn_end DAY);

	INSERT INTO swganh.resources VALUES (
		generated_resource_id,
		final_name,
		generated_id,
		final_er,
		final_cr,
		final_cd,
		final_dr,
		final_fl,
		final_hr,
		final_ma,
		final_oq,
		final_sr,
		final_ut,
		final_pe,
		now(),
		final_spawn_end,
		1);

--
-- Generate Muliple Planets for the same resource
--

  	SET loop_counter = 0;
  	SET loop_end = FLOOR(RAND()*10);

  	IF loop_end = 0 THEN SET loop_end = 2;
  	END IF;

	loop1: LOOP

    		loop2: LOOP

    			SET generated_planet_id = FLOOR(RAND()*10);

    			SELECT COUNT(*) FROM swganh.resources_spawn_config WHERE generated_resource_id = resource_id AND planet_id = generated_planet_id INTO planet_check;

    			IF planet_check = 0 THEN LEAVE loop2;
    			END IF;

    		END LOOP loop2;

  SET gen_noiseMapBoundsX1 = FLOOR(2 + RAND() * 10);
	SET gen_noiseMapBoundsX2 = gen_noiseMapBoundsX1 + FLOOR(1 + RAND() * 2);
	SET gen_noiseMapBoundsY1 = FLOOR(2 + RAND() * 10);
	SET gen_noiseMapBoundsY2 = gen_noiseMapBoundsY1 + FLOOR(1 + RAND() * 2);
	SET gen_noiseMapOctaves = FLOOR(1 + RAND() * 2);
	SET gen_noiseMapFrequency = 1;
	SET gen_noiseMapPersistence = RAND();
	SET gen_unitsTotal = FLOOR(1 + RAND() * 99999999);
	SET gen_noiseMapScale = RAND();
	SET gen_noiseMapBias = RAND();

	INSERT INTO swganh.resources_spawn_config VALUES (
		generated_resource_id,
		generated_planet_id,
		gen_noiseMapBoundsX1,
		gen_noiseMapBoundsX2,
		gen_noiseMapBoundsY1,
		gen_noiseMapBoundsY2,
		gen_noiseMapOctaves,
		gen_noiseMapFrequency,
		gen_noiseMapPersistence,
		gen_noiseMapScale,
		gen_noiseMapBias,
		gen_unitsTotal,
		gen_unitsTotal);

  IF loop_counter = loop_end THEN LEAVE loop1;
	  ELSE SET loop_counter = loop_counter + 1;
	END IF;

	END LOOP loop1;

  SET generated_resource_id = generated_resource_id + 1;

  UNTIL done END REPEAT;
	CLOSE curResourceRandom;

SET done = 0;

--
-- Generate Minimum Resource Pool (Final)
--

OPEN curResourceMinimum;

REPEAT

	FETCH curResourceMinimum INTO
		generated_id,
		generated_category_id,
		generated_namefile_name,
		generated_type_name,
		generated_type_swg,
		generated_er_min,
		generated_er_max,
		generated_cr_min,
		generated_cr_max,
		generated_cd_min,
		generated_cd_max,
		generated_dr_min,
		generated_dr_max,
		generated_fl_min,
		generated_fl_max,
		generated_hr_min,
		generated_hr_max,
		generated_ma_min,
		generated_ma_max,
		generated_oq_min,
		generated_oq_max,
		generated_sr_min,
		generated_sr_max,
		generated_ut_min,
		generated_ut_max,
		generated_pe_min,
		generated_pe_max,
		generated_tang,
		generated_type,
		generated_spawn_min,
		generated_spawn_max,
		generated_time_min,
		generated_time_max,
		generated_weight,
		generated_planet_id,
    		generated_bazaar_catid;

		
	SELECT name FROM resource_name_begin ORDER BY RAND() limit 1 INTO begin_name;
	SELECT name FROM resource_name_end ORDER BY RAND() limit 1 INTO end_name;

	SET final_name = CONCAT(begin_name,end_name);
	SET final_er = rand() * (generated_er_max - generated_er_min) + generated_er_min;
	SET final_cr = rand() * (generated_cr_max - generated_cr_min) + generated_cr_min;
	SET final_cd = rand() * (generated_cd_max - generated_cd_min) + generated_cd_min;
	SET final_dr = rand() * (generated_dr_max - generated_dr_min) + generated_dr_min;
	SET final_fl = rand() * (generated_fl_max - generated_fl_min) + generated_fl_min;
	SET final_hr = rand() * (generated_hr_max - generated_hr_min) + generated_hr_min;
	SET final_ma = rand() * (generated_ma_max - generated_ma_min) + generated_ma_min;
	SET final_oq = rand() * (generated_oq_max - generated_oq_min) + generated_oq_min;
	SET final_sr = rand() * (generated_sr_max - generated_sr_min) + generated_sr_min;
	SET final_ut = rand() * (generated_ut_max - generated_ut_min) + generated_ut_min;
	SET final_pe = rand() * (generated_pe_max - generated_pe_min) + generated_pe_min;
	SET generated_spawn_end = (RAND() * (generated_time_min - generated_time_max) + generated_time_min);
	SET final_spawn_end = DATE_ADD(NOW(), INTERVAL generated_spawn_end DAY);

	INSERT INTO swganh.resources VALUES (
		generated_resource_id,
		final_name,
		generated_id,
		final_er,
		final_cr,
		final_cd,
		final_dr,
		final_fl,
		final_hr,
		final_ma,
		final_oq,
		final_sr,
		final_ut,
		final_pe,
		now(),
		final_spawn_end,
		1);

--
-- Generate Muliple Planets for the same resource
--

  	SET loop_counter = 0;
  	SET loop_end = FLOOR(RAND()*10);

  	IF loop_end = 0 THEN SET loop_end = 2;
  	END IF;

	loop1: LOOP

    		loop2: LOOP

    			SET generated_planet_id = FLOOR(RAND()*10);

    			SELECT COUNT(*) FROM swganh.resources_spawn_config WHERE generated_resource_id = resource_id AND planet_id = generated_planet_id INTO planet_check;

    			IF planet_check = 0 THEN LEAVE loop2;
    			END IF;

    		END LOOP loop2;

  	SET gen_noiseMapBoundsX1 = FLOOR(2 + RAND() * 10);
	SET gen_noiseMapBoundsX2 = gen_noiseMapBoundsX1 + FLOOR(1 + RAND() * 2);
	SET gen_noiseMapBoundsY1 = FLOOR(2 + RAND() * 10);
	SET gen_noiseMapBoundsY2 = gen_noiseMapBoundsY1 + FLOOR(1 + RAND() * 2);
	SET gen_noiseMapOctaves = FLOOR(1 + RAND() * 2);
	SET gen_noiseMapFrequency = 1;
	SET gen_noiseMapPersistence = RAND();
	SET gen_unitsTotal = FLOOR(1 + RAND() * 99999999);
	SET gen_noiseMapScale = RAND();
	SET gen_noiseMapBias = RAND();

	INSERT INTO swganh.resources_spawn_config VALUES (
		generated_resource_id,
		generated_planet_id,
		gen_noiseMapBoundsX1,
		gen_noiseMapBoundsX2,
		gen_noiseMapBoundsY1,
		gen_noiseMapBoundsY2,
		gen_noiseMapOctaves,
		gen_noiseMapFrequency,
		gen_noiseMapPersistence,
		gen_noiseMapScale,
		gen_noiseMapBias,
		gen_unitsTotal,
		gen_unitsTotal);

  IF loop_counter = loop_end THEN LEAVE loop1;
	  ELSE SET loop_counter = loop_counter + 1;
	END IF;

	END LOOP loop1;

  SET generated_resource_id = generated_resource_id + 1;

  UNTIL done END REPEAT;
	CLOSE curResourceMinimum;

SET done = 0;

--
-- Cleanup
--

DROP TABLE IF EXISTS `MemNativePool`;
DROP TABLE IF EXISTS `MemFixedPool`;
DROP TABLE IF EXISTS `MemRandomPool`;
DROP TABLE IF EXISTS `MemMinimumPool`;

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
