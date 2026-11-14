-- Account passwords are Argon2id hashes in libsodium's string form (about
-- 97 characters, at most 127) instead of 40-character SHA-1 hex. Existing
-- SHA-1 passwords stay as they are until their account next logs in; the
-- LoginServer then replaces them (docs/KNOWN_ISSUES.md #3e).
ALTER TABLE swganh.account MODIFY account_password VARCHAR(128) NOT NULL COMMENT 'Argon2id hash, or SHA-1 hex until the next login';
