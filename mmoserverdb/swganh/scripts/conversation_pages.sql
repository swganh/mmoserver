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
-- Definition of table `conversation_pages`
--

DROP TABLE IF EXISTS `conversation_pages`;
CREATE TABLE `conversation_pages` (
  `conversation_id` int(10) unsigned NOT NULL default '0',
  `page` int(10) unsigned NOT NULL default '0',
  `customText` varchar(512) NOT NULL,
  `stfFile` char(255) NOT NULL,
  `stfVariable` char(255) NOT NULL,
  `option_batch` int(10) unsigned default NULL,
  `animation` int(10) unsigned default NULL,
  PRIMARY KEY  (`conversation_id`,`page`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `conversation_pages`
--

/*!40000 ALTER TABLE `conversation_pages` DISABLE KEYS */;
INSERT INTO `conversation_pages` (`conversation_id`,`page`,`customText`,`stfFile`,`stfVariable`,`option_batch`,`animation`) VALUES 
 (1,1,'','skill_teacher','trainer_brawler',1,2),
 (1,2,'','skill_teacher','msg2_1',2,46),
 (1,3,'','skill_teacher','prose_cost',67,46),
 (1,4,'','skill_teacher','msg2_2',68,46),
 (1,5,'','skill_teacher','msg_yes',1,46),		-- Yes, accepted training
 (1,6,'','skill_teacher','msg_no',1,46),		-- No, leaving conversation
 (1,7,'','skill_teacher','msg3_3',1,46),
 (1,8,'','skill_teacher','error_empty_category',1,46),
 (1,9,'','skill_teacher','topped_out',1,46),	-- Already Master
 (1,10,'','skill_teacher','nsf_skill_pts',1,46),	-- Lack of skill points
 (1,11,'','skill_teacher','msg1_1',1,46),		-- Lack of credits
 (1,12,'','skill_teacher','no_qualify',1,46),	-- No skill requirements
 (1,13,'','skill_teacher','surpass_trainer',1,46),	-- Trained to Master.
 (1,14,'','skill_teacher','trainer_brawler',1,46),	-- Restart without animation.
 (1,15,'','skill_teacher','msg3_2',1,46),		-- Yes, accepted training in the Tutorial
 (2,1,'','skill_teacher','trainer_entertainer',3,2),
 (2,2,'','skill_teacher','msg2_1',4,46),
 (2,3,'','skill_teacher','prose_cost',67,46),
 (2,4,'','skill_teacher','msg2_2',69,46),
 (2,5,'','skill_teacher','msg_yes',3,46),		-- Yes, accepted training
 (2,6,'','skill_teacher','msg_no',3,46),		-- No, leaving conversation
 (2,7,'','skill_teacher','msg3_3',3,46),
 (2,8,'','skill_teacher','error_empty_category',3,46),
 (2,9,'','skill_teacher','topped_out',3,46),	-- Already Master
 (2,10,'','skill_teacher','nsf_skill_pts',3,46),	-- Lack of skill points
 (2,11,'','skill_teacher','msg1_1',3,46),		-- Lack of credits
 (2,12,'','skill_teacher','no_qualify',3,46),	-- No skill requirements
 (2,13,'','skill_teacher','surpass_trainer',3,46),	-- Trained to Master.
 (2,14,'','skill_teacher','trainer_entertainer',3,46),
 (2,15,'','skill_teacher','msg3_2',3,46),		-- Yes, accepted training in the Tutorial 
 (3,1,'','skill_teacher','trainer_scout',5,2),
 (3,2,'','skill_teacher','msg2_1',6,46),
 (3,3,'','skill_teacher','prose_cost',67,46),
 (3,4,'','skill_teacher','msg2_2',70,46),
 (3,5,'','skill_teacher','msg_yes',5,46),		-- Yes, accepted training
 (3,6,'','skill_teacher','msg_no',5,46),		-- No, leaving conversation
 (3,7,'','skill_teacher','msg3_3',5,46),
 (3,8,'','skill_teacher','error_empty_category',5,46),
 (3,9,'','skill_teacher','topped_out',5,46),	-- Already Master
 (3,10,'','skill_teacher','nsf_skill_pts',5,46),	-- Lack of skill points
 (3,11,'','skill_teacher','msg1_1',5,46),		-- Lack of credits
 (3,12,'','skill_teacher','no_qualify',5,46),	-- No skill requirements
 (3,13,'','skill_teacher','surpass_trainer',5,46),	-- Trained to Master.
 (3,14,'','skill_teacher','trainer_scout',5,46),
 (3,15,'','skill_teacher','msg3_2',5,46),		-- Yes, accepted training in the Tutorial  
 (4,1,'','skill_teacher','trainer_medic',7,2),
 (4,2,'','skill_teacher','msg2_1',8,46),
 (4,3,'','skill_teacher','prose_cost',67,46),
 (4,4,'','skill_teacher','msg2_2',71,46),			-- Change option batch here, new number
 (4,5,'','skill_teacher','msg_yes',7,46),			-- Change option batch here, same as the trainer at #1
 (4,6,'','skill_teacher','msg_no',7,46),			-- Change option batch here, same as the trainer at #1
 (4,7,'','skill_teacher','msg3_3',7,46),			-- Change option batch here, same as the trainer at #1	
 (4,8,'','skill_teacher','error_empty_category',7,46),	-- Change option batch here, same as the trainer at #1	
 (4,9,'','skill_teacher','topped_out',7,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (4,10,'','skill_teacher','nsf_skill_pts',7,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (4,11,'','skill_teacher','msg1_1',7,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (4,12,'','skill_teacher','no_qualify',7,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (4,13,'','skill_teacher','surpass_trainer',7,46),		-- Trained to Master.
 (4,14,'','skill_teacher','trainer_medic',7,46),
 (4,15,'','skill_teacher','msg3_2',7,46),		-- Yes, accepted training in the Tutorial  
 (5,1,'','skill_teacher','trainer_artisan',9,2),
 (5,2,'','skill_teacher','msg2_1',10,46),
 (5,3,'','skill_teacher','prose_cost',67,46),
 (5,4,'','skill_teacher','msg2_2',72,46),
 (5,5,'','skill_teacher','msg_yes',9,46),
 (5,6,'','skill_teacher','msg_no',9,46),
 (5,7,'','skill_teacher','msg3_3',9,46),			-- Change option batch here, same as the trainer at #1	
 (5,8,'','skill_teacher','error_empty_category',9,46),	-- Change option batch here, same as the trainer at #1	
 (5,9,'','skill_teacher','topped_out',9,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (5,10,'','skill_teacher','nsf_skill_pts',9,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (5,11,'','skill_teacher','msg1_1',9,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (5,12,'','skill_teacher','no_qualify',9,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (5,13,'','skill_teacher','surpass_trainer',9,46),		-- Trained to Master.
 (5,14,'','skill_teacher','trainer_artisan',9,46),
 (5,15,'','skill_teacher','msg3_2',9,46),		-- Yes, accepted training in the Tutorial  
 (6,1,'','skill_teacher','trainer_marksman',11,2),
 (6,2,'','skill_teacher','msg2_1',12,46),
 (6,3,'','skill_teacher','prose_cost',67,46),
 (6,4,'','skill_teacher','msg2_2',73,46),
 (6,5,'','skill_teacher','msg_yes',11,46),
 (6,6,'','skill_teacher','msg_no',11,46),
 (6,7,'','skill_teacher','msg3_3',11,46),			-- Change option batch here, same as the trainer at #1	
 (6,8,'','skill_teacher','error_empty_category',11,46),	-- Change option batch here, same as the trainer at #1	
 (6,9,'','skill_teacher','topped_out',11,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (6,10,'','skill_teacher','nsf_skill_pts',11,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (6,11,'','skill_teacher','msg1_1',11,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (6,12,'','skill_teacher','no_qualify',11,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (6,13,'','skill_teacher','surpass_trainer',11,46),		-- Trained to Master.
 (6,14,'','skill_teacher','trainer_marksman',11,46),
 (6,15,'','skill_teacher','msg3_2',11,46),		-- Yes, accepted training in the Tutorial 
 (7,1,'','skill_teacher','trainer_rifleman',13,2),
 (7,2,'','skill_teacher','msg2_1',14,46),
 (7,3,'','skill_teacher','prose_cost',67,46),
 (7,4,'','skill_teacher','msg2_2',74,46),
 (7,5,'','skill_teacher','msg_yes',13,46),
 (7,6,'','skill_teacher','msg_no',13,46),
 (7,7,'','skill_teacher','msg3_3',13,46),			-- Change option batch here, same as the trainer at #1	
 (7,8,'','skill_teacher','error_empty_category',13,46),	-- Change option batch here, same as the trainer at #1	
 (7,9,'','skill_teacher','topped_out',13,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (7,10,'','skill_teacher','nsf_skill_pts',13,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (7,11,'','skill_teacher','msg1_1',13,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (7,12,'','skill_teacher','no_qualify',13,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (7,13,'','skill_teacher','surpass_trainer',13,46),		-- Trained to Master.
 (7,14,'','skill_teacher','trainer_rifleman',13,46),
 (8,1,'','skill_teacher','trainer_pistol',15,2),
 (8,2,'','skill_teacher','msg2_1',16,46),
 (8,3,'','skill_teacher','prose_cost',67,46),
 (8,4,'','skill_teacher','msg2_2',75,46),
 (8,5,'','skill_teacher','msg_yes',15,46),
 (8,6,'','skill_teacher','msg_no',15,46),
 (8,7,'','skill_teacher','msg3_3',15,46),			-- Change option batch here, same as the trainer at #1	
 (8,8,'','skill_teacher','error_empty_category',15,46),	-- Change option batch here, same as the trainer at #1	
 (8,9,'','skill_teacher','topped_out',15,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (8,10,'','skill_teacher','nsf_skill_pts',15,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (8,11,'','skill_teacher','msg1_1',15,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (8,12,'','skill_teacher','no_qualify',15,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (8,13,'','skill_teacher','surpass_trainer',15,46),		-- Trained to Master.
 (8,14,'','skill_teacher','trainer_pistol',15,46),
 (9,1,'','skill_teacher','trainer_carbine',17,2),
 (9,2,'','skill_teacher','msg2_1',18,46),
 (9,3,'','skill_teacher','prose_cost',67,46),
 (9,4,'','skill_teacher','msg2_2',76,46),
 (9,5,'','skill_teacher','msg_yes',17,46),
 (9,6,'','skill_teacher','msg_no',17,46),
 (9,7,'','skill_teacher','msg3_3',17,46),			-- Change option batch here, same as the trainer at #1	
 (9,8,'','skill_teacher','error_empty_category',17,46),	-- Change option batch here, same as the trainer at #1	
 (9,9,'','skill_teacher','topped_out',17,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (9,10,'','skill_teacher','nsf_skill_pts',17,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (9,11,'','skill_teacher','msg1_1',17,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (9,12,'','skill_teacher','no_qualify',17,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (9,13,'','skill_teacher','surpass_trainer',17,46),		-- Trained to Master.
 (9,14,'','skill_teacher','trainer_carbine',17,46),
 (10,1,'','skill_teacher','trainer_unarmed',19,2),
 (10,2,'','skill_teacher','msg2_1',20,46),
 (10,3,'','skill_teacher','prose_cost',67,46),
 (10,4,'','skill_teacher','msg2_2',77,46),
 (10,5,'','skill_teacher','msg_yes',19,46),
 (10,6,'','skill_teacher','msg_no',19,46),
 (10,7,'','skill_teacher','msg3_3',19,46),			-- Change option batch here, same as the trainer at #1	
 (10,8,'','skill_teacher','error_empty_category',19,46),	-- Change option batch here, same as the trainer at #1	
 (10,9,'','skill_teacher','topped_out',19,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (10,10,'','skill_teacher','nsf_skill_pts',19,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (10,11,'','skill_teacher','msg1_1',19,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (10,12,'','skill_teacher','no_qualify',19,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (10,13,'','skill_teacher','surpass_trainer',19,46),		-- Trained to Master.
 (10,14,'','skill_teacher','trainer_unarmed',19,46),
 (11,1,'','skill_teacher','trainer_1hsword',21,2),
 (11,2,'','skill_teacher','msg2_1',22,46),
 (11,3,'','skill_teacher','prose_cost',67,46),
 (11,4,'','skill_teacher','msg2_2',78,46),
 (11,5,'','skill_teacher','msg_yes',21,46),
 (11,6,'','skill_teacher','msg_no',21,46),
 (11,7,'','skill_teacher','msg3_3',21,46),			-- Change option batch here, same as the trainer at #1	
 (11,8,'','skill_teacher','error_empty_category',21,46),	-- Change option batch here, same as the trainer at #1	
 (11,9,'','skill_teacher','topped_out',21,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (11,10,'','skill_teacher','nsf_skill_pts',21,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (11,11,'','skill_teacher','msg1_1',21,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (11,12,'','skill_teacher','no_qualify',21,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (11,13,'','skill_teacher','surpass_trainer',21,46),		-- Trained to Master.
 (11,14,'','skill_teacher','trainer_1hsword',21,46),
 (12,1,'','skill_teacher','trainer_2hsword',23,2),
 (12,2,'','skill_teacher','msg2_1',24,46),
 (12,3,'','skill_teacher','prose_cost',67,46),
 (12,4,'','skill_teacher','msg2_2',79,46),
 (12,5,'','skill_teacher','msg_yes',23,46),
 (12,6,'','skill_teacher','msg_no',23,46),
 (12,7,'','skill_teacher','msg3_3',23,46),			-- Change option batch here, same as the trainer at #1	
 (12,8,'','skill_teacher','error_empty_category',23,46),	-- Change option batch here, same as the trainer at #1	
 (12,9,'','skill_teacher','topped_out',23,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (12,10,'','skill_teacher','nsf_skill_pts',23,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (12,11,'','skill_teacher','msg1_1',23,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (12,12,'','skill_teacher','no_qualify',23,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (12,13,'','skill_teacher','surpass_trainer',23,46),		-- Trained to Master.
 (12,14,'','skill_teacher','trainer_2hsword',23,46),
 (13,1,'','skill_teacher','trainer_polearm',25,2),
 (13,2,'','skill_teacher','msg2_1',26,46),
 (13,3,'','skill_teacher','prose_cost',67,46),
 (13,4,'','skill_teacher','msg2_2',80,46),
 (13,5,'','skill_teacher','msg_yes',25,46),
 (13,6,'','skill_teacher','msg_no',25,46),
 (13,7,'','skill_teacher','msg3_3',25,46),			-- Change option batch here, same as the trainer at #1	
 (13,8,'','skill_teacher','error_empty_category',25,46),	-- Change option batch here, same as the trainer at #1	
 (13,9,'','skill_teacher','topped_out',25,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (13,10,'','skill_teacher','nsf_skill_pts',25,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (13,11,'','skill_teacher','msg1_1',25,46),			-- Lack of credits, change option batch here, same as the trainer at #1	
 (13,12,'','skill_teacher','no_qualify',25,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (13,13,'','skill_teacher','surpass_trainer',25,46),		-- Trained to Master.
 (13,14,'','skill_teacher','trainer_polearm',25,46),
 (14,1,'','skill_teacher','trainer_dancer',27,2),
 (14,2,'','skill_teacher','msg2_1',28,46),
 (14,3,'','skill_teacher','prose_cost',67,46),
 (14,4,'','skill_teacher','msg2_2',81,46),
 (14,5,'','skill_teacher','msg_yes',27,46),
 (14,6,'','skill_teacher','msg_no',27,46),
 (14,7,'','skill_teacher','msg3_3',27,46),			-- Change option batch here, same as the trainer at #1	
 (14,8,'','skill_teacher','error_empty_category',27,46),	-- Change option batch here, same as the trainer at #1	
 (14,9,'','skill_teacher','topped_out',27,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (14,10,'','skill_teacher','nsf_skill_pts',27,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (14,11,'','skill_teacher','msg1_1',27,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (14,12,'','skill_teacher','no_qualify',27,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (14,13,'','skill_teacher','surpass_trainer',27,46),		-- Trained to Master.
 (14,14,'','skill_teacher','trainer_dancer',27,46),
 (15,1,'','skill_teacher','trainer_musician',29,2),
 (15,2,'','skill_teacher','msg2_1',30,46),
 (15,3,'','skill_teacher','prose_cost',67,46),
 (15,4,'','skill_teacher','msg2_2',82,46),
 (15,5,'','skill_teacher','msg_yes',29,46),
 (15,6,'','skill_teacher','msg_no',29,46),
 (15,7,'','skill_teacher','msg3_3',29,46),			-- Change option batch here, same as the trainer at #1	
 (15,8,'','skill_teacher','error_empty_category',29,46),	-- Change option batch here, same as the trainer at #1	
 (15,9,'','skill_teacher','topped_out',29,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (15,10,'','skill_teacher','nsf_skill_pts',29,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (15,11,'','skill_teacher','msg1_1',29,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (15,12,'','skill_teacher','no_qualify',29,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (15,13,'','skill_teacher','surpass_trainer',29,46),		-- Trained to Master.
 (15,14,'','skill_teacher','trainer_musician',29,46),
 (16,1,'','skill_teacher','trainer_doctor',31,2),
 (16,2,'','skill_teacher','msg2_1',32,46),
 (16,3,'','skill_teacher','prose_cost',67,46),
 (16,4,'','skill_teacher','msg2_2',83,46),
 (16,5,'','skill_teacher','msg_yes',31,46),
 (16,6,'','skill_teacher','msg_no',31,46),
 (16,7,'','skill_teacher','msg3_3',31,46),			-- Change option batch here, same as the trainer at #1	
 (16,8,'','skill_teacher','error_empty_category',31,46),	-- Change option batch here, same as the trainer at #1	
 (16,9,'','skill_teacher','topped_out',31,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (16,10,'','skill_teacher','nsf_skill_pts',31,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (16,11,'','skill_teacher','msg1_1',31,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (16,12,'','skill_teacher','no_qualify',31,46),		-- No skill requirements, change option batch here, same as the trainer at #1
 (16,13,'','skill_teacher','surpass_trainer',31,46),		-- Trained to Master.	
 (16,14,'','skill_teacher','trainer_doctor',31,46),
 (17,1,'','skill_teacher','trainer_ranger',33,2),
 (17,2,'','skill_teacher','msg2_1',34,46),
 (17,3,'','skill_teacher','prose_cost',67,46),
 (17,4,'','skill_teacher','msg2_2',84,46),
 (17,5,'','skill_teacher','msg_yes',33,46),
 (17,6,'','skill_teacher','msg_no',33,46),
 (17,7,'','skill_teacher','msg3_3',33,46),			-- Change option batch here, same as the trainer at #1	
 (17,8,'','skill_teacher','error_empty_category',33,46),	-- Change option batch here, same as the trainer at #1	
 (17,9,'','skill_teacher','topped_out',33,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (17,10,'','skill_teacher','nsf_skill_pts',33,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (17,11,'','skill_teacher','msg1_1',33,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (17,12,'','skill_teacher','no_qualify',33,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (17,13,'','skill_teacher','surpass_trainer',33,46),		-- Trained to Master.
 (17,14,'','skill_teacher','trainer_ranger',33,46),
 (18,1,'','skill_teacher','trainer_creaturehandler',35,2),
 (18,2,'','skill_teacher','msg2_1',36,46),
 (18,3,'','skill_teacher','prose_cost',67,46),
 (18,4,'','skill_teacher','msg2_2',85,46),
 (18,5,'','skill_teacher','msg_yes',35,46),
 (18,6,'','skill_teacher','msg_no',35,46),
 (18,7,'','skill_teacher','msg3_3',35,46),			-- Change option batch here, same as the trainer at #1	
 (18,8,'','skill_teacher','error_empty_category',35,46),	-- Change option batch here, same as the trainer at #1	
 (18,9,'','skill_teacher','topped_out',35,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (18,10,'','skill_teacher','nsf_skill_pts',35,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (18,11,'','skill_teacher','msg1_1',35,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (18,12,'','skill_teacher','no_qualify',35,46),		-- No skill requirements, change option batch here, same as the trainer at #1
 (18,13,'','skill_teacher','surpass_trainer',35,46),		-- Trained to Master.	
 (18,14,'','skill_teacher','trainer_creaturehandler',35,46),
 (19,1,'','skill_teacher','trainer_bioengineer',37,2),
 (19,2,'','skill_teacher','msg2_1',38,46),
 (19,3,'','skill_teacher','prose_cost',67,46),
 (19,4,'','skill_teacher','msg2_2',86,46),
 (19,5,'','skill_teacher','msg_yes',37,46),
 (19,6,'','skill_teacher','msg_no',37,46),
 (19,7,'','skill_teacher','msg3_3',37,46),			-- Change option batch here, same as the trainer at #1	
 (19,8,'','skill_teacher','error_empty_category',37,46),	-- Change option batch here, same as the trainer at #1	
 (19,9,'','skill_teacher','topped_out',37,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (19,10,'','skill_teacher','nsf_skill_pts',37,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (19,11,'','skill_teacher','msg1_1',37,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (19,12,'','skill_teacher','no_qualify',37,46),		-- No skill requirements, change option batch here, same as the trainer at #1
 (19,13,'','skill_teacher','surpass_trainer',37,46),		-- Trained to Master.	
 (19,14,'','skill_teacher','trainer_bioengineer',37,46),
 (20,1,'','skill_teacher','trainer_armorsmith',39,2),
 (20,2,'','skill_teacher','msg2_1',40,46),
 (20,3,'','skill_teacher','prose_cost',67,46),
 (20,4,'','skill_teacher','msg2_2',87,46),
 (20,5,'','skill_teacher','msg_yes',39,46),
 (20,6,'','skill_teacher','msg_no',39,46),
 (20,7,'','skill_teacher','msg3_3',39,46),			-- Change option batch here, same as the trainer at #1	
 (20,8,'','skill_teacher','error_empty_category',39,46),	-- Change option batch here, same as the trainer at #1	
 (20,9,'','skill_teacher','topped_out',39,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (20,10,'','skill_teacher','nsf_skill_pts',39,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (20,11,'','skill_teacher','msg1_1',39,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (20,12,'','skill_teacher','no_qualify',39,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (20,13,'','skill_teacher','surpass_trainer',39,46),		-- Trained to Master.
 (20,14,'','skill_teacher','trainer_armorsmith',39,46),
 (21,1,'','skill_teacher','trainer_weaponsmith',41,2),
 (21,2,'','skill_teacher','msg2_1',42,46),
 (21,3,'','skill_teacher','prose_cost',67,46),
 (21,4,'','skill_teacher','msg2_2',88,46),
 (21,5,'','skill_teacher','msg_yes',41,46),
 (21,6,'','skill_teacher','msg_no',41,46),
 (21,7,'','skill_teacher','msg3_3',41,46),			-- Change option batch here, same as the trainer at #1	
 (21,8,'','skill_teacher','error_empty_category',41,46),	-- Change option batch here, same as the trainer at #1	
 (21,9,'','skill_teacher','topped_out',41,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (21,10,'','skill_teacher','nsf_skill_pts',41,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (21,11,'','skill_teacher','msg1_1',41,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (21,12,'','skill_teacher','no_qualify',41,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (21,13,'','skill_teacher','surpass_trainer',41,46),		-- Trained to Master.
 (21,14,'','skill_teacher','trainer_weaponsmith',41,46),
 (22,1,'','skill_teacher','trainer_chef',43,2),
 (22,2,'','skill_teacher','msg2_1',44,46),
 (22,3,'','skill_teacher','prose_cost',67,46),
 (22,4,'','skill_teacher','msg2_2',89,46),
 (22,5,'','skill_teacher','msg_yes',43,46),
 (22,6,'','skill_teacher','msg_no',43,46),
 (22,7,'','skill_teacher','msg3_3',43,46),			-- Change option batch here, same as the trainer at #1	
 (22,8,'','skill_teacher','error_empty_category',43,46),	-- Change option batch here, same as the trainer at #1	
 (22,9,'','skill_teacher','topped_out',43,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (22,10,'','skill_teacher','nsf_skill_pts',43,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (22,11,'','skill_teacher','msg1_1',43,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (22,12,'','skill_teacher','no_qualify',43,46),		-- No skill requirements, change option batch here, same as the trainer at #1
 (22,13,'','skill_teacher','surpass_trainer',43,46),		-- Trained to Master.	
 (22,14,'','skill_teacher','trainer_chef',43,46),
 (23,1,'','skill_teacher','trainer_tailor',45,2),
 (23,2,'','skill_teacher','msg2_1',46,46),
 (23,3,'','skill_teacher','prose_cost',67,46),
 (23,4,'','skill_teacher','msg2_2',90,46),
 (23,5,'','skill_teacher','msg_yes',45,46),
 (23,6,'','skill_teacher','msg_no',45,46),
 (23,7,'','skill_teacher','msg3_3',45,46),			-- Change option batch here, same as the trainer at #1	
 (23,8,'','skill_teacher','error_empty_category',45,46),	-- Change option batch here, same as the trainer at #1	
 (23,9,'','skill_teacher','topped_out',45,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (23,10,'','skill_teacher','nsf_skill_pts',45,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (23,11,'','skill_teacher','msg1_1',45,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (23,12,'','skill_teacher','no_qualify',45,46),		-- No skill requirements, change option batch here, same as the trainer at #1
 (23,13,'','skill_teacher','surpass_trainer',45,46),		-- Trained to Master.	
 (23,14,'','skill_teacher','trainer_tailor',45,46),
 (24,1,'','skill_teacher','trainer_architect',47,2),
 (24,2,'','skill_teacher','msg2_1',48,46),
 (24,3,'','skill_teacher','prose_cost',67,46),
 (24,4,'','skill_teacher','msg2_2',91,46),
 (24,5,'','skill_teacher','msg_yes',47,46),
 (24,6,'','skill_teacher','msg_no',47,46),
 (24,7,'','skill_teacher','msg3_3',47,46),			-- Change option batch here, same as the trainer at #1	
 (24,8,'','skill_teacher','error_empty_category',47,46),	-- Change option batch here, same as the trainer at #1	
 (24,9,'','skill_teacher','topped_out',47,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (24,10,'','skill_teacher','nsf_skill_pts',47,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (24,11,'','skill_teacher','msg1_1',47,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (24,12,'','skill_teacher','no_qualify',47,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (24,13,'','skill_teacher','surpass_trainer',47,46),		-- Trained to Master.
 (24,14,'','skill_teacher','trainer_architect',47,46),
 (25,1,'','skill_teacher','trainer_droidengineer',49,2),
 (25,2,'','skill_teacher','msg2_1',50,46),
 (25,3,'','skill_teacher','prose_cost',67,46),
 (25,4,'','skill_teacher','msg2_2',92,46),
 (25,5,'','skill_teacher','msg_yes',49,46),
 (25,6,'','skill_teacher','msg_no',49,46),
 (25,7,'','skill_teacher','msg3_3',49,46),			-- Change option batch here, same as the trainer at #1	
 (25,8,'','skill_teacher','error_empty_category',49,46),	-- Change option batch here, same as the trainer at #1	
 (25,9,'','skill_teacher','topped_out',49,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (25,10,'','skill_teacher','nsf_skill_pts',49,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (25,11,'','skill_teacher','msg1_1',49,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (25,12,'','skill_teacher','no_qualify',49,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (25,13,'','skill_teacher','surpass_trainer',49,46),		-- Trained to Master.
 (25,14,'','skill_teacher','trainer_droidengineer',49,46),
 (26,1,'','skill_teacher','trainer_merchant',51,2),
 (26,2,'','skill_teacher','msg2_1',52,46),
 (26,3,'','skill_teacher','prose_cost',67,46),
 (26,4,'','skill_teacher','msg2_2',93,46),
 (26,5,'','skill_teacher','msg_yes',51,46),
 (26,6,'','skill_teacher','msg_no',51,46),
 (26,7,'','skill_teacher','msg3_3',51,46),			-- Change option batch here, same as the trainer at #1	
 (26,8,'','skill_teacher','error_empty_category',51,46),	-- Change option batch here, same as the trainer at #1	
 (26,9,'','skill_teacher','topped_out',51,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (26,10,'','skill_teacher','nsf_skill_pts',51,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (26,11,'','skill_teacher','msg1_1',51,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (26,12,'','skill_teacher','no_qualify',51,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (26,13,'','skill_teacher','surpass_trainer',51,46),		-- Trained to Master.
 (26,14,'','skill_teacher','trainer_merchant',51,46),
 (27,1,'','skill_teacher','trainer_smuggler',53,2),
 (27,2,'','skill_teacher','msg2_1',54,46),
 (27,3,'','skill_teacher','prose_cost',67,46),
 (27,4,'','skill_teacher','msg2_2',94,46),
 (27,5,'','skill_teacher','msg_yes',53,46),
 (27,6,'','skill_teacher','msg_no',53,46),
 (27,7,'','skill_teacher','msg3_3',53,46),			-- Change option batch here, same as the trainer at #1	
 (27,8,'','skill_teacher','error_empty_category',53,46),	-- Change option batch here, same as the trainer at #1	
 (27,9,'','skill_teacher','topped_out',53,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (27,10,'','skill_teacher','nsf_skill_pts',53,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (27,11,'','skill_teacher','msg1_1',53,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (27,12,'','skill_teacher','no_qualify',53,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (27,13,'','skill_teacher','surpass_trainer',53,46),		-- Trained to Master.
 (27,14,'','skill_teacher','trainer_smuggler',53,46),
 (28,1,'','skill_teacher','trainer_bountyhunter',55,2),
 (28,2,'','skill_teacher','msg2_1',56,46),
 (28,3,'','skill_teacher','prose_cost',67,46),
 (28,4,'','skill_teacher','msg2_2',95,46),
 (28,5,'','skill_teacher','msg_yes',55,46),
 (28,6,'','skill_teacher','msg_no',55,46),
 (28,7,'','skill_teacher','msg3_3',55,46),			-- Change option batch here, same as the trainer at #1	
 (28,8,'','skill_teacher','error_empty_category',55,46),	-- Change option batch here, same as the trainer at #1	
 (28,9,'','skill_teacher','topped_out',55,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (28,10,'','skill_teacher','nsf_skill_pts',55,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (28,11,'','skill_teacher','msg1_1',55,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (28,12,'','skill_teacher','no_qualify',55,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (28,13,'','skill_teacher','surpass_trainer',55,46),		-- Trained to Master.
 (28,14,'','skill_teacher','trainer_bountyhunter',55,46),
 (29,1,'','skill_teacher','trainer_commando',57,2),
 (29,2,'','skill_teacher','msg2_1',58,46),
 (29,3,'','skill_teacher','prose_cost',67,46),
 (29,4,'','skill_teacher','msg2_2',96,46),
 (29,5,'','skill_teacher','msg_yes',57,46),
 (29,6,'','skill_teacher','msg_no',57,46),
 (29,7,'','skill_teacher','msg3_3',57,46),			-- Change option batch here, same as the trainer at #1	
 (29,8,'','skill_teacher','error_empty_category',57,46),	-- Change option batch here, same as the trainer at #1	
 (29,9,'','skill_teacher','topped_out',57,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (29,10,'','skill_teacher','nsf_skill_pts',57,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (29,11,'','skill_teacher','msg1_1',57,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (29,12,'','skill_teacher','no_qualify',57,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (29,13,'','skill_teacher','surpass_trainer',57,46),		-- Trained to Master.
 (29,14,'','skill_teacher','trainer_commando',57,46),
 (30,1,'','skill_teacher','trainer_combatmedic',59,2),
 (30,2,'','skill_teacher','msg2_1',60,46),
 (30,3,'','skill_teacher','prose_cost',67,46),
 (30,4,'','skill_teacher','msg2_2',97,46),
 (30,5,'','skill_teacher','msg_yes',59,46),
 (30,6,'','skill_teacher','msg_no',59,46),
 (30,7,'','skill_teacher','msg3_3',59,46),			-- Change option batch here, same as the trainer at #1	
 (30,8,'','skill_teacher','error_empty_category',59,46),	-- Change option batch here, same as the trainer at #1	
 (30,9,'','skill_teacher','topped_out',59,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (30,10,'','skill_teacher','nsf_skill_pts',59,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (30,11,'','skill_teacher','msg1_1',59,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (30,12,'','skill_teacher','no_qualify',59,46),		-- No skill requirements, change option batch here, same as the trainer at #1
 (30,13,'','skill_teacher','surpass_trainer',59,46),		-- Trained to Master.
 (30,14,'','skill_teacher','trainer_combatmedic',59,46),	
 (31,1,'','skill_teacher','trainer_imagedesigner',61,2),
 (31,2,'','skill_teacher','msg2_1',62,46),
 (31,3,'','skill_teacher','prose_cost',67,46),
 (31,4,'','skill_teacher','msg2_2',98,46),
 (31,5,'','skill_teacher','msg_yes',61,46),
 (31,6,'','skill_teacher','msg_no',61,46),
 (31,7,'','skill_teacher','msg3_3',61,46),			-- Change option batch here, same as the trainer at #1	
 (31,8,'','skill_teacher','error_empty_category',61,46),	-- Change option batch here, same as the trainer at #1	
 (31,9,'','skill_teacher','topped_out',61,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (31,10,'','skill_teacher','nsf_skill_pts',61,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (31,11,'','skill_teacher','msg1_1',61,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (31,12,'','skill_teacher','no_qualify',61,46),		-- No skill requirements, change option batch here, same as the trainer at #1	
 (31,13,'','skill_teacher','surpass_trainer',61,46),		-- Trained to Master.
 (31,14,'','skill_teacher','trainer_imagedesigner',61,46),
 (32,1,'','skill_teacher','trainer_squadleader',63,2),
 (32,2,'','skill_teacher','msg2_1',64,46),
 (32,3,'','skill_teacher','prose_cost',67,46),
 (32,4,'','skill_teacher','msg2_2',99,46),
 (32,5,'','skill_teacher','msg_yes',63,46),
 (32,6,'','skill_teacher','msg_no',63,46),
 (32,7,'','skill_teacher','msg3_3',63,46),			-- Change option batch here, same as the trainer at #1	
 (32,8,'','skill_teacher','error_empty_category',63,46),	-- Change option batch here, same as the trainer at #1	
 (32,9,'','skill_teacher','topped_out',63,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (32,10,'','skill_teacher','nsf_skill_pts',63,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (32,11,'','skill_teacher','msg1_1',63,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (32,12,'','skill_teacher','no_qualify',63,46),		-- No skill requirements, change option batch here, same as the trainer at #1
 (32,13,'','skill_teacher','surpass_trainer',63,46),		-- Trained to Master.	
 (32,14,'','skill_teacher','trainer_squadleader',63,46),
 (33,1,'','skill_teacher','trainer_politician',65,2),
 (33,2,'','skill_teacher','msg2_1',66,46),
 (33,3,'','skill_teacher','prose_cost',67,46),
 (33,4,'','skill_teacher','msg2_2',100,46),
 (33,5,'','skill_teacher','msg_yes',65,46),
 (33,6,'','skill_teacher','msg_no',65,46),
 (33,7,'','skill_teacher','msg3_3',65,46),			-- Change option batch here, same as the trainer at #1	
 (33,8,'','skill_teacher','error_empty_category',65,46),	-- Change option batch here, same as the trainer at #1	
 (33,9,'','skill_teacher','topped_out',65,46),		-- Already Master, change option batch here, same as the trainer at #1	
 (33,10,'','skill_teacher','nsf_skill_pts',65,46),		-- Lack of skill points, change option batch here, same as the trainer at #1	
 (33,11,'','skill_teacher','msg1_1',65,46),		-- Lack of credits, change option batch here, same as the trainer at #1	
 (33,12,'','skill_teacher','no_qualify',65,46),		-- No skill requirements, change option batch here, same as the trainer at #1
 (33,13,'','skill_teacher','surpass_trainer',65,46),		-- Trained to Master.	
 (33,14,'','skill_teacher','trainer_politician',65,46),
 (34,1,'','newbie_tutorial/newbie_convo','convo_1_start',101,46),		-- tutorial item room imperial officer dialog.
 (34,2,'','newbie_tutorial/newbie_convo','convo_1_more',102,46),		-- tutorial item room imperial officer dialog.
 (34,3,'','newbie_tutorial/newbie_convo','convo_1_explain',103,46),		-- tutorial item room imperial officer dialog.
 (34,4,'','newbie_tutorial/newbie_convo','convo_1_stuff',104,46),		-- tutorial item room imperial officer dialog.

-- tutorial bank and bazzar room imperial officer dialog.
 (35,1,'','newbie_tutorial/newbie_convo','banker_1_start',105,46),		-- "Well it seems your ship wasn't carrying contraband after all.  Pity we didn't discover that sooner..."
 (35,2,'','newbie_tutorial/newbie_convo','banker_2_start',106,46),		-- "Well we suspected you were smuggling, so we boarded the transport you were travelling aboard, of course.  Don't worry, we've cleared you of all charges." 
 (35,3,'','newbie_tutorial/newbie_convo','banker_2_more',106,46),		-- "I'm afraid not.  It seems your ship was badly damaged and most of your possessions were destroyed."  
 (35,4,'','newbie_tutorial/newbie_convo','banker_2_explain',107,46),	-- "Don't worry!  We've taken care of everything.  We've given you more than enough credits to reimburse you for your loss.  You can deposit them at this banking terminal."   
 (35,5,'','newbie_tutorial/newbie_convo','banker_2_explain_2',106,46),	-- "When you get to a planet, look for a commodities market and buy anything you might possibly need."
   -- This is the replay to the second question at the beginning.
 (35,6,'','newbie_tutorial/newbie_convo','banker_2_explain_bank',107,46),	-- "Once we realized you were innocent, we reimbursed you with a generous number of credits for the destruction of your gear.  You can deposit them in this bank terminal." 
 (35,7,'','newbie_tutorial/newbie_convo','banker_2_explain_terminals',108,46),	-- "For your convenience, we'll allow you to access this starport's bank terminal." 
 (35,8,'','newbie_tutorial/newbie_convo','banker_2_more_bank',109,46),	  -- "You should definitely use the bank's safety deposit feature to keep your items safe from harm. It's also a good idea to deposit your credits into your bank accout before shopping at the commodities market since the market deducts credits from your bank account for all purchases." 
 (35,9,'','newbie_tutorial/newbie_convo','banker_2_bank_answer',109,46),  -- "You don't need cash on hand to buy things.  You can purchase items from the commodities market on any planet, and the credits will be deducted from your bank automatically." 
 (35,10,'','newbie_tutorial/newbie_convo','banker_bank_answer_2',109,46), -- "Any items you store in this bank will be transferred to your new bank.  Once you get to your planet, join any bank there and we'll send these items along."
 (35,11,'','newbie_tutorial/newbie_convo','banker_bazaar_1',110,46),	 -- "Once you've deposited your money at the bank terminal, browse the Item Dispenser to see if there is any equipment you'd like to purchase." 
 (35,12,'','newbie_tutorial/newbie_convo','banker_bazaar_2',111,46),	 -- "I think The Empire is being more than generous.  Do you know how much it cost us to capture the transport you were on, kill the crew, search for secret cargo holds... not to mention pulling you out of the fire!" 
 (35,13,'','newbie_tutorial/newbie_convo','banker_bazaar_3',112,46),	 -- "Ah, but now, how were we supposed to know that without capturing the transport first?" 
 
 -- tutorial cloning and insurance room droid dialog
 (36,1,'','newbie_tutorial/newbie_convo','convo_2_start',113,46),		-- "I'm here to explain Cloning and Insurance.  Where would you like to begin?" 
 (36,2,'','newbie_tutorial/newbie_convo','convo_2_cloning',114,46),		-- "If something should happen to you, your clone will take over with all the skills you had at the time you died.  Though you always have the option to respawn at the nearest cloning facility, unless you've cloned there you will emerge with many, many wounds." 
 (36,3,'','newbie_tutorial/newbie_convo','convo_2_cloning_2',115,46),	-- "You should clone yourself whenever you want the option to emerge from that particular location. Otherwise you will appear at the closest cloning facility from where you died, with many wounds. You will also need to set a new cloning location when you travel to a new planet, because there is no interplanetary cloning."  
 -- (36,4,'','newbie_tutorial/newbie_convo','convo_2_cloning_3',113,46),	-- "No.  Only when you want to have the option of appearing at a particular location without a lot of wounds, regardless of where you died."
 (36,4,'','newbie_tutorial/newbie_convo','convo_2_cloning_3',118,46),	-- "No.  Only when you want to have the option of appearing at a particular location without a lot of wounds, regardless of where you died."
 
 (36,5,'','newbie_tutorial/newbie_convo','convo_2_insurance',116,46),	-- "Insurance is a way of better preserving the condition of your belongings in the event of your untimely demise.  The return of nominally damaged, insured items to a cloning facility fetches a much higher price with the facility at which you choose to clone since you subsidize their item retrieval programs by purchasing insurance."
 (36,6,'','newbie_tutorial/newbie_convo','convo_2_insurance_2',117,46),	-- "Your items will suffer a significantly increased decay rate when transported back to your clone. People like to be paid, and cloning facilities don't pay much for recovery of uninsured items; so it stands to reason that uninsured items tend to not be cared for as well as their insured counterparts." 
 -- (36,7,'','newbie_tutorial/newbie_convo','convo_2_insurance_5',113,46);	-- "Yes!  Insurance is only valid one time.  If you want your items returned to you in a condition similar to last you saw them, you'll have to insure the item again!"  
 (36,7,'','newbie_tutorial/newbie_convo','convo_2_insurance_5',118,46),	-- "Yes!  Insurance is only valid one time.  If you want your items returned to you in a condition similar to last you saw them, you'll have to insure the item again!"   
 
  -- tutorial covard officer room dialog
 (37,1,'','newbie_tutorial/newbie_convo','convo_4_start',119,46),		-- "Y-you can't go IN there.  He cut them to pieces!  He's crazy...!"
 (37,2,'','newbie_tutorial/newbie_convo','convo_4_explain_1',120,46),	-- "Uh huh.  Look, y-you look pretty capable.  C-could you take a peek and see if there are any survivors in the hallway?" 
 (37,3,'','newbie_tutorial/newbie_convo','convo_4_explain_2',122,46),	-- "The only way through is here.  Past... him."
 (37,4,'','newbie_tutorial/newbie_convo','convo_4_explain_3',121,46),	-- "I, uh, need to stand guard.  In case reinforcements come."
 (37,5,'','newbie_tutorial/newbie_convo','convo_4_weapon_1',123,46),	-- "Wait, you'll need a weapon.  Uh, this is against regs, but... look, just don't tell anyone.  Here."
 (37,6,'','newbie_tutorial/newbie_convo','convo_4_end',0,46),			-- "Sure, whatever.  Look, I think I'll stand guard from the other side of the room, Ok?  And whatever you do, don't get this maniac angry!" 
 (37,7,'','newbie_tutorial/newbie_convo','convo_4_repeat',0,46),		-- Option not used yet. -- "What are you doing here?  Get a move on.  I'm, uh, still guarding here."
 (37,8,'','newbie_tutorial/newbie_convo','trooper_move_along',0,46),	-- Option not used yet. -- "Move along."

  -- tutorial imperial officer with trainer room dialog
 (38,1,'','newbie_tutorial/newbie_convo','off_1_start',124,46),			-- "Good show, in there.  You're quite resourceful."
 (38,2,'','newbie_tutorial/newbie_convo','off_1_gotrain',0,46),			-- "Well there's someone you should talk to right over there." 
 (38,3,'','newbie_tutorial/newbie_convo','off_1_gotrain2',0,46),		-- "Ha ha ha!  I suppose not.  Hey, there's someone right here you should talk to about that.  I bet you could learn something."

  -- tutorial imperial officer in mission terminal room dialog
 (39,1,'','newbie_tutorial/newbie_convo','mission_1_start',125,46),		-- "What took you so long to get here?  The shuttle's ready to leave!  Take these release documents to the Quartermaster and get yourself off my space station."
 (39,2,'','newbie_tutorial/newbie_convo','mission_1_waypoint',0,46),	-- "Here are your release documents.  Now get moving!  The Quartermaster is just down the hall, there."

  -- tutorial quartermaster room dialog
 (40,1,'','newbie_tutorial/newbie_convo','quarter_1_start',126,46),		-- "Ah!  I see you have the proper documents there.  I'll just take those from you." 
 (40,2,'','newbie_tutorial/newbie_convo','quarter_1_go_nochoice',0,46), -- "You may use the travel terminal here and be on your way.  And don't worry about the cost of transportation, the Empire will be picking up the tab on this one." 
 (40,3,'','newbie_tutorial/newbie_convo','quarter_nodocs',0,46);		-- "You don't have your release papers?  Well you'd better go get them from the Corporal.  Make it snappy!  Hup!  Hup!"  
 
/*!40000 ALTER TABLE `conversation_pages` ENABLE KEYS */;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;