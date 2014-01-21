CREATE DATABASE  IF NOT EXISTS `swganh` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `swganh`;
-- MySQL dump 10.13  Distrib 5.6.13, for Win32 (x86)
--
-- Host: localhost    Database: swganh
-- ------------------------------------------------------
-- Server version	5.7.3-m13-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `account`
--

DROP TABLE IF EXISTS `account`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `account` (
  `account_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Account ID',
  `account_username` char(32) NOT NULL DEFAULT '' COMMENT 'Account username',
  `account_password` char(64) NOT NULL COMMENT 'Account password',
  `account_station_id` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Account STATION_ID',
  `account_csr` int(1) NOT NULL DEFAULT '0' COMMENT 'Account - CSR Flag',
  `account_banned` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'Account - Banned Status',
  `account_email` char(64) NOT NULL DEFAULT '' COMMENT 'Account - User email',
  `account_joindate` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT 'Account - Join Date',
  `account_lastlogin` timestamp NULL DEFAULT NULL COMMENT 'Account - Last Login Timestamp',
  `account_active` tinyint(1) DEFAULT NULL COMMENT 'Account - Active Flag',
  `account_loggedin` int(1) NOT NULL DEFAULT '0' COMMENT 'Account - Cluster id account is logged into',
  `account_authenticated` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'Account - Authenticated Status',
  `account_characters_allowed` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT 'Number of characters allowed',
  `account_session_key` varchar(32) DEFAULT NULL COMMENT 'Client Launcher - Session Key',
  `account_lastcreate` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT 'Account - Last Character Create TimeStamp',
  `banned_until` datetime DEFAULT '0000-00-00 00:00:00',
  `banned_date` datetime DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`account_id`),
  UNIQUE KEY `account_username` (`account_username`)
) ENGINE=InnoDB AUTO_INCREMENT=30 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `account`
--

LOCK TABLES `account` WRITE;
/*!40000 ALTER TABLE `account` DISABLE KEYS */;
INSERT INTO `account` VALUES (1,'swganh0','41d14978522cc50b70e73f6f0007e82a000a95d5',100000001,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00'),(2,'swganh1','41d14978522cc50b70e73f6f0007e82a000a95d5',100000002,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00'),(3,'swganh2','41d14978522cc50b70e73f6f0007e82a000a95d5',100000003,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00'),(4,'swganh3','41d14978522cc50b70e73f6f0007e82a000a95d5',100000004,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00'),(5,'swganh4','41d14978522cc50b70e73f6f0007e82a000a95d5',100000005,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00'),(6,'swganh5','41d14978522cc50b70e73f6f0007e82a000a95d5',100000006,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00'),(7,'swganh6','41d14978522cc50b70e73f6f0007e82a000a95d5',100000007,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00'),(8,'swganh7','41d14978522cc50b70e73f6f0007e82a000a95d5',100000008,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00'),(9,'swganh8','41d14978522cc50b70e73f6f0007e82a000a95d5',100000009,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00'),(10,'swganh9','41d14978522cc50b70e73f6f0007e82a000a95d5',100000010,0,0,'test@swganh.org','0000-00-00 00:00:00','0000-00-00 00:00:00',1,0,0,2,'','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00');
/*!40000 ALTER TABLE `account` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2014-01-21 18:59:56
