-- Listing an item also deleted it from the database (the ZoneServer removed
-- it from the zone with ContainerManager::deleteObject; docs/KNOWN_ISSUES.md
-- #2j), so those listings name an item that no longer exists: nothing to
-- show, sell or retrieve. Remove them and their bids. The items cannot be
-- recovered.
DELETE h FROM swganh.commerce_bidhistory h
  JOIN swganh.commerce_auction ca ON ca.auction_id = h.auction_id
  LEFT JOIN swganh.items i ON i.id = ca.auction_id
  LEFT JOIN swganh.resource_containers r ON r.id = ca.auction_id
  WHERE i.id IS NULL AND r.id IS NULL;
DELETE ca FROM swganh.commerce_auction ca
  LEFT JOIN swganh.items i ON i.id = ca.auction_id
  LEFT JOIN swganh.resource_containers r ON r.id = ca.auction_id
  WHERE i.id IS NULL AND r.id IS NULL;
