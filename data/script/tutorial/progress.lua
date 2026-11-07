-- Tutorial progress: what the player has done, recorded from script events
-- from the moment the tutorial starts, so a step can see that the player
-- did it before the script got there (a player racing ahead) instead of
-- waiting for it again.
--
--   local Progress = require 'script/tutorial/progress'
--   local progress = Progress.new(tutorial, playerId)
--   progress:say("welcome", 1267, 5000)
--   progress:waitFor(function() return progress.chatted end,
--                    function() progress:say("repeatchatprompt", 1294, 5000) end, 25000, 30000)
--   progress:stop()
--
-- Events used: docs/SCRIPTING.md, "Events".

local Progress = {}
Progress.__index = Progress

-- What the client reports when asked (NewbieTutorialRequest). Each request
-- gets one report, so a report is asked for again once it arrives.
local CLIENT_REPORTS = { "zoomCamera", "closeHolocron", "openInventory", "closeInventory" }

local TICK = 250  -- ms between checks while waiting

function Progress.new(tutorial, playerId)
  local p = setmetatable({
    tutorial = tutorial,
    player = playerId,
    handlers = {},
    now = 0,              -- ms of waiting done by this script (its clock)
    busyUntil = 0,        -- when the current narration ends
    reports = {},         -- client report -> how many times
    pending = {},         -- client reports to ask for again
    inventoryOpen = false,
    chatted = false,
    opened = {},          -- container id -> true
    closed = {},          -- container id -> true
    taken = {},           -- container id -> items taken from it
    talking = {},         -- npc id -> conversation started
    talked = {},          -- npc id -> conversation ended
    foodExamined = false,
    ate = false,
    cloned = false,
    insured = false,
  }, Progress)

  local function on(name, handler)
    p.handlers[#p.handlers + 1] = LuaScriptEngine.OnEvent(name, handler)
  end
  -- Events about the player (subject) and events the player caused (source).
  local function about(handler)
    return function(e) if e.subject == playerId then handler(e) end end
  end
  local function by(handler)
    return function(e) if e.source == playerId then handler(e) end end
  end

  on("client_report", about(function(e)
    p.reports[e.action] = (p.reports[e.action] or 0) + 1
    p.pending[e.action] = true
    if e.action == "openInventory" then p.inventoryOpen = true end
    if e.action == "closeInventory" then p.inventoryOpen = false end
  end))
  on("player_chat", about(function(e) p.chatted = true end))
  on("container_opened", by(function(e) p.opened[e.subject] = true end))
  on("container_closed", by(function(e) p.closed[e.subject] = true end))
  on("item_transferred", by(function(e)
    p.taken[e.from_container] = (p.taken[e.from_container] or 0) + 1
  end))
  on("conversation_started", by(function(e) p.talking[e.subject] = true end))
  on("conversation_ended", by(function(e) p.talked[e.subject] = true end))
  on("inventory_item_examined", by(function(e) if e.food == 1 then p.foodExamined = true end end))
  on("food_eaten", by(function(e) p.ate = true end))
  on("clone_saved", by(function(e) p.cloned = true end))
  on("items_insured", by(function(e) p.insured = true end))
  -- The handlers only touch this table; still, stop with the player.
  on("player_left_zone", about(function(e) p:stop() end))

  for _, action in ipairs(CLIENT_REPORTS) do
    tutorial:updateTutorial(action)
  end
  return p
end

function Progress:stop()
  for _, id in ipairs(self.handlers) do
    LuaScriptEngine.RemoveEventHandler(id)
  end
  self.handlers = {}
end

function Progress:reported(action)
  return (self.reports[action] or 0) > 0
end

-- Forget a report, to see the next one (e.g. closeHolocron before opening it).
function Progress:forget(action)
  self.reports[action] = nil
end

-- Ask the client again for the reports that came in.
function Progress:rearm()
  for action in pairs(self.pending) do
    self.tutorial:updateTutorial(action)
  end
  self.pending = {}
end

-- Sleep in ticks, so reports that arrive meanwhile are asked for again.
function Progress:sleep(ms)
  while ms > 0 do
    local step = math.min(ms, TICK)
    self:rearm()
    LuaScriptEngine.WaitMSec(step)
    self.now = self.now + step
    ms = ms - step
  end
  self:rearm()
end

-- Wait until the current narration (a prompt's sound) has finished.
function Progress:quiet()
  while self.now < self.busyUntil do
    self:sleep(TICK)
  end
end

-- A system message from newbie_tutorial/system_messages, its sound (or nil)
-- and how long the sound plays. Never talks over the previous one.
function Progress:say(message, sound, ms)
  self:quiet()
  self.tutorial:scriptSystemMessage("@newbie_tutorial/system_messages:" .. message)
  if sound then
    self.tutorial:scriptPlayMusic(sound)
  end
  self.busyUntil = self.now + (ms or 0)
end

-- Wait until done() is true. remind() is called once the player has had
-- firstMs to do it after the narration ended, then every everyMs.
function Progress:waitFor(done, remind, firstMs, everyMs)
  local idle = 0
  local nextReminder = firstMs
  while not done() do
    self:sleep(TICK)
    -- Count only ticks that were wholly after the narration.
    if self.now - TICK >= self.busyUntil then
      idle = idle + TICK
    end
    if remind and nextReminder and idle >= nextReminder then
      remind()
      idle = 0
      nextReminder = everyMs
    end
  end
end

return Progress
