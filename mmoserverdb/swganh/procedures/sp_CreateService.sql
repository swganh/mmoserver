/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET FOREIGN_KEY_CHECKS=0 */;

DROP PROCEDURE IF EXISTS `sp_CreateService`;
DELIMITER //
CREATE PROCEDURE `sp_CreateService`(IN `in_galaxy_id` INT, IN `in_name` VARCHAR(50), IN `in_type` VARCHAR(50), IN `in_version` INT, IN `in_address` VARCHAR(50), IN `in_tcp_port` INT, IN `in_udp_port` INT, IN `in_ping_port` INT, IN `in_status` INT)
BEGIN
INSERT INTO service (galaxy_id, name,type,version,address,tcp_port,udp_port,ping_port,status,last_pulse,created_at,updated_at)
VALUES(in_galaxy_id, in_name, in_type, in_version, INET_ATON(in_address), in_tcp_port, in_udp_port, in_ping_port, in_status, NOW(), NOW(), NOW());

SELECT id, status, CONVERT(TIMESTAMP(last_pulse), CHAR) as last_pulse_timestamp from service where id = LAST_INSERT_ID();
END//
DELIMITER ;
/*!40014 SET FOREIGN_KEY_CHECKS=1 */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
