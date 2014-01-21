/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET FOREIGN_KEY_CHECKS=0 */;

DROP PROCEDURE IF EXISTS `sp_SaveService`;
DELIMITER //
CREATE PROCEDURE `sp_SaveService`(IN `in_address` VARCHAR(255), IN `in_tcp_port` INT, IN `in_udp_port` INT, IN `in_ping_port` INT, IN `in_status` INT, IN `in_pulse` VARCHAR(50), IN `in_id` INT)
BEGIN
UPDATE service SET address = INET_ATON(in_address), tcp_port = in_tcp_port, udp_port = in_udp_port,
 ping_port = in_ping_port, status = in_status, last_pulse = in_pulse WHERE id = in_id;
END//
DELIMITER ;
/*!40014 SET FOREIGN_KEY_CHECKS=1 */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
