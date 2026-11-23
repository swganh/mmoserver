-- The portal's own tables. They live in a database of their own
-- (swganh_portal by default), so the game's schema is left as it is and a
-- community can drop the portal without touching the game. Accounts are the
-- game's swganh.account rows, referred to by account_id; there are no
-- foreign keys across the two databases, so reinstalling the game database
-- is never blocked by the portal.
--
-- {portal} is replaced with the configured database name by the installer.

CREATE TABLE IF NOT EXISTS {portal}.schema_version (
  version INT NOT NULL PRIMARY KEY,
  applied_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Staff roles. Administrator (builtin) always has every permission and is
-- the only role that can manage roles; its permissions are not stored.
CREATE TABLE IF NOT EXISTS {portal}.role (
  role_id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  name VARCHAR(64) NOT NULL,
  description VARCHAR(255) NOT NULL DEFAULT '',
  builtin TINYINT(1) NOT NULL DEFAULT 0,
  UNIQUE KEY role_name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS {portal}.role_permission (
  role_id INT UNSIGNED NOT NULL,
  permission VARCHAR(64) NOT NULL,
  PRIMARY KEY (role_id, permission),
  CONSTRAINT role_permission_role FOREIGN KEY (role_id) REFERENCES {portal}.role (role_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS {portal}.account_role (
  account_id BIGINT UNSIGNED NOT NULL,
  role_id INT UNSIGNED NOT NULL,
  granted_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  granted_by BIGINT UNSIGNED NULL,
  PRIMARY KEY (account_id, role_id),
  KEY account_role_role (role_id),
  CONSTRAINT account_role_role FOREIGN KEY (role_id) REFERENCES {portal}.role (role_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Password reset links. Only a SHA-256 of the token is stored, so a copy of
-- this table cannot be used to reset anyone's password.
CREATE TABLE IF NOT EXISTS {portal}.password_reset (
  reset_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  account_id BIGINT UNSIGNED NOT NULL,
  token_hash CHAR(64) NOT NULL,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  expires_at DATETIME NOT NULL,
  used_at DATETIME NULL,
  requested_by BIGINT UNSIGNED NULL COMMENT 'Staff account that sent it, or NULL when the player asked',
  UNIQUE KEY password_reset_token (token_hash),
  KEY password_reset_account (account_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Failed logins and reset requests, for rate limiting. Old rows are pruned.
CREATE TABLE IF NOT EXISTS {portal}.attempt (
  attempt_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  kind VARCHAR(16) NOT NULL,
  ip VARCHAR(45) NOT NULL,
  username VARCHAR(64) NOT NULL DEFAULT '',
  at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  KEY attempt_ip (kind, ip, at),
  KEY attempt_username (kind, username, at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- What staff (and players, for their own account) changed.
CREATE TABLE IF NOT EXISTS {portal}.audit_log (
  audit_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  actor_account_id BIGINT UNSIGNED NULL,
  target_account_id BIGINT UNSIGNED NULL,
  action VARCHAR(64) NOT NULL,
  detail VARCHAR(500) NOT NULL DEFAULT '',
  ip VARCHAR(45) NOT NULL DEFAULT '',
  KEY audit_target (target_account_id, at),
  KEY audit_actor (actor_account_id, at),
  KEY audit_at (at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO {portal}.role (role_id, name, description, builtin) VALUES
  (1, 'Administrator', 'Every permission, including managing roles and staff', 1),
  (2, 'CSR', 'Customer service: look up accounts, reset passwords, ban', 0);

INSERT IGNORE INTO {portal}.role_permission (role_id, permission) VALUES
  (2, 'admin.access'), (2, 'accounts.view'), (2, 'accounts.reset_password'),
  (2, 'accounts.ban'), (2, 'audit.view');

INSERT IGNORE INTO {portal}.schema_version (version) VALUES (1);
