/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET FOREIGN_KEY_CHECKS=0 */;

DROP PROCEDURE IF EXISTS `sp_CreateGalaxy`;
DELIMITER //
CREATE PROCEDURE `sp_CreateGalaxy`(IN `in_name` VARCHAR(50), IN `in_version` VARCHAR(25), IN `in_status` TINYINT)
BEGIN
INSERT INTO galaxy(name, version, status, created_at, updated_at) VALUES(in_name, in_version, in_status, NOW(), NOW());
SELECT * FROM galaxy where id = LAST_INSERT_ID();
END//
DELIMITER ;
/*!40014 SET FOREIGN_KEY_CHECKS=1 */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
