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
-- Definition of table `config_message_routes`
--

DROP TABLE IF EXISTS `config_message_routes`;
CREATE TABLE `config_message_routes` (
  `messageId` int(32) NOT NULL default '0',
  `processId` int(32) unsigned NOT NULL default '0',
  `description` char(255) NOT NULL,
  PRIMARY KEY  (`messageId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `config_message_routes`
--

/*!40000 ALTER TABLE `config_message_routes` DISABLE KEYS */;
INSERT INTO `config_message_routes` (`messageId`,`processId`,`description`) VALUES 
 (-2068110857,6,'0x84BB21F7 - ChatInstantMessageToCharacter'),
 (-1975546364,6,'0x8A3F8E04 - ChatRemoveModeratorFromRoom'),
 (-1893394879,6,'0x8F251641 - ChatDeletePersistantMessage'),
 (-1866602641,6,'0x90BDE76F - ChatAddModeratorToRoom'),
 (-1861069741,6,'0x91125453 - BidAuctionInstant'),
 (-1775334501,6,'0x962E8B9B - SearchKnowledgeBaseMessage'),
 (-1661816430,6,'0x9CF2b192 - ChatQueryRoom'),
 (-1632613473,6,'0x9EB04B9F - check_char_name - Checks a new characters name to make sure it is unique.  First name only.'),
 (-1257665162,1,'0xB5098D76 - SelectCharacter'),
 (-1198567066,1,'0xB88F5166 - ClusterZoneRegisterName'),
 (-1182846860,6,'0xB97F3074 - ClientChreateCharacter'),
 (-1133781518,6,'0xBC6BDDF2 - ChatEnterRoomById'),
 (-911869555,6,'0xC9A5F98D - GetTicketsMessage'),
 (-747701532,6,'0xD36EFAE4 - GetAuctionsDetails'),
 (-712404442,1,'0xD5899226 - ClientIdMsg'),
 (-690899247,6,'0xD6D1B6D1 - verify_character_name'),
 (-637927020,6,'0xD9FA0194 - ChatBanAvatarFromRoom'),
 (-124198305,6,'0xF898E25F - RequestCategoriesMessage'),
 (-57867791,6,'0xFC8D01F1 - ChatUninviteFromRoom'),
 (-32611253,6,'0xFE0E644B - GetAuctionsDetailsResponse'),
 (132339103,6,'0x07E3559F - ChatRequestPersistantMessage'),
 (180655684,6,'0x0AC49644 - AppendCommentMessage'),
 (313578569,6,'0x12B0D449 - RetrieveAuctionItemMessage'),
 (395986942,6,'0x179A47FE - ChatAvatarId'),
 (444250169,6,'0x1A7AB839 - GetMapLocationsMessage'),
 (551869411,6,'0x20E4DBE3 - ChatSendToRoom'),
 (565533243,6,'0x21B55A3B - IsVendorMessage'),
 (631414694,6,'0x25A29FA6 - ChatPersistantMessageToServer'),
 (655007429,6,'0x270A9EC5 - GetCommentsMessage'),
 (659508856,6,'0x274F4E78 - NewTicketActivityMessage'),
 (775311896,6,'0x2E365218 - ConnectPlayerMessage'),
 (830496480,6,'0x31805EE0 -'),
 (892759021,6,'0x35366BED - ChatCreateRoom'),
 (155921015,6,'0x94B2A77 - ChatDestroyRoom, noted as revered in wiki'),
 (914859218,6,'0x3687A4D2 - CancelLiveAuctionMessage'),
 (1088834988,6,'0x40E64DAC - CreateTicketMessage'),
 (1223988165,6,'0xD4E937FC - CommoditiesTypeList'),
 (1228816378,6,'0x493E3FFA - ChatRemoveAvatarFromRoom'),
 (1279077626,6,'0x4C3D2CFA - ChatRequestRoomList'),
 (1284478121,6,'0x4C8F94A9 - ChatUnbanAvatarFromRoom'),
 (1585137734,6,'0x5E7B4846 - GetArticleMessage'),
 (1670313009,6,'0x638EF431 - CancelTicketMessage'),
 (1738411264,6,'0x679E0D00 - AuctionQueryHeadersMessage'),
 (1811950867,6,'0x6C002D13 - ChatAddFriend'),
 (1825766616,6,'0x6CD2FCD8 - ChatFriendlistUpdate'),
 (1920199891,6,'0x7273ECD3 - ChatInviteAvatarToRoom');
/*!40000 ALTER TABLE `config_message_routes` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;