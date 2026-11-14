-- Chat rooms created with no name (docs/KNOWN_ISSUES.md #2g): the ChatServer
-- cut the name out of the room path at the length of its own galaxy name,
-- which left it empty when the client's galaxy name differed. The client
-- listed such a room again on every refresh. The ChatServer now refuses an
-- empty name; this removes the rooms already made.
DELETE m FROM swganh.chat_channels_moderators m
  JOIN swganh.chat_channels c ON c.id = m.channel_id WHERE c.name = '';
DELETE i FROM swganh.chat_channels_invited i
  JOIN swganh.chat_channels c ON c.id = i.channel_id WHERE c.name = '';
DELETE b FROM swganh.chat_channels_banned b
  JOIN swganh.chat_channels c ON c.id = b.channel_id WHERE c.name = '';
DELETE FROM swganh.chat_channels WHERE name = '';
