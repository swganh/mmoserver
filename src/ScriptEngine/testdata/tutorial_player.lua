-- A player going through the fake station (tutorial_world.lua). SLOW
-- players wait for the prompt before each action; others race ahead.

local function wait(ms) LuaScriptEngine.WaitMSec(ms or 250) end
local function raise(name, subject, source, fields)
  LuaScriptEngine.RaiseEvent(name, subject, source or 0, fields)
end
local function after(prompt)
  if SLOW then
    while not world.heardKey(prompt) do wait() end
    wait(2000)
  end
end
local function report(action) raise("client_report", PLAYER, 0, { action = action }); wait() end
local function talk(npc)
  raise("conversation_started", npc, PLAYER); wait(1000)
  world.conversationEffect(npc)
  raise("conversation_ended", npc, PLAYER); wait()
end
local function talkUntil(npc, substate)
  while world.substate < substate do talk(npc); wait(1000) end
end
local function enter(room)
  world.room = room
  raise("cell_entered", 2203318222959 + room, PLAYER); wait()
end
local function kill(typeId)
  while not world.spawned[typeId] do wait() end
  wait(1000)
  world.dead[world.spawned[typeId]] = true
  raise("creature_died", world.spawned[typeId], PLAYER); wait()
end

-- Part 1
after("mousewheel"); report("zoomCamera")
after("chatprompt"); raise("player_chat", PLAYER, 0, { text = "hello" }); wait()
while not world.holocronOpen do wait() end
after("holocube"); report("closeHolocron")
after("move_to_item_room"); enter(2)
-- Part 2
after("tut_08"); world.target = IDS.officer1; wait()
after("tut_11"); talk(IDS.officer1)
after("prompt_open_box"); world.target = IDS.drum; wait()
after("prompt_choose_open"); raise("container_opened", IDS.drum, PLAYER); wait()
after("prompt_take_items")
for i = 1, 2 do
  world.drumItems = world.drumItems - 1
  raise("item_transferred", 500 + i, PLAYER, { from_container = IDS.drum }); wait()
end
world.target = 0
after("explain_inventory"); report("openInventory")
after("prompt_find_food"); raise("inventory_item_examined", IDS.food, PLAYER, { food = 1 }); wait()
after("prompt_use_item"); raise("food_eaten", IDS.food, PLAYER); wait()
after("close_inventory"); report("closeInventory")
after("visit_commerce_room"); enter(3)
-- Part 3
after("tut_28"); talk(IDS.bankOfficer)
world.target = IDS.bank; wait(1000)
after("bank_info_4"); world.target = IDS.bazaar; wait(1000)
after("bazaar_info_3"); world.target = 0
-- Part 4
enter(4); enter(5)
after("clone_greeting"); talk(IDS.droid)
after("tut_37"); raise("clone_saved", IDS.cloning, PLAYER); wait()
after("tut_38"); raise("items_insured", IDS.insurance, PLAYER); wait()
enter(6)
-- Part 5
enter(7)
after("tut_40"); talkUntil(IDS.coward, 17)
enter(14); kill(4)
-- Part 6
enter(8); kill(5)
-- Part 7
after("loot_pirate"); enter(9)
after("tut_40"); talk(IDS.trainerOfficer)
world.trained = true
talk(world.spawned[77] or 0)
-- Part 8
enter(10); talkUntil(IDS.missionOfficer, 23)
-- Part 9
enter(11); talkUntil(IDS.quartermaster, 26)
after("make_promise"); while not world.heardKey("make_promise") do wait() end
wait(500)
raise("player_chat", PLAYER, 0, { text = "I promise" }); wait()

playerDone = true
