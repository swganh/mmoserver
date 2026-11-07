-- A fake tutorial station for tutorial_playthrough_unittest.cc: the C++
-- objects tutorial/part1.lua uses (Tutorial, ScriptSupport), backed by a
-- `world` table, and the conversation gates QuestGiver applies.

PLAYER = 8589960593
IDS = {
  officer1 = 47513079097, bankOfficer = 47513079099,
  bank = 4294968324, bazaar = 4294968325,
  droid = 47513085659, cloning = 4294968326, insurance = 4294968327,
  coward = 47513085665, trainerOfficer = 47513085681,
  missionOfficer = 47513085683, quartermaster = 47513085685,
  drum = 2533274790395904, food = 281474976710858,
}

world = {
  substate = 1, room = 1, target = 0, pos = { x = 0, y = 0, z = 0 },
  holocronOpen = false, drumItems = 2, trained = false,
  nextId = 900000, spawned = {}, dead = {}, heard = {},
}

local function key(message)
  return string.match(message, ":([%w_]+)$") or message
end

local T = {}
function T:getReady() return true end
function T:getPlayer() return PLAYER end
function T:getState() return 1 end
function T:getSubState() return world.substate end
function T:setSubState(n) world.substate = n end
function T:getRoom() return world.room end
function T:disableHudElement() end
function T:enableHudElement() end
function T:scriptSystemMessage(m) world.heard[#world.heard + 1] = key(m) end
function T:scriptPlayMusic() end
function T:updateTutorial() end
function T:openHolocron() world.holocronOpen = true end
function T:enableItemContainerEvent() end
function T:isContainerEmpty() return world.drumItems == 0 end
function T:spatialChat(id, m) world.heard[#world.heard + 1] = key(m) end
function T:spatialChatShout(id, m) world.heard[#world.heard + 1] = key(m) end
function T:getPlayerPosToObject() return 5 end
function T:enableNpcConversationEvent() end
function T:getPlayerPosX() return world.pos.x end
function T:getPlayerPosY() return world.pos.y end
function T:getPlayerPosZ() return world.pos.z end
function T:getSkillTrainerTypeId() return 77 end
function T:setTutorialCelebTaunts() end
function T:setTutorialRefugeeTaunts() end
function T:makeCreatureAttackable() end
function T:isLowHam() return false end
function T:npcStopFight() end
function T:isPlayerTrained() return world.trained end
function T:npcSendAnimation() end

ScriptEngine = { Init = function() return { getTutorial = function() return T end } end }

local S = {}
function S:npcCreate(typeId)
  world.nextId = world.nextId + 1
  world.spawned[typeId] = world.nextId
  return world.nextId
end
function S:objectIsReady() return true end
function S:npcGetObject(id) return id end
function S:npcSpawnPrivate() end
function S:itemCreate() world.nextId = world.nextId + 1; return world.nextId end
function S:eraseObject() end
function S:itemPopulateInventory() end
function S:getTarget() return world.target end
function S:sendFlyText() end
function S:npcIsDead(id) return world.dead[id] == true end
function S:npcInCombat() return false end
function S:npcKill(id) world.dead[id] = true end
function S:scriptSystemMessage() end
function S:setPlayerPosition() end

ScriptSupport = { Instance = function() return S end }

-- What QuestGiver does at the end of these conversations.
function world.conversationEffect(npc)
  if npc == IDS.coward and world.substate == 16 then world.substate = 17 end
  if npc == IDS.missionOfficer and world.substate == 22 then world.substate = 23 end
  if npc == IDS.quartermaster and world.substate == 25 then world.substate = 26 end
end

function world.heardKey(k)
  for _, h in ipairs(world.heard) do
    if h == k then return true end
  end
  return false
end
