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
-- Definition of table `entertainer_performances`
--

DROP TABLE IF EXISTS `entertainer_performances`;
CREATE TABLE `entertainer_performances` (
  `performanceName` char(32) default NULL,
  `instrumentAudioId` int(11) default NULL,
  `requiredSong` char(64) default NULL,
  `requiredInstrument` char(64) default NULL,
  `InstrumenType` int(11) default NULL,
  `requiredDance` char(64) default NULL,
  `danceVisualId` int(11) default NULL,
  `actionPointPerLoop` int(11) default NULL,
  `loopDuration` int(11) default NULL,
  `type` bigint(20) default NULL,
  `baseXp` int(11) default NULL,
  `florushXpMod` int(11) default NULL,
  `healMindWound` int(11) default NULL,
  `healShockWound` int(11) default NULL,
  `requiredSkillMod` char(32) default NULL,
  `mainloop` int(11) default NULL,
  `MusicVisualId` int(11) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `entertainer_performances`
--

/*!40000 ALTER TABLE `entertainer_performances` DISABLE KEYS */;
INSERT INTO `entertainer_performances` (`performanceName`,`instrumentAudioId`,`requiredSong`,`requiredInstrument`,`InstrumenType`,`requiredDance`,`danceVisualId`,`actionPointPerLoop`,`loopDuration`,`type`,`baseXp`,`florushXpMod`,`healMindWound`,`healShockWound`,`requiredSkillMod`,`mainloop`,`MusicVisualId`) VALUES
 ('starwars1',1,'startmusic+starwars1','slitherhorn',1313,'',0,28,10,866729052,0,2,4,2,'healing_music_ability',5,3),
 ('starwars1',2,'startmusic+starwars1','fizz',1315,'',0,32,10,866729052,0,3,6,3,'healing_music_ability',5,3),
 ('starwars1',3,'startmusic+starwars1','fanfar',1318,'',0,36,10,866729052,0,6,6,3,'healing_music_ability',10,3),
 ('starwars1',4,'startmusic+starwars1','kloohorn',1314,'',0,36,10,866729052,0,7,8,4,'healing_music_ability',10,3),
 ('starwars1',5,'startmusic+starwars1','mandoviol',1323,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',15,5),
 ('starwars1',6,'startmusic+starwars1','traz',1322,'',0,40,10,866729052,0,9,10,5,'healing_music_ability',15,3),
 ('starwars1',7,'startmusic+starwars1','bandfill',1316,'',0,44,10,866729052,0,13,10,5,'healing_music_ability',15,1),
 ('starwars1',8,'startmusic+starwars1','flutedroopy',1317,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',10,1),
 ('starwars1',9,'startmusic+starwars1','omnibox',1319,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',10,4),
 ('starwars1',10,'startmusic+starwars1','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',20,2),
 ('starwars1',10,'startmusic+starwars1','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',20,2),
 ('rock',11,'startmusic+rock','slitherhorn',1313,'',0,32,10,866729052,0,3,6,3,'healing_music_ability',10,3),
 ('rock',12,'startmusic+rock','fizz',1315,'',0,32,10,866729052,0,3,6,3,'healing_music_ability',10,3),
 ('rock',13,'startmusic+rock','fanfar',1318,'',0,36,10,866729052,0,6,6,3,'healing_music_ability',15,3),
 ('rock',14,'startmusic+rock','kloohorn',1314,'',0,36,10,866729052,0,7,8,4,'healing_music_ability',15,3),
 ('rock',15,'startmusic+rock','mandoviol',1323,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',20,5),
 ('rock',16,'startmusic+rock','traz',1322,'',0,40,10,866729052,0,9,10,5,'healing_music_ability',20,3),
 ('rock',17,'startmusic+rock','bandfill',1316,'',0,44,10,866729052,0,13,10,5,'healing_music_ability',20,1),
 ('rock',18,'startmusic+rock','flutedroopy',1317,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',15,1),
 ('rock',19,'startmusic+rock','omnibox',1319,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',15,4),
 ('rock',20,'startmusic+rock','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',25,2),
 ('rock',20,'startmusic+rock','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',25,2),
 ('starwars2',21,'startmusic+starwars2','slitherhorn',1313,'',0,32,10,866729052,0,4,6,3,'healing_music_ability',20,3),
 ('starwars2',22,'startmusic+starwars2','fizz',1315,'',0,32,10,866729052,0,4,6,3,'healing_music_ability',20,3),
 ('starwars2',23,'startmusic+starwars2','fanfar',1318,'',0,36,10,866729052,0,6,6,3,'healing_music_ability',25,3),
 ('starwars2',24,'startmusic+starwars2','kloohorn',1314,'',0,36,10,866729052,0,7,8,4,'healing_music_ability',25,3),
 ('starwars2',25,'startmusic+starwars2','mandoviol',1323,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',30,5),
 ('starwars2',26,'startmusic+starwars2','traz',1322,'',0,40,10,866729052,0,9,10,5,'healing_music_ability',30,3),
 ('starwars2',27,'startmusic+starwars2','bandfill',1316,'',0,44,10,866729052,0,13,10,5,'healing_music_ability',30,1),
 ('starwars2',28,'startmusic+starwars2','flutedroopy',1317,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',25,1),
 ('starwars2',29,'startmusic+starwars2','omnibox',1319,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',25,4),
 ('starwars2',30,'startmusic+starwars2','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',35,2),
 ('starwars2',30,'startmusic+starwars2','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',35,2),
 ('folk',31,'startmusic+folk','slitherhorn',1313,'',0,36,10,866729052,0,6,6,3,'healing_music_ability',30,3),
 ('folk',32,'startmusic+folk','fizz',1315,'',0,36,10,866729052,0,6,6,3,'healing_music_ability',30,3),
 ('folk',33,'startmusic+folk','fanfar',1318,'',0,36,10,866729052,0,6,6,3,'healing_music_ability',35,3),
 ('folk',34,'startmusic+folk','kloohorn',1314,'',0,36,10,866729052,0,7,8,4,'healing_music_ability',35,3),
 ('folk',35,'startmusic+folk','mandoviol',1323,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',35,5),
 ('folk',36,'startmusic+folk','traz',1322,'',0,40,10,866729052,0,9,10,5,'healing_music_ability',40,3),
 ('folk',37,'startmusic+folk','bandfill',1316,'',0,44,10,866729052,0,13,10,5,'healing_music_ability',40,1),
 ('folk',38,'startmusic+folk','flutedroopy',1317,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',35,1),
 ('folk',39,'startmusic+folk','omnibox',1319,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',35,4),
 ('folk',40,'startmusic+folk','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',45,2),
 ('folk',40,'startmusic+folk','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',45,2), 
 ('starwars3',41,'startmusic+starwars3','slitherhorn',1313,'',0,36,10,866729052,0,7,8,4,'healing_music_ability',40,3),
 ('starwars3',42,'startmusic+starwars3','fizz',1315,'',0,36,10,866729052,0,7,8,4,'healing_music_ability',40,3),
 ('starwars3',43,'startmusic+starwars3','fanfar',1318,'',0,36,10,866729052,0,7,8,4,'healing_music_ability',45,3),
 ('starwars3',44,'startmusic+starwars3','kloohorn',1314,'',0,36,10,866729052,0,7,8,4,'healing_music_ability',45,3),
 ('starwars3',45,'startmusic+starwars3','mandoviol',1323,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',50,5),
 ('starwars3',46,'startmusic+starwars3','traz',1322,'',0,40,10,866729052,0,9,10,5,'healing_music_ability',50,3),
 ('starwars3',47,'startmusic+starwars3','bandfill',1316,'',0,44,10,866729052,0,13,10,5,'healing_music_ability',50,1),
 ('starwars3',48,'startmusic+starwars3','flutedroopy',1317,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',45,1),
 ('starwars3',49,'startmusic+starwars3','omnibox',1319,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',45,4),
 ('starwars3',50,'startmusic+starwars3','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',55,2),
 ('starwars3',50,'startmusic+starwars3','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',55,2), 
 ('ceremonial',51,'startmusic+ceremonial','slitherhorn',1313,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',45,3),
 ('ceremonial',52,'startmusic+ceremonial','fizz',1315,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',45,3),
 ('ceremonial',53,'startmusic+ceremonial','fanfar',1318,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',50,3),
 ('ceremonial',54,'startmusic+ceremonial','kloohorn',1314,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',50,3),
 ('ceremonial',55,'startmusic+ceremonial','mandoviol',1323,'',0,40,10,866729052,0,9,8,4,'healing_music_ability',55,5),
 ('ceremonial',56,'startmusic+ceremonial','traz',1322,'',0,40,10,866729052,0,9,10,5,'healing_music_ability',55,3),
 ('ceremonial',57,'startmusic+ceremonial','bandfill',1316,'',0,44,10,866729052,0,13,10,5,'healing_music_ability',55,1),
 ('ceremonial',58,'startmusic+ceremonial','flutedroopy',1317,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',50,1),
 ('ceremonial',59,'startmusic+ceremonial','omnibox',1319,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',50,4),
 ('ceremonial',60,'startmusic+ceremonial','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',60,2),
 ('ceremonial',60,'startmusic+ceremonial','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',60,2), 
 ('ballad',61,'startmusic+ballad','slitherhorn',1313,'',0,40,10,866729052,0,11,10,5,'healing_music_ability',60,3),
 ('ballad',62,'startmusic+ballad','fizz',1315,'',0,40,10,866729052,0,11,10,5,'healing_music_ability',60,3),
 ('ballad',63,'startmusic+ballad','fanfar',1318,'',0,40,10,866729052,0,11,10,5,'healing_music_ability',65,3),
 ('ballad',64,'startmusic+ballad','kloohorn',1314,'',0,40,10,866729052,0,11,10,5,'healing_music_ability',65,3),
 ('ballad',65,'startmusic+ballad','mandoviol',1323,'',0,40,10,866729052,0,11,10,5,'healing_music_ability',70,5),
 ('ballad',66,'startmusic+ballad','traz',1322,'',0,40,10,866729052,0,11,10,5,'healing_music_ability',70,3),
 ('ballad',67,'startmusic+ballad','bandfill',1316,'',0,44,10,866729052,0,13,10,5,'healing_music_ability',70,1),
 ('ballad',68,'startmusic+ballad','flutedroopy',1317,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',65,1),
 ('ballad',69,'startmusic+ballad','omnibox',1319,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',65,4),
 ('ballad',70,'startmusic+ballad','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',75,2),
 ('ballad',70,'startmusic+ballad','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',75,2), 
 ('waltz',71,'startmusic+waltz','slitherhorn',1313,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',70,3),
 ('waltz',72,'startmusic+waltz','fizz',1315,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',70,3),
 ('waltz',73,'startmusic+waltz','fanfar',1318,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',75,3),
 ('waltz',74,'startmusic+waltz','kloohorn',1314,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',75,3),
 ('waltz',75,'startmusic+waltz','mandoviol',1323,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',80,5),
 ('waltz',76,'startmusic+waltz','traz',1322,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',80,3),
 ('waltz',77,'startmusic+waltz','bandfill',1316,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',80,1),
 ('waltz',78,'startmusic+waltz','flutedroopy',1317,'',0,44,10,866729052,0,15,12,6,'healing_music_ability',75,1),
 ('waltz',79,'startmusic+waltz','omnibox',1319,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',75,4),
 ('waltz',80,'startmusic+waltz','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',85,2),
 ('waltz',80,'startmusic+waltz','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',85,2), 
 ('jazz',81,'startmusic+jazz','slitherhorn',1313,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',85,3),
 ('jazz',82,'startmusic+jazz','fizz',1315,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',85,3),
 ('jazz',83,'startmusic+jazz','fanfar',1318,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',90,3),
 ('jazz',84,'startmusic+jazz','kloohorn',1314,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',90,3),
 ('jazz',85,'startmusic+jazz','mandoviol',1323,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',95,5),
 ('jazz',86,'startmusic+jazz','traz',1322,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',95,3),
 ('jazz',87,'startmusic+jazz','bandfill',1316,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',95,1),
 ('jazz',88,'startmusic+jazz','flutedroopy',1317,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',90,1),
 ('jazz',89,'startmusic+jazz','omnibox',1319,'',0,48,10,866729052,0,17,12,6,'healing_music_ability',90,4),
 ('jazz',90,'startmusic+jazz','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,2),
 ('jazz',90,'startmusic+jazz','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,2), 
 ('virtuoso',91,'startmusic+virtuoso','slitherhorn',1313,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,3),
 ('virtuoso',92,'startmusic+virtuoso','fizz',1315,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,3),
 ('virtuoso',93,'startmusic+virtuoso','fanfar',1318,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,3),
 ('virtuoso',94,'startmusic+virtuoso','kloohorn',1314,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,3),
 ('virtuoso',95,'startmusic+virtuoso','mandoviol',1323,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,5),
 ('virtuoso',96,'startmusic+virtuoso','traz',1322,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,3),
 ('virtuoso',97,'startmusic+virtuoso','bandfill',1316,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,1),
 ('virtuoso',98,'startmusic+virtuoso','flutedroopy',1317,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,1),
 ('virtuoso',99,'startmusic+virtuoso','omnibox',1319,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,4),
 ('virtuoso',100,'startmusic+virtuoso','nalargon',1320,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,2),
 ('virtuoso',100,'startmusic+virtuoso','nalargon_max_rebo',1321,'',0,48,10,866729052,0,20,14,7,'healing_music_ability',100,2), 
 ('western',101,'startmusic+western','slitherhorn',1313,'',0,36,6,866729052,0,7,8,4,'healing_music_ability',40,3),
 ('western',102,'startmusic+western','fizz',1315,'',0,36,6,866729052,0,7,8,4,'healing_music_ability',40,3),
 ('western',103,'startmusic+western','fanfar',1318,'',0,36,6,866729052,0,7,8,4,'healing_music_ability',45,3),
 ('western',104,'startmusic+western','kloohorn',1314,'',0,36,6,866729052,0,7,8,4,'healing_music_ability',45,3),
 ('western',105,'startmusic+western','mandoviol',1323,'',0,40,6,866729052,0,9,8,4,'healing_music_ability',50,5),
 ('western',106,'startmusic+western','traz',1322,'',0,40,6,866729052,0,9,10,5,'healing_music_ability',50,3),
 ('western',107,'startmusic+western','bandfill',1316,'',0,44,6,866729052,0,13,10,5,'healing_music_ability',50,1),
 ('western',108,'startmusic+western','flutedroopy',1317,'',0,44,6,866729052,0,15,12,6,'healing_music_ability',45,1),
 ('western',109,'startmusic+western','omnibox',1319,'',0,48,6,866729052,0,17,12,6,'healing_music_ability',45,4),
 ('western',110,'startmusic+western','nalargon',1320,'',0,48,6,866729052,0,20,14,7,'healing_music_ability',55,2),
 ('western',110,'startmusic+western','nalargon_max_rebo',1321,'',0,48,6,866729052,0,20,14,7,'healing_music_ability',55,2), 
 ('starwars4',111,'startmusic+starwars4','slitherhorn',1313,'',0,36,6,866729052,0,8,9,2,'healing_music_ability',40,3),
 ('starwars4',112,'startmusic+starwars4','fizz',1315,'',0,36,6,866729052,0,8,9,3,'healing_music_ability',40,3),
 ('starwars4',113,'startmusic+starwars4','fanfar',1318,'',0,36,6,866729052,0,8,9,3,'healing_music_ability',45,3),
 ('starwars4',114,'startmusic+starwars4','kloohorn',1314,'',0,36,6,866729052,0,8,9,4,'healing_music_ability',45,3),
 ('starwars4',115,'startmusic+starwars4','mandoviol',1323,'',0,40,6,866729052,0,10,9,4,'healing_music_ability',50,5),
 ('starwars4',116,'startmusic+starwars4','traz',1322,'',0,40,6,866729052,0,10,11,5,'healing_music_ability',50,3),
 ('starwars4',117,'startmusic+starwars4','bandfill',1316,'',0,44,6,866729052,0,14,11,5,'healing_music_ability',50,1),
 ('starwars4',118,'startmusic+starwars4','flutedroopy',1317,'',0,44,6,866729052,0,16,13,6,'healing_music_ability',45,1),
 ('starwars4',119,'startmusic+starwars4','omnibox',1319,'',0,48,6,866729052,0,17,12,6,'healing_music_ability',45,4),
 ('starwars4',120,'startmusic+starwars4','nalargon',1320,'',0,48,6,866729052,0,20,14,7,'healing_music_ability',55,2),
 ('starwars4',120,'startmusic+starwars4','nalargon_max_rebo',1321,'',0,48,6,866729052,0,20,14,7,'healing_music_ability',55,2), 
 ('funk',121,'startmusic+funk','slitherhorn',1313,'',0,40,6,866729052,0,13,10,5,'healing_music_ability',60,3),
 ('funk',122,'startmusic+funk','fizz',1315,'',0,40,6,866729052,0,13,10,5,'healing_music_ability',60,3),
 ('funk',123,'startmusic+funk','fanfar',1318,'',0,40,6,866729052,0,13,10,5,'healing_music_ability',65,3),
 ('funk',124,'startmusic+funk','kloohorn',1314,'',0,40,6,866729052,0,13,10,5,'healing_music_ability',65,3),
 ('funk',125,'startmusic+funk','mandoviol',1323,'',0,40,6,866729052,0,13,10,5,'healing_music_ability',70,5),
 ('funk',126,'startmusic+funk','traz',1322,'',0,40,6,866729052,0,13,10,5,'healing_music_ability',70,3),
 ('funk',127,'startmusic+funk','bandfill',1316,'',0,44,6,866729052,0,13,10,5,'healing_music_ability',70,1),
 ('funk',128,'startmusic+funk','flutedroopy',1317,'',0,44,6,866729052,0,13,10,6,'healing_music_ability',65,1),
 ('funk',129,'startmusic+funk','omnibox',1319,'',0,48,6,866729052,0,17,12,6,'healing_music_ability',65,4),
 ('funk',130,'startmusic+funk','nalargon',1320,'',0,48,6,866729052,0,20,14,7,'healing_music_ability',75,2),
 ('funk',130,'startmusic+funk','nalargon_max_rebo',1321,'',0,48,6,866729052,0,20,14,7,'healing_music_ability',75,2), 
 ('basic',0,'','',0,'startDance+basic',1,28,10,-1788534963,0,4,4,2,'healing_dance_ability',5,1),
 ('rhythmic',0,'','',0,'startDance+rhythmic',3,28,10,-1788534963,0,4,4,2,'healing_dance_ability',5,1),
 ('basic2',0,'','',0,'startDance+basic2',2,32,10,-1788534963,0,6,6,3,'healing_dance_ability',10,1),
 ('rhythmic2',0,'','',0,'startDance+rhythmic2',4,32,10,-1788534963,0,8,6,3,'healing_dance_ability',15,1),
 ('footloose',0,'','',0,'startDance+footloose',15,36,10,-1788534963,0,12,6,3,'healing_dance_ability',20,1),
 ('formal',0,'','',0,'startDance+formal',17,36,10,-1788534963,0,14,8,4,'healing_dance_ability',30,1),
 ('footloose2',0,'','',0,'startDance+footloose2',16,40,10,-1788534963,0,18,8,4,'healing_dance_ability',40,1),
 ('formal2',0,'','',0,'startDance+formal2',18,40,10,-1788534963,0,18,8,4,'healing_dance_ability',40,1),
 ('popular',0,'','',0,'startDance+popular',9,40,10,-1788534963,0,18,10,5,'healing_dance_ability',50,1),
 ('poplock',0,'','',0,'startDance+poplock',13,40,10,-1788534963,0,18,10,5,'healing_dance_ability',50,1),
 ('popular2',0,'','',0,'startDance+popular2',10,40,10,-1788534963,0,22,10,5,'healing_dance_ability',60,1),
 ('poplock2',0,'','',0,'startDance+poplock2',14,44,10,-1788534963,0,26,10,5,'healing_dance_ability',70,1),
 ('lyrical',0,'','',0,'startDance+lyrical',11,44,10,-1788534963,0,30,10,6,'healing_dance_ability',80,1),
 ('exotic',0,'','',0,'startDance+exotic',5,48,10,-1788534963,0,34,10,6,'healing_dance_ability',90,1),
 ('exotic2',0,'','',0,'startDance+exotic2',6,48,10,-1788534963,0,34,12,6,'healing_dance_ability',90,1),
 ('lyrical2',0,'','',0,'startDance+lyrical2',12,48,10,-1788534963,0,40,12,7,'healing_dance_ability',100,1),
 ('exotic3',0,'','',0,'startDance+exotic3',7,48,10,-1788534963,0,40,12,7,'healing_dance_ability',100,1),
 ('exotic4',0,'','',0,'startDance+exotic4',8,48,10,-1788534963,0,40,14,7,'healing_dance_ability',100,1),
 ('theatrical',0,'','',0,'startDance+theatrical',21,36,10,-1788534963,0,12,6,3,'healing_dance_ability',50,1),
 ('theatrical2',0,'','',0,'startDance+theatrical2',22,40,13,-1788534963,0,18,8,4,'healing_dance_ability',70,1),
 ('breakdance',0,'','',0,'startDance+breakdance',29,40,10,-1788534963,0,22,10,5,'healing_dance_ability',60,1),
 ('breakdance2',0,'','',0,'startDance+breakdance2',30,44,10,-1788534963,0,26,10,5,'healing_dance_ability',70,1),
 ('tumble',0,'','',0,'startDance+tumble',31,44,10,-1788534963,0,30,10,6,'healing_dance_ability',80,1),
 ('tumble2',0,'','',0,'startDance+tumble2',32,48,10,-1788534963,0,34,10,6,'healing_dance_ability',90,1);
/*!40000 ALTER TABLE `entertainer_performances` ENABLE KEYS */;




/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;