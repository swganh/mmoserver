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
-- Definition of table `vehicle_type_attributes`
--

DROP TABLE IF EXISTS `vehicle_type_attributes`;
CREATE TABLE `vehicle_type_attributes` (
  `vehicle_types_id` bigint(20) NOT NULL,
  `attribute_id` int(11) NOT NULL,
  `attribute_value` char(255) default NULL,
  `attribute_order` int(11) default NULL,
  PRIMARY KEY  (`vehicle_types_id`,`attribute_id`),
  KEY `fk_vehicle_type_attributes_vehicle_types` (`vehicle_types_id`),
  CONSTRAINT `fk_vehicle_type_attributes_vehicle_types` FOREIGN KEY (`vehicle_types_id`) REFERENCES `vehicle_types` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `vehicle_type_attributes`
--

/*!40000 ALTER TABLE `vehicle_type_attributes` DISABLE KEYS */;
INSERT INTO `vehicle_type_attributes` (`vehicle_types_id`,`attribute_id`,`attribute_value`,`attribute_order`) VALUES 
 (1,73,'A personal jetpack',1),
 (1,198,'25000/25000',13),
 (1,200,NULL,12),
 (1,568,NULL,11),
 (1,2011,'25%',3),
 (1,2012,'25%',4),
 (1,2013,'25%',5),
 (1,2014,'25%',7),
 (1,2015,'25%',8),
 (1,2016,'25%',9),
 (1,2017,'25%',10),
 (1,2018,'25%',6),
 (1,2019,'30%',2),
 (2,73,'AV-21 Landspeeder',1),
 (2,198,'25000/25000',13),
 (2,200,NULL,12),
 (2,568,NULL,11),
 (2,2011,'25%',3),
 (2,2012,'25%',4),
 (2,2013,'25%',5),
 (2,2014,'25%',7),
 (2,2015,'25%',8),
 (2,2016,'25%',9),
 (2,2017,'25%',10),
 (2,2018,'25%',6),
 (2,2019,'30%',2),
 (4,73,'X31 Landspeeder',1),
 (4,198,'25000/25000',13),
 (4,200,'',12),
 (4,568,'',11),
 (4,2011,'25%',3),
 (4,2012,'25%',4),
 (4,2013,'25%',5),
 (4,2014,'25%',7),
 (4,2015,'25%',8),
 (4,2016,'25%',9),
 (4,2017,'25%',10),
 (4,2018,'25%',6),
 (4,2019,'30%',2),
 (5,73,'X34 Landspeeder',1),
 (5,198,'25000/25000',13),
 (5,200,'',12),
 (5,568,'',11),
 (5,2011,'25%',3),
 (5,2012,'25%',4),
 (5,2013,'25%',5),
 (5,2014,'25%',7),
 (5,2015,'25%',8),
 (5,2016,'25%',9),
 (5,2017,'25%',10),
 (5,2018,'25%',6),
 (5,2019,'30%',2),
 (7,73,'Speederbike vehicle',1),
 (7,198,'25000/25000',13),
 (7,200,'',12),
 (7,568,'',11),
 (7,2011,'25%',3),
 (7,2012,'25%',4),
 (7,2013,'25%',5),
 (7,2014,'25%',7),
 (7,2015,'25%',8),
 (7,2016,'25%',9),
 (7,2017,'25%',10),
 (7,2018,'25%',6),
 (7,2019,'30%',2),
 (9,73,'A Flash Speeder vehicle.',1),
 (9,198,'25000/25000',13),
 (9,200,'',12),
 (9,568,'',11),
 (9,2011,'25%',3),
 (9,2012,'25%',4),
 (9,2013,'25%',5),
 (9,2014,'25%',7),
 (9,2015,'25%',8),
 (9,2016,'25%',9),
 (9,2017,'25%',10),
 (9,2018,'25%',6),
 (9,2019,'30%',2),
 (10,73,'A Flash Speeder vehicle.',1),
 (10,198,'25000/25000',13),
 (10,200,'',12),
 (10,568,'',11),
 (10,2011,'25%',3),
 (10,2012,'25%',4),
 (10,2013,'25%',5),
 (10,2014,'25%',7),
 (10,2015,'25%',8),
 (10,2016,'25%',9),
 (10,2017,'25%',10),
 (10,2018,'25%',6),
 (10,2019,'30%',2);
/*!40000 ALTER TABLE `vehicle_type_attributes` ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;