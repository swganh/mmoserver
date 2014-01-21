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
-- Definition of table `moods`
--

DROP TABLE IF EXISTS `moods`;
CREATE TABLE `moods` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `name` char(64) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=188 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `moods`
--

/*!40000 ALTER TABLE `moods` DISABLE KEYS */;
INSERT INTO `moods` (`id`,`name`) VALUES 
 (0,'none'),
 (1,'absentminded'),
 (2,'amazed'),
 (3,'amused'),
 (4,'angry'),
 (5,'approving'),
 (6,'bitter'),
 (7,'bloodthirsty'),
 (8,'brave'),
 (9,'callous'),
 (10,'careful'),
 (11,'careless'),
 (12,'casual'),
 (13,'clinical'),
 (14,'cocky'),
 (15,'cold'),
 (16,'compassionate'),
 (17,'condescending'),
 (18,'confident'),
 (19,'confused'),
 (20,'content'),
 (21,'courtly'),
 (22,'coy'),
 (23,'crude'),
 (24,'cruel'),
 (25,'curious'),
 (26,'cynical'),
 (27,'defensive'),
 (28,'depressed'),
 (29,'devious'),
 (30,'dimwitted'),
 (31,'disappointes'),
 (32,'discreet'),
 (33,'disgruntled'),
 (34,'disgusted'),
 (35,'dismayed'),
 (36,'disoriented'),
 (37,'distracted'),
 (38,'doubtful'),
 (39,'dramatic'),
 (40,'dreamy'),
 (41,'drunk'),
 (42,'earnest'),
 (43,'ecstatic'),
 (44,'embarrassed'),
 (45,'emphatic'),
 (46,'encouraging'),
 (47,'enthusiastic'),
 (48,'evil'),
 (49,'exasperated'),
 (50,'exuberant'),
 (51,'fanatical'),
 (52,'forgive'),
 (53,'frustrated'),
 (54,'guilty'),
 (55,'happy'),
 (56,'honest'),
 (57,'hopeful'),
 (58,'hopeless'),
 (59,'humble'),
 (60,'hysterical'),
 (61,'imploring'),
 (62,'indifferent'),
 (63,'indignant'),
 (64,'interested'),
 (65,'jealous'),
 (66,'joyful'),
 (67,'lofty'),
 (68,'loud'),
 (69,'loving'),
 (70,'lustful'),
 (71,'mean'),
 (72,'mischievous'),
 (73,'nervous'),
 (74,'neutral'),
 (75,'offended'),
 (76,'optimistic'),
 (77,'pedantic'),
 (78,'pessimistic'),
 (79,'petulant'),
 (80,'philosophical'),
 (81,'pitying'),
 (82,'playful'),
 (83,'polite'),
 (84,'pompous'),
 (85,'proud'),
 (86,'provocative'),
 (87,'puzzled'),
 (88,'regretful'),
 (89,'relieved'),
 (90,'reluctant'),
 (91,'resigned'),
 (92,'respectful'),
 (93,'romantic'),
 (94,'rude'),
 (95,'sad'),
 (96,'sarcastic'),
 (97,'scared'),
 (98,'scolding'),
 (99,'scornful'),
 (100,'serious'),
 (101,'shameless'),
 (102,'shocked'),
 (103,'shy'),
 (104,'sincere'),
 (105,'sleepy'),
 (106,'sly'),
 (107,'smug'),
 (108,'snobby'),
 (109,'sorry'),
 (110,'spiteful'),
 (111,'stubborn'),
 (112,'sullen'),
 (113,'suspicious'),
 (114,'taunting'),
 (115,'terrified'),
 (116,'thankful'),
 (117,'thoughtful'),
 (118,'tolerant'),
 (119,'uncertain'),
 (120,'unhappy'),
 (121,'unwilling'),
 (122,'warm'),
 (123,'whiny'),
 (124,'wicked'),
 (125,'wistful'),
 (126,'worried'),
 (127,'tired'),
 (128,'exhausted'),
 (129,'friendly'),
 (130,'timid'),
 (131,'lazy'),
 (132,'surprised'),
 (133,'innocent'),
 (134,'wise'),
 (135,'youthful'),
 (136,'adventurous'),
 (137,'annoyed'),
 (138,'perturbed'),
 (139,'sedate'),
 (140,'calm'),
 (141,'suffering'),
 (142,'hungry'),
 (143,'thirsty'),
 (144,'alert'),
 (145,'shifty'),
 (146,'relaxed'),
 (147,'crotchety'),
 (148,'surly'),
 (149,'painful'),
 (150,'wounded'),
 (151,'bubbly'),
 (152,'heroic'),
 (153,'quiet'),
 (154,'remorseful'),
 (155,'grumpy'),
 (156,'logical'),
 (157,'emotional'),
 (158,'troubled'),
 (159,'panicked'),
 (160,'nice'),
 (161,'cheerful'),
 (162,'emotionless'),
 (163,'gloomy'),
 (164,'ambivalent'),
 (165,'envious'),
 (166,'vengeful'),
 (167,'fearful'),
 (168,'enraged'),
 (169,'sheepish'),
 (170,'belligerent'),
 (171,'obnoxious'),
 (172,'fastidious'),
 (173,'squeamish'),
 (174,'dainty'),
 (175,'dignified'),
 (176,'haughty'),
 (177,'obscure'),
 (178,'goofy'),
 (179,'silly'),
 (180,'disdainful'),
 (181,'contemptuous'),
 (182,'diplomatic'),
 (183,'wary'),
 (184,'malevolent'),
 (185,'hurried'),
 (186,'patient'),
 (187,'firm');
/*!40000 ALTER TABLE `moods` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;