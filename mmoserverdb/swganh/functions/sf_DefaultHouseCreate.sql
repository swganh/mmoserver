-- --------------------------------------------------------------------------------
-- Routine DDL
-- Note: comments before and after the routine body will not be stored by the server
-- --------------------------------------------------------------------------------
DELIMITER $$

CREATE DEFINER=`root`@`localhost` FUNCTION `sf_DefaultHouseCreate`(type_id INT(11), parent_id BIGINT(20), privateowner_id BIGINT(20), inPlanet INT, oX FLOAT, oY FLOAT, oZ FLOAT, oW FLOAT, inX FLOAT, inY FLOAT, inZ FLOAT, custom_name CHAR(255), deed_id BIGINT(20)) RETURNS bigint(20)
BEGIN

  
  DECLARE tmpCells INT;
  DECLARE structureID BIGINT(20);
  DECLARE structureCellID BIGINT(20);
  DECLARE attr_id INT;
  DECLARE attr_order INT;
  DECLARE attr_value CHAR(255);
  DECLARE loop_counter INT DEFAULT 0;
  DECLARE loopEnd INT DEFAULT 0;
  DECLARE startingCellID BIGINT DEFAULT 0;
  DECLARE TerminalOffset INT DEFAULT 0;
  DECLARE TerminalCell BIGINT DEFAULT 0;
  DECLARE t_value INT DEFAULT 0;

  
  

  DECLARE terminal_count INT;        
  DECLARE final_terminal_id INT;
  DECLARE final_terminal_type INT;
  DECLARE final_structure_type INT;
  DECLARE final_cell_id INT;
  DECLARE final_x FLOAT;
  DECLARE final_y FLOAT;
  DECLARE final_z FLOAT;
  DECLARE final_qx FLOAT;
  DECLARE final_qy FLOAT;
  DECLARE final_qz FLOAT;
  DECLARE final_qw FLOAT;
  DECLARE final_planet INT DEFAULT 99;

  
  

  DECLARE cur_1 CURSOR FOR SELECT attribute_id,attribute_value,attribute_order FROM swganh.structure_attribute_defaults WHERE structure_attribute_defaults.structure_type=type_id;
  DECLARE CTerminal CURSOR FOR SELECT * FROM swganh.structure_terminal_link WHERE structure_type = type_id;
  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET loopEnd = 1;


  SELECT cellcount FROM structure_type_data WHERE structure_type_data.type = type_id LIMIT 1 INTO tmpCells;

  
  SELECT MAX(id) FROM structures into structureID;


  IF structureID IS NULL THEN SET structureID = 2204928835634;
    ELSE SET structureID = structureID + 1;
  END IF;

  
  INSERT INTO structures VALUES (structureID, type_id, oX, oY, oZ, oW, inX, inY, inZ, custom_name, privateowner_id, 0, 0, inPlanet, 0);

  INSERT INTO houses VALUES (structureID, 0);  


  SELECT MAX(id) FROM structure_cells INTO structureCellID;

  IF structureCellID IS NULL THEN SET structureCellID = 2210000000000;
    ELSE SET structureCellID = structureCellID + 1;
  END IF;

  
  

	SET loop_counter = 0;

	loop1: LOOP

	  IF loop_counter = tmpCells THEN
      	LEAVE loop1;
    	ELSE SET loop_counter = loop_counter + 1;
	  END IF;

  	INSERT INTO structure_cells VALUES (structureCellID, structureID);

    SET structureCellID = structureCellID + 1;

	END LOOP loop1;

  
  

  SELECT id FROM structure_cells WHERE structure_cells.parent_id = structureID ORDER BY id LIMIT 1 INTO startingCellID;

  SET loopend = 0;

  
  

  SELECT COUNT(*) from structure_terminal_link WHERE id = type_id INTO terminal_count;

  

    OPEN CTerminal;
      REPEAT
      FETCH CTerminal INTO
       final_terminal_id,
       final_terminal_type,
       final_structure_type,
       final_cell_id,
       final_x,
       final_y,
       final_z,
       final_qx,
       final_qy,
       final_qz,
       final_qw;

  
  

  SET TerminalCell = startingCellID + final_cell_id;

      IF NOT loopEnd THEN

        INSERT INTO terminals VALUES (NULL, TerminalCell, final_terminal_type, final_qx, final_qy, final_qz, final_qw, final_x, final_y, final_z, final_planet, NULL, 0, 0, 0, NULL);

      END IF;

    UNTIL loopEnd END REPEAT;

    CLOSE CTerminal;

  

  INSERT INTO structure_admin_data VALUES (NULL, structureID, privateowner_id, 'ADMIN');

 SET loopend = 0;
  OPEN cur_1;
    REPEAT
      FETCH cur_1 INTO attr_id, attr_value, attr_order;
        IF NOT loopEnd THEN
          INSERT INTO structure_attributes VALUES (structureID, attr_id, attr_value, attr_order, NULL);

          SELECT ia.value FROM item_attributes ia WHERE ia.item_id = deed_id AND ia.attribute_id = attr_id INTO t_value;

          IF t_value IS NOT NULL THEN

            UPDATE structure_attributes sa SET sa.value = t_value WHERE sa.structure_id = structureID AND sa.attribute_id = attr_id;

          END IF;
        END IF;
    UNTIL loopEnd END REPEAT;

    CLOSE cur_1;

  
  

  SELECT MAX(id) FROM structures INTO structureID;

  RETURN(structureID);

  END