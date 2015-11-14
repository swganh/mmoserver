-- --------------------------------------------------------------------------------
-- Routine DDL
-- Note: comments before and after the routine body will not be stored by the server
-- --------------------------------------------------------------------------------
DELIMITER $$

CREATE DEFINER=`root`@`localhost` FUNCTION `sf_DefaultHarvesterUpdateDeed`(parent_id BIGINT(20), owner_id BIGINT(20)) RETURNS bigint(20)
BEGIN

        DECLARE deed_id BIGINT(20);
        DECLARE att_id INT;
        DECLARE att_value CHAR(255);
        DECLARE t_value CHAR(255);
        DECLARE loopEnd INT DEFAULT 0;
        DECLARE cond INTEGER;
        DECLARE maintchar VARCHAR(128);
        DECLARE maint INTEGER;
        DECLARE rate INTEGER;

        DECLARE cur_1 CURSOR FOR SELECT sa.attribute_id, sa.value FROM structure_attributes sa WHERE sa.structure_id=parent_id;

        DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET loopEnd = 1;





        UPDATE  characters c INNER JOIN structure_cells sc ON (c.parent_id = sc.id) INNER JOIN structures h ON (h.id = sc.parent_id) SET c.x = h.x, c.y = h.y, c.z = h.z, c.parent_id = 0 WHERE h.id = parent_id;


        SELECT i.id FROM items i WHERE i.parent_id = parent_id AND item_family = 15 INTO deed_id;





        SELECT sa.value FROM structure_attributes sa WHERE sa.structure_id = parent_id AND sa.attribute_id = 382 INTO maintchar;
        SELECT CAST(maintchar AS SIGNED) INTO maint;

        SELECT st.maint_cost_wk FROM structures s INNER JOIN structure_type_data st ON (s.type = st.type) WHERE s.ID = parent_id  INTO rate;

        IF(maint < ((rate/168)*45))THEN
           DELETE FROM items WHERE id = deed_id;
           RETURN(1);
        END IF;

        SELECT CAST((maint-((rate/168)*45)) AS SIGNED) INTO maint;
        SELECT CAST(maint AS CHAR(128)) INTO maintchar;
        UPDATE structure_attributes sa SET sa.VALUE = maintchar WHERE sa.structure_id = parent_id AND sa.attribute_id = 382;

        IF loopEnd THEN
           RETURN(0);
        END IF;


        OPEN cur_1;
        REPEAT
                FETCH cur_1 INTO att_id,att_value;
                IF NOT loopEnd THEN
                        UPDATE  item_attributes ia SET ia.value = att_value WHERE ia.item_id = deed_id AND ia.attribute_id = att_id;
                END IF;
        UNTIL loopEnd END REPEAT;

        CLOSE cur_1;

        UPDATE  items i SET i.parent_id = owner_id WHERE i.parent_id = parent_id AND i.item_family = 15;

        RETURN(deed_id);
END