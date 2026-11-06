-- Prints every script event to the zone's log, to see what the server knows
-- players are doing. A dev script: started on Tatooine by migration 002.
--   tail -f build/linux-debug/bin/logs/tatooine.out | grep 'event:'

local events = {
  "player_entered_zone", "player_left_zone",
  "creature_died",
  "conversation_started", "conversation_option_selected", "conversation_ended",
  "region_entered", "region_left",
  "object_used",
  "cell_entered", "building_left",
  "client_report",
}

local function describe(e)
  local keys = {}
  for k in pairs(e) do
    if k ~= "name" then table.insert(keys, k) end
  end
  table.sort(keys)
  local parts = {}
  for _, k in ipairs(keys) do
    table.insert(parts, k .. "=" .. string.format(type(e[k]) == "number" and "%.0f" or "%s", e[k]))
  end
  return table.concat(parts, " ")
end

for _, name in ipairs(events) do
  LuaScriptEngine.OnEvent(name, function(e)
    print("event: " .. name .. " " .. describe(e))
  end)
end

print("event_logger: listening to " .. #events .. " events")
