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
-- Definition of table `commerce_bazaar`
--

DROP TABLE IF EXISTS `commerce_bazaar`;
CREATE TABLE `commerce_bazaar` (
  `bazaar_id` bigint(11) unsigned NOT NULL,
  `bazaar_string` char(128) NOT NULL,
  `region_id` int(2) NOT NULL,
  `planet_id` int(2) NOT NULL,
  KEY `region_id` (`region_id`),
  KEY `terminal_id` (`planet_id`),
  KEY `bazaar_id` (`bazaar_id`),
  CONSTRAINT `commerce_bazaar_ibfk_1` FOREIGN KEY (`bazaar_id`) REFERENCES `terminals` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `commerce_bazaar_ibfk_2` FOREIGN KEY (`planet_id`) REFERENCES `planet` (`planet_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `commerce_bazaar`
--

/*!40000 ALTER TABLE `commerce_bazaar` DISABLE KEYS */;
INSERT INTO `commerce_bazaar` (`bazaar_id`,`bazaar_string`,`region_id`,`planet_id`) VALUES
 (683456,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.683456#-1398,-3717',27,8),
 (683457,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.683457#-1393,-3717',27,8),
 (683458,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.683458#-1393,-3707',27,8),
 (683459,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.683459#-1398,-3707',27,8),
 (683508,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.683508#-1108,-3537',27,8),
 (683509,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.683509#-1106,-3535',27,8),
 (683510,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.683510#-1104,-3532',27,8),
 (1250243,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.1250243#-1124,-3676',27,8),
 (1250246,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.1250246#-1105,-3692',27,8),
 (1365910,'dantooine.@dantooine_region_names:dantooine_imperial_outpost.@terminal_name:terminal_bazaar.1365910#-4,-1',3,1),
 (1365954,'dantooine.@dantooine_region_names:dantooine_imperial_outpost.@terminal_name:terminal_bazaar.1365954#-4253,-2413',3,1),
 (1366001,'dantooine.@dantooine_region_names:dantooine_mining_outpost.@terminal_name:terminal_bazaar.1366001#-619,2491',1,1),
 (1392905,'dathomir.@dathomir_region_names:dathomir_survey_outpost.@terminal_name:terminal_bazaar.1392905#573,3092',11,2),
 (2776037,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.2776037#-5114,4146',16,5),
 (2776038,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.2776038#-5145,4143',16,5),
 (2776039,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.2776039#-5148,4174',16,5),
 (2776040,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.2776040#-5684,4171',16,5),
 (2776041,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.2776041#-5706,4149',16,5),
 (2776042,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.2776042#-5728,4171',16,5),
 (2776043,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.2776043#-5706,4193',16,5),
 (2776044,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.2776044#-5010,4256',16,5),
 (2776045,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.2776045#-5018,4247',16,5),
 (2835652,'dathomir.@dathomir_region_names:dathomir_trade_outpost.@terminal_name:terminal_bazaar.2835652#-81,-1589',10,2),
 (2835653,'dathomir.@dathomir_region_names:dathomir_trade_outpost.@terminal_name:terminal_bazaar.2835653#-70,-1598',10,2),
 (3035821,'yavin4.@yavin4_region_names:yavin4_labor_outpost.@terminal_name:terminal_bazaar.3035821#-6917,-5742',35,9),
 (3035822,'yavin4.@yavin4_region_names:yavin4_labor_outpost.@terminal_name:terminal_bazaar.3035822#-6917,-5737',35,9),
 (3606001,'endor.@endor_region_names:endor_neutral_outpost.@terminal_name:terminal_bazaar.3606001#3232,-3502',13,3),
 (3606003,'endor.@endor_region_names:endor_neutral_outpost.@terminal_name:terminal_bazaar.3606003#3244,-3502',13,3),
 (3695501,'corellia.@corellia_region_names:bela_vistal.@terminal_name:terminal_bazaar.3695501#6645,-5943',6,0),
 (3695502,'corellia.@corellia_region_names:bela_vistal.@terminal_name:terminal_bazaar.3695502#6649,-5943',6,0),
 (3945352,'tatooine.@tatooine_region_names:anchorhead.@terminal_name:terminal_bazaar.3945352#64,-5351',32,8),
 (3945353,'tatooine.@tatooine_region_names:anchorhead.@terminal_name:terminal_bazaar.3945353#61,-5351',32,8),
 (3945356,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.3945356#-1447,-3548',27,8),
 (3945359,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.3945359#-1447,-3551',27,8),
 (3945360,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.3945360#-1452,-3577',27,8),
 (3945361,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.3945361#-1452,-3580',27,8),
 (3945362,'tatooine.@tatooine_region_names:mos_eisley.@terminal_name:terminal_bazaar.3945362#3694,-4816',29,8),
 (3945363,'tatooine.@tatooine_region_names:mos_eisley.@terminal_name:terminal_bazaar.3945363#3694,-4813',29,8),
 (3945364,'tatooine.@tatooine_region_names:mos_eisley.@terminal_name:terminal_bazaar.3945364#3693,-4786',29,8),
 (3945365,'tatooine.@tatooine_region_names:mos_eisley.@terminal_name:terminal_bazaar.3945365#3693,-4783',29,8),
 (3945376,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.3945376#1591,3027',30,8),
 (3945377,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.3945377#1591,3024',30,8),
 (3945383,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.3945383#-2775,2025',28,8),
 (3945384,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.3945384#-2773,2028',28,8),
 (3945385,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.3945385#-2759,2051',28,8),
 (3945386,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.3945386#-2757,2053',28,8),
 (3945387,'tatooine.@tatooine_region_names:mos_taike.@terminal_name:terminal_bazaar.3945387#3790,2337',31,8),
 (3945388,'tatooine.@tatooine_region_names:mos_taike.@terminal_name:terminal_bazaar.3945388#3790,2340',31,8),
 (3945394,'tatooine.@tatooine_region_names:wayfar.@terminal_name:terminal_bazaar.3945394#-5177,-6485',33,8),
 (3945395,'tatooine.@tatooine_region_names:wayfar.@terminal_name:terminal_bazaar.3945395#-5174,-6485',33,8),
 (3955353,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.3955353#-5014,4252',16,5),
 (3955363,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.3955363#-5351,4141',16,5),
 (3955364,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.3955364#-5351,4148',16,5),
 (3955372,'naboo.@naboo_region_names:moenia.@terminal_name:terminal_bazaar.3955372#4879,-4851',18,5),
 (3955374,'naboo.@naboo_region_names:keren.@terminal_name:terminal_bazaar.3955374#1278,2771',17,5),
 (3955375,'naboo.@naboo_region_names:keren.@terminal_name:terminal_bazaar.3955375#1278,2774',17,5),
 (3955376,'naboo.@naboo_region_names:keren.@terminal_name:terminal_bazaar.3955376#1278,2758',17,5),
 (3955377,'naboo.@naboo_region_names:keren.@terminal_name:terminal_bazaar.3955377#1278,2761',17,5),
 (4005560,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.4005560#1206,2993',30,8),
 (4005561,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.4005561#1209,2991',30,8),
 (4005562,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.4005562#1181,3010',30,8),
 (4005563,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.4005563#1185,3008',30,8),
 (4105355,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.4105355#-5119,4334',16,5),
 (4105356,'naboo.@naboo_region_names:theed.@terminal_name:terminal_bazaar.4105356#-5077,4299',16,5),
 (4105632,'naboo.@naboo_region_names:kaadara.@terminal_name:terminal_bazaar.4105632#5368,6646',20,5),
 (4105633,'naboo.@naboo_region_names:kaadara.@terminal_name:terminal_bazaar.4105633#5369,6649',20,5),
 (4105634,'naboo.@naboo_region_names:kaadara.@terminal_name:terminal_bazaar.4105634#5370,6656',20,5),
 (4105635,'naboo.@naboo_region_names:kaadara.@terminal_name:terminal_bazaar.4105635#5370,6659',20,5),
 (4235511,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.4235511#-1294,-3601',27,8),
 (4235514,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.4235514#-1290,-3578',27,8),
 (4385489,'talus.@talus_region_names:nashal.@terminal_name:terminal_bazaar.4385489#4520,5430',25,7),
 (4385490,'talus.@talus_region_names:nashal.@terminal_name:terminal_bazaar.4385490#4516,5431',25,7),
 (4385491,'talus.@talus_region_names:nashal.@terminal_name:terminal_bazaar.4385491#4492,5440',25,7),
 (4385492,'talus.@talus_region_names:nashal.@terminal_name:terminal_bazaar.4385492#4488,5441',25,7),
 (4395373,'corellia.@corellia_region_names:daoba_guerfel.@terminal_name:terminal_bazaar.4395373#3414,5670',8,0),
 (4395374,'corellia.@corellia_region_names:daoba_guerfel.@terminal_name:terminal_bazaar.4395374#3410,5672',8,0),
 (4505545,'rori.@rori_region_names:rebel_outpost.@terminal_name:terminal_bazaar.4505545#3664,-6422',23,6),
 (4636084,'talus.@talus_region_names:nashal.@terminal_name:terminal_bazaar.4636084#4332,5444',25,7),
 (4655386,'naboo.@naboo_region_names:lake_retreat.@terminal_name:terminal_bazaar.4655386#-5488,-6',37,5),
 (4685571,'tatooine.@tatooine_region_names:mos_eisley.@terminal_name:terminal_bazaar.4685571#3485,-4961',29,8),
 (4685572,'tatooine.@tatooine_region_names:mos_eisley.@terminal_name:terminal_bazaar.4685572#3513,-4927',29,8),
 (4685573,'tatooine.@tatooine_region_names:mos_eisley.@terminal_name:terminal_bazaar.4685573#3516,-4958',29,8),
 (4685574,'tatooine.@tatooine_region_names:mos_eisley.@terminal_name:terminal_bazaar.4685574#3482,-4931',29,8),
 (4685580,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.4685580#1285,3159',30,8),
 (4685581,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.4685581#1435,3266',30,8),
 (4685582,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.4685582#1432,3323',30,8),
 (4685589,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.4685589#-2988,2315',28,8),
 (4685590,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.4685590#-2972,2342',28,8),
 (4685591,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.4685591#-2998,2358',28,8),
 (4685592,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.4685592#-3014,2332',28,8),
 (4685593,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.4685593#-2813,2201',28,8),
 (4695458,'rori.@rori_region_names:restuss.@terminal_name:terminal_bazaar.4695458#5305,5832',22,6),
 (4695459,'rori.@rori_region_names:restuss.@terminal_name:terminal_bazaar.4695459#5302,5829',22,6),
 (4695462,'rori.@rori_region_names:restuss.@terminal_name:terminal_bazaar.4695462#5307,5823',22,6),
 (4695463,'rori.@rori_region_names:restuss.@terminal_name:terminal_bazaar.4695463#5310,5826',22,6),
 (4695696,'rori.@rori_region_names:narmle.@terminal_name:terminal_bazaar.4695696#-5432,-2114',21,6),
 (4695697,'rori.@rori_region_names:narmle.@terminal_name:terminal_bazaar.4695697#-5437,-2120',21,6),
 (4695698,'rori.@rori_region_names:narmle.@terminal_name:terminal_bazaar.4695698#-5435,-2117',21,6),
 (4695699,'rori.@rori_region_names:narmle.@terminal_name:terminal_bazaar.4695699#-5429,-2111',21,6),
 (4705366,'lok.@lok_region_names:lok_nyms_stronghold.@terminal_name:terminal_bazaar.4705366#459,5507',14,4),
 (4705367,'lok.@lok_region_names:lok_nyms_stronghold.@terminal_name:terminal_bazaar.4705367#462,5507',14,4),
 (4705376,'corellia.@corellia_region_names:vreni_island.@terminal_name:terminal_bazaar.4705376#-5542,-6211',9,0),
 (4705377,'corellia.@corellia_region_names:vreni_island.@terminal_name:terminal_bazaar.4705377#-5542,-6209',9,0),
 (4725481,'rori.@rori_region_names:narmle.@terminal_name:terminal_bazaar.4725481#-5088,-2475',21,6),
 (4725482,'rori.@rori_region_names:narmle.@terminal_name:terminal_bazaar.4725482#-5132,-2475',21,6),
 (4725483,'rori.@rori_region_names:restuss.@terminal_name:terminal_bazaar.4725483#5181,5562',22,6),
 (4725484,'rori.@rori_region_names:restuss.@terminal_name:terminal_bazaar.4725484#5181,5605',22,6),
 (4775555,'talus.@talus_region_names:dearic.@terminal_name:terminal_bazaar.4775555#170,-2949',24,7),
 (4775556,'talus.@talus_region_names:dearic.@terminal_name:terminal_bazaar.4775556#170,-2944',24,7),
 (4775557,'talus.@talus_region_names:dearic.@terminal_name:terminal_bazaar.4775557#171,-2919',24,7),
 (4775558,'talus.@talus_region_names:dearic.@terminal_name:terminal_bazaar.4775558#171,-2913',24,7),
 (4776016,'naboo.@naboo_region_names:kaadara.@terminal_name:terminal_bazaar.4776016#5060,6724',20,5),
 (4776017,'naboo.@naboo_region_names:kaadara.@terminal_name:terminal_bazaar.4776017#5064,6727',20,5),
 (4776018,'naboo.@naboo_region_names:kaadara.@terminal_name:terminal_bazaar.4776018#5095,6696',20,5),
 (4776019,'naboo.@naboo_region_names:kaadara.@terminal_name:terminal_bazaar.4776019#5091,6693',20,5),
 (4776020,'naboo.@naboo_region_names:moenia.@terminal_name:terminal_bazaar.4776020#4787,-4853',26,5),
 (4776021,'naboo.@naboo_region_names:moenia.@terminal_name:terminal_bazaar.4776021#4818,-4849',26,5),
 (4776022,'naboo.@naboo_region_names:moenia.@terminal_name:terminal_bazaar.4776022#4821,-4880',26,5),
 (4776023,'naboo.@naboo_region_names:moenia.@terminal_name:terminal_bazaar.4776023#4791,-4884',26,5),
 (4776039,'naboo.@naboo_region_names:deeja_peak.@terminal_name:terminal_bazaar.4776039#5339,-1560',19,5),
 (4776042,'naboo.@naboo_region_names:deeja_peak.@terminal_name:terminal_bazaar.4776042#4937,-1502',19,5),
 (4776043,'naboo.@naboo_region_names:deeja_peak.@terminal_name:terminal_bazaar.4776043#4937,-1445',19,5),
 (4776044,'naboo.@naboo_region_names:deeja_peak.@terminal_name:terminal_bazaar.4776044#4731,-1342',19,5),
 (4776048,'naboo.@naboo_region_names:keren.@terminal_name:terminal_bazaar.4776048#1765,2614',24,5),
 (4776049,'naboo.@naboo_region_names:keren.@terminal_name:terminal_bazaar.4776049#1786,2636',24,5),
 (4776050,'naboo.@naboo_region_names:keren.@terminal_name:terminal_bazaar.4776050#1787,2593',24,5),
 (4795392,'talus.@talus_region_names:dearic.@terminal_name:terminal_bazaar.4795392#553,-3003',24,7),
 (4795393,'talus.@talus_region_names:dearic.@terminal_name:terminal_bazaar.4795393#527,-2977',24,7),
 (4795397,'talus.@talus_region_names:nashal.@terminal_name:terminal_bazaar.4795397#4525,5066',25,7),
 (4795398,'talus.@talus_region_names:nashal.@terminal_name:terminal_bazaar.4795398#4105,5289',25,7),
 (4795402,'talus.@talus_region_names:nashal.@terminal_name:terminal_bazaar.4795402#4349,5151',25,7),
 (4795403,'talus.@talus_region_names:nashal.@terminal_name:terminal_bazaar.4795403#4348,5179',25,7),
 (4855366,'naboo.@naboo_region_names:moenia.@terminal_name:terminal_bazaar.4855366#4659,-4618',18,5),
 (4855367,'naboo.@naboo_region_names:moenia.@terminal_name:terminal_bazaar.4855367#4656,-4622',18,5),
 (4855368,'naboo.@naboo_region_names:moenia.@terminal_name:terminal_bazaar.4855368#4676,-4593',18,5),
 (4855369,'naboo.@naboo_region_names:moenia.@terminal_name:terminal_bazaar.4855369#4673,-4597',18,5),
 (4865404,'dathomir.@dathomir_region_names:an_outpost.@terminal_name:terminal_bazaar.4865404#-6314,874',84,2),
 (4865405,'dathomir.@dathomir_region_names:an_outpost.@terminal_name:terminal_bazaar.4865405#-6326,874',84,2),
 (5265435,'rori.@rori_region_names:narmle.@terminal_name:terminal_bazaar.5265435#-5281,-2171',21,6),
 (5315981,'talus.@talus_region_names:an_imperial_outpost.@terminal_name:terminal_bazaar.5315981#-4,-1',26,7),
 (5315984,'talus.@talus_region_names:an_imperial_outpost.@terminal_name:terminal_bazaar.5315984#-2171,2344',26,7),
 (5316176,'corellia.@corellia_region_names:daoba_guerfel.@terminal_name:terminal_bazaar.5316176#3225,5371',8,0),
 (5316177,'corellia.@corellia_region_names:daoba_guerfel.@terminal_name:terminal_bazaar.5316177#3194,5402',8,0),
 (5316178,'corellia.@corellia_region_names:daoba_guerfel.@terminal_name:terminal_bazaar.5316178#3225,5402',8,0),
 (5316179,'corellia.@corellia_region_names:daoba_guerfel.@terminal_name:terminal_bazaar.5316179#3194,5371',8,0),
 (5515501,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.5515501#-1295,-3669',27,8),
 (5515502,'tatooine.@tatooine_region_names:bestine.@terminal_name:terminal_bazaar.5515502#-1252,-3660',27,8),
 (5585534,'corellia.@corellia_region_names:daoba_guerfel.@terminal_name:terminal_bazaar.5585534#3089,5013',8,0),
 (5625462,'corellia.@corellia_region_names:tyrena.@terminal_name:terminal_bazaar.5625462#-5122,-2368',5,0),
 (5625463,'corellia.@corellia_region_names:tyrena.@terminal_name:terminal_bazaar.5625463#-5091,-2368',5,0),
 (5625464,'corellia.@corellia_region_names:tyrena.@terminal_name:terminal_bazaar.5625464#-5091,-2400',5,0),
 (5625465,'corellia.@corellia_region_names:tyrena.@terminal_name:terminal_bazaar.5625465#-5122,-2400',5,0),
 (5625467,'corellia.@corellia_region_names:coronet.@terminal_name:terminal_bazaar.5625467#-56,-4621',4,0),
 (5625468,'corellia.@corellia_region_names:coronet.@terminal_name:terminal_bazaar.5625468#-78,-4599',4,0),
 (5625469,'corellia.@corellia_region_names:coronet.@terminal_name:terminal_bazaar.5625469#-55,-4577',4,0),
 (5625470,'corellia.@corellia_region_names:coronet.@terminal_name:terminal_bazaar.5625470#-34,-4599',4,0),
 (5705412,'lok.@lok_region_names:lok_nyms_stronghold.@terminal_name:terminal_bazaar.5705412#444,5050',14,4),
 (5705413,'lok.@lok_region_names:lok_nyms_stronghold.@terminal_name:terminal_bazaar.5705413#422,5028',14,4),
 (5705414,'lok.@lok_region_names:lok_nyms_stronghold.@terminal_name:terminal_bazaar.5705414#400,5050',14,4),
 (5705415,'lok.@lok_region_names:lok_nyms_stronghold.@terminal_name:terminal_bazaar.5705415#422,5072',14,4),
 (6035457,'corellia.@corellia_region_names:vreni_island.@terminal_name:terminal_bazaar.6035457#-5568,-6053',9,0),
 (6035509,'corellia.@corellia_region_names:kor_vella.@terminal_name:terminal_bazaar.6035509#-3117,2968',7,0),
 (6035510,'corellia.@corellia_region_names:kor_vella.@terminal_name:terminal_bazaar.6035510#-3120,2968',7,0),
 (6035511,'corellia.@corellia_region_names:kor_vella.@terminal_name:terminal_bazaar.6035511#-3147,2970',7,0),
 (6035512,'corellia.@corellia_region_names:kor_vella.@terminal_name:terminal_bazaar.6035512#-3150,2970',7,0),
 (6035513,'corellia.@corellia_region_names:kor_vella.@terminal_name:terminal_bazaar.6035513#-3151,2785',7,0),
 (6035514,'corellia.@corellia_region_names:kor_vella.@terminal_name:terminal_bazaar.6035514#-3151,2781',7,0),
 (6035515,'corellia.@corellia_region_names:kor_vella.@terminal_name:terminal_bazaar.6035515#-3790,3179',7,0),
 (6035516,'corellia.@corellia_region_names:kor_vella.@terminal_name:terminal_bazaar.6035516#-3777,3193',7,0),
 (6076306,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.6076306#1284,2953',30,8),
 (6076307,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.6076307#1258,2926',30,8),
 (6076308,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.6076308#1258,2952',30,8),
 (6076309,'tatooine.@tatooine_region_names:mos_entha.@terminal_name:terminal_bazaar.6076309#1285,2926',30,8),
 (6255435,'talus.@talus_region_names:dearic.@terminal_name:terminal_bazaar.6255435#715,-3049',24,7),
 (6445440,'rori.@rori_region_names:restuss.@terminal_name:terminal_bazaar.6445440#5242,5790',22,6),
 (6445441,'rori.@rori_region_names:restuss.@terminal_name:terminal_bazaar.6445441#5242,5787',22,6),
 (6645628,'endor.@endor_region_names:endor_smuggler_outpost.@terminal_name:terminal_bazaar.6645628#-3,0',12,3),
 (6645630,'endor.@endor_region_names:endor_smuggler_outpost.@terminal_name:terminal_bazaar.6645630#-3,-4',12,3),
 (7275354,'dantooine.@dantooine_region_names:dantooine_imperial_outpost.@terminal_name:terminal_bazaar.7275354#-6,-3',3,1),
 (7495394,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.7495394#-3115,2158',28,8),
 (7495607,'tatooine.@tatooine_region_names:mos_espa.@terminal_name:terminal_bazaar.7495607#-2909,1927',28,8),
 (7755372,'corellia.@corellia_region_names:coronet.@terminal_name:terminal_bazaar.7755372#-340,-4644',4,0),
 (7755375,'corellia.@corellia_region_names:coronet.@terminal_name:terminal_bazaar.7755375#-312,-4644',4,0),
 (7925512,'yavin4.@yavin4_region_names:yavin4_mining_outpost.@terminal_name:terminal_bazaar.7925512#-312,4854',34,9),
 (7925513,'yavin4.@yavin4_region_names:yavin4_mining_outpost.@terminal_name:terminal_bazaar.7925513#-316,4854',34,9),
 (8605394,'dantooine.@dantooine_region_names:dantooine_pirate_outpost.@terminal_name:terminal_bazaar.8605394#-1616,-6366',2,1),
 (8605395,'dantooine.@dantooine_region_names:dantooine_pirate_outpost.@terminal_name:terminal_bazaar.8605395#-1561,-6366',2,1),
 (4294967817,'taanab.@taanab_region_names:pandath.@terminal_name:terminal_bazaar.4294967817#321,3878',36,42),
 (4294967818,'taanab.@taanab_region_names:pandath.@terminal_name:terminal_bazaar.4294967818#299,3901',36,42),
 (4294967819,'taanab.@taanab_region_names:pandath.@terminal_name:terminal_bazaar.4294967819#277,3879',36,42),
 (4294967820,'taanab.@taanab_region_names:pandath.@terminal_name:terminal_bazaar.4294967820#299,3857',36,42),
 (4294968325,'tutorial.Bazaar Terminal Room.@terminal_name:terminal_bazaar.4294968325#38,-18',97,41);
/*!40000 ALTER TABLE `commerce_bazaar` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;