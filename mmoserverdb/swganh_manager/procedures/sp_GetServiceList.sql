/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET FOREIGN_KEY_CHECKS=0 */;

DROP PROCEDURE IF EXISTS `sp_GetServiceList`;
DELIMITER //
CREATE PROCEDURE `sp_GetServiceList`(IN `in_galaxy_id` INT)
BEGIN
SELECT id, galaxy_id, name, type, version, address, tcp_port, udp_port, ping_port, status, TIMESTAMP(last_pulse) as last_pulse_timestamp 
FROM service WHERE galaxy_id = in_galaxy_id ORDER BY service.type;
END//
DELIMITER ;
/*!40014 SET FOREIGN_KEY_CHECKS=1 */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
