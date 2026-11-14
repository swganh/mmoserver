-- sp_AccountForLogin: the account a login names, for the LoginServer to
-- check the password against (docs/KNOWN_ISSUES.md #3e).

DROP PROCEDURE IF EXISTS `sp_AccountForLogin`;

DELIMITER $$

CREATE PROCEDURE `sp_AccountForLogin`(IN usrName CHAR(255), IN pwrd VARCHAR(255), IN createUnknown TINYINT) SQL SECURITY INVOKER
BEGIN
  ##
  ## Use: CALL sp_AccountForLogin(username, password, createUnknown);
  ##
  ## Returns the account (not banned) with that username, whatever the
  ## password: the LoginServer checks it. legacy_match says whether the
  ## password matches an old unsalted SHA-1 one, the only form the server
  ## cannot check itself; it then stores an Argon2id hash instead.
  ##
  ## createUnknown: 1 creates an account for an unknown username with the
  ## password given (AutoCreateAccounts, for test servers); it is stored
  ## the old way and replaced at once, like any old password.

  DECLARE id BIGINT(20);
  DECLARE mStation_id BIGINT(20);

  SELECT account_id FROM swganh.account WHERE account_username = usrName INTO id;

  IF id IS NULL AND createUnknown = 1 THEN
    SELECT MAX(account_station_id) + 1 FROM swganh.account INTO mStation_id;
    IF mStation_id IS NULL THEN
      SET mStation_id = 1000000;
    END IF;

    -- As sp_ReturnUserAccount made them.
    INSERT INTO swganh.account
      (account_username, account_password, account_station_id, account_email, account_lastlogin,
       account_active, account_characters_allowed, account_lastcreate)
      VALUES (usrName, SHA1(pwrd), mStation_id, '', NOW(), 1, 1, NOW() - INTERVAL 1 DAY);
  END IF;

  SELECT account_id, account_username, account_password, account_station_id,
         account_banned, account_active, account_characters_allowed, account_csr,
         account_password = SHA1(pwrd) AS legacy_match
    FROM swganh.account
    WHERE account_banned = 0 AND account_username = usrName;
END $$

DELIMITER ;
