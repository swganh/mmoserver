-- Mail attachments are binary (UTF-16 out-of-band data: the body of a
-- system mail, e.g. the tutorial's welcome letter, or a waypoint). In a
-- utf8mb3 TEXT column they were mangled, so such mails arrived empty
-- (KNOWN_ISSUES #22). sf_MailCreate takes them as BLOB too
-- (mmoserverdb/swganh/functions/sf_MailCreate.sql, loaded by update).
ALTER TABLE swganh.chat_mail MODIFY `attachments` BLOB;
