-- Bazaar listings made while the ChatServer dropped the update that follows
-- a new listing (docs/KNOWN_ISSUES.md #2i) have no category ("Unknown
-- Object Type"), region or planet. Fill them in as that update would: the
-- category from the item's type, the region and planet from the terminal.
-- Resource listings carry their category from the zone; those already made
-- without one are left as they are.
UPDATE swganh.commerce_auction ca
  JOIN swganh.items i ON i.id = ca.auction_id
  JOIN swganh.item_types it ON it.id = i.item_type
  SET ca.category = it.bazaar_category
  WHERE ca.category = 0;

UPDATE swganh.commerce_auction ca
  JOIN swganh.commerce_bazaar b ON b.bazaar_id = ca.bazaar_id
  SET ca.region_id = b.region_id, ca.planet_id = b.planet_id
  WHERE ca.region_id = 0 AND ca.planet_id = 0;
