-- Combat rules: what an attack does. The zone calls rules.attack for every
-- attack, by players and by creatures, and uses what it returns: whether
-- it hit, the damage, the pool it lands on and the time to the next attack.
--
-- Communities can rebalance combat here without rebuilding the server: the
-- numbers are at the top, the formulas below them. A zone reads this file
-- when it starts.
--
-- Sources (docs/SOURCES.md, docs/COMBAT.md): Prima's An Empire Divided
-- (2003), ch. 5 and the weapons appendix. The guide gives the rules in
-- words and the weapon and armour figures, but not the numbers behind
-- accuracy. Numbers marked "reconstructed" are ours, chosen to fit the
-- rules, and are the ones to tune.
--
-- What the zone passes (rules.attack(a)):
--   a.distance              metres between attacker and defender
--   a.damage_multiplier     the command's (specials), 1 for the default attack
--   a.delay_multiplier      the command's, 1 for the default attack
--   a.pool                  "health", "action" or "mind" if the command names
--                           one, nil for the default attack
--   a.special               true for a special move, false for the default attack
--   a.accuracy_bonus        the special's to-hit modifier
--   a.cost_multipliers      { health, action, mind }: the special's HAM cost
--                           modifiers (x the weapon's costs)
--   a.effects               effect -> { chance, duration }: "knockdown",
--                           "posture_down", "posture_up", "stun", "dizzy",
--                           "blind", "intimidate"
--   a.attacker, a.defender  sides:
--     kind                  "player", "creature" or "object" (lairs, debris)
--     posture               "standing", "kneeling", "prone" or "other"
--     skill_mods            name -> value (players)
--     armor_rating          0 none, 1 light, 2 medium, 3 heavy
--     armor                 damage type -> effectiveness in percent
--     to_hit                creature to-hit in percent, if known
--   a.weapon:
--     group                 "pistol", "carbine", "rifle", "unarmed",
--                           "onehandmelee", "twohandmelee", "polearm",
--                           "flamethrower", "saber", "launcher"
--     min_damage, max_damage, damage_type ("energy", "kinetic", ...)
--     speed                 seconds per attack
--     zero_mod, mid_mod, max_mod   accuracy at point blank, ideal and max range
--     mid_range, max_range  metres
--     armor_piercing        0-3, the weapon's armour rating
--     health_cost, action_cost, mind_cost   the weapon's attack costs
--
-- What it returns: { hit, chance, damage, pool, delay, effects } (delay in
-- seconds; effects: a list of { name, duration } that landed).
-- rules.costs(a) returns the health, action and mind a special costs.

local rules = {}

-- Accuracy -------------------------------------------------------------------

-- Chance to hit with an accuracy modifier of 0, in percent. The target
-- window shows the modifier; the guide: "the higher the number, the
-- better" (p. 73). Reconstructed.
rules.BASE_TO_HIT = 50
-- Each point of accuracy modifier adds this much to the chance. Reconstructed.
rules.TO_HIT_PER_POINT = 1
-- Bounds on the chance to hit. Reconstructed.
rules.MIN_TO_HIT = 5
rules.MAX_TO_HIT = 95

-- Posture (p. 73): kneeling is a bonus to your accuracy, prone a
-- significant one; against a kneeling or prone target, ranged attackers
-- have a penalty, melee attackers a bonus. Sizes reconstructed.
rules.ATTACKER_POSTURE = { standing = 0, kneeling = 10, prone = 20, other = 0 }
rules.DEFENDER_POSTURE_RANGED = { standing = 0, kneeling = -10, prone = -20, other = 0 }
rules.DEFENDER_POSTURE_MELEE = { standing = 0, kneeling = 10, prone = 20, other = 0 }

-- Skill mods that add to accuracy, speed and defence, by weapon group.
-- The names are the ones in skills_skillmods (Novice Marksman: +10
-- pistol_accuracy, +5 pistol_speed; p. 145).
rules.ACCURACY_MODS = {
  pistol = { "pistol_accuracy", "ranged_accuracy" },
  carbine = { "carbine_accuracy", "ranged_accuracy" },
  rifle = { "rifle_accuracy", "ranged_accuracy" },
  launcher = { "ranged_accuracy" },
  flamethrower = { "ranged_accuracy" },
  unarmed = { "unarmed_accuracy", "melee_accuracy" },
  onehandmelee = { "onehandmelee_accuracy", "melee_accuracy" },
  twohandmelee = { "twohandmelee_accuracy", "melee_accuracy" },
  polearm = { "polearm_accuracy", "melee_accuracy" },
  saber = { "melee_accuracy" },
}
rules.SPEED_MODS = {
  pistol = { "pistol_speed", "ranged_speed" },
  carbine = { "carbine_speed", "ranged_speed" },
  rifle = { "rifle_speed", "ranged_speed" },
  launcher = { "ranged_speed" },
  flamethrower = { "ranged_speed" },
  unarmed = { "unarmed_speed", "melee_speed" },
  onehandmelee = { "onehandmelee_speed", "melee_speed" },
  twohandmelee = { "twohandmelee_speed", "melee_speed" },
  polearm = { "polearm_speed", "melee_speed" },
  saber = { "melee_speed" },
}
rules.RANGED_DEFENSE_MODS = { "ranged_defense" }
rules.MELEE_DEFENSE_MODS = { "melee_defense" }

rules.RANGED = { pistol = true, carbine = true, rifle = true, launcher = true, flamethrower = true }

-- A weapon without range data (items made before the weapon data was
-- added) gets these, from the guide's typical weapon of the group
-- (pp. 390-393): the CDEF pistol, carbine and rifle, a melee weapon.
rules.DEFAULT_RANGES = {
  pistol = { mid_range = 15, max_range = 64 },
  carbine = { mid_range = 30, max_range = 64 },
  rifle = { mid_range = 50, max_range = 64 },
  launcher = { mid_range = 50, max_range = 64 },
  flamethrower = { mid_range = 10, max_range = 20 },
  melee = { mid_range = 3, max_range = 5 },
}

-- Distances are measured between centres; the client measures about this
-- much less, for the bodies (target window, docs/COMBAT.md: the CDEF's
-- range curve shows 2 m further out, and the target stays in range 2 m
-- past the max range). Ranges and range modifiers use the same, so the
-- chance rolled matches the one shown.
rules.RANGE_ALLOWANCE = 2

-- Damage ---------------------------------------------------------------------

-- Armour rating (p. 388): a weapon rated above the armour does 25% more
-- damage per level, below it 50% less per level (100 becomes 156 for two
-- levels above, 25 for two below).
rules.ARMOR_PIERCE_BONUS = 0.25
rules.ARMOR_PIERCE_PENALTY = 0.5

-- Where a default attack lands. The guide shows damage on one pool or
-- more (p. 85) but not how often each; reconstructed.
rules.POOL_WEIGHTS = { { "health", 50 }, { "action", 30 }, { "mind", 20 } }

-- Specials -------------------------------------------------------------------

-- Each point of the defender's defence mod takes this much off the chance
-- of the matching effect (Knockdown Defense, Stun Defense...). Reconstructed.
rules.EFFECT_DEFENSE_PER_POINT = 1
rules.EFFECT_DEFENSE_MODS = {
  knockdown = { "knockdown_defense" },
  posture_down = { "posture_change_down_defense" },
  posture_up = { "posture_change_up_defense" },
  stun = { "stun_defense" },
  dizzy = { "dizzy_defense" },
  blind = { "blind_defense" },
  intimidate = { "intimidate_defense" },
}
-- An effect the table gives no duration for lasts this long, in seconds:
-- the table's usual duration.
rules.DEFAULT_EFFECT_DURATION = 30
-- A posture change the table gives no duration for: how long, in seconds,
-- a creature stays down before it gets up. Players get up themselves.
-- Reconstructed.
rules.POSTURE_EFFECT_DURATION = 5
rules.POSTURE_EFFECTS = { knockdown = true, posture_down = true, posture_up = true }
-- What a special costs with a weapon that has no attack costs: bare hands
-- (the server's default weapon has no attributes). The guide's only
-- unarmed weapon, the vibroknuckler (p. 390), stands in. Reconstructed.
rules.DEFAULT_ATTACK_COSTS = {
  unarmed = { health = 15, action = 25, mind = 15 },
}
-- The order effects are tried in (the posture changes first).
rules.EFFECT_ORDER = { "knockdown", "posture_down", "posture_up", "stun", "dizzy", "blind", "intimidate" }

-- Brawler moves: Intimidate, Warcry, Taunt -------------------------------------

-- An intimidated creature is easier to hit (p. 47 of the 2004 guide); how
-- much easier is reconstructed. Added to the attacker's to-hit.
rules.DEFENDER_STATE_MODS = { intimidated = 10 }
-- Each move: what it does ("intimidate" or "taunt"), its chance in percent
-- (Intimidate's is the command table's intimidate_chance, 25 and 40), the
-- attacker's skill mod that adds to it and the defender's that takes from
-- it, and its reach: range to the target, or radius around the attacker for
-- Warcry, which intimidates every enemy around. The guide describes none of
-- the three moves; Warcry's and Taunt's chances and reaches are
-- reconstructed, Intimidate's 15 m is the command table's.
rules.MOVES = {
  intimidate1 = { kind = "intimidate", chance = 25, mod = "intimidate", defense = "intimidate_defense", range = 15 },
  intimidate2 = { kind = "intimidate", chance = 40, mod = "intimidate", defense = "intimidate_defense", range = 15 },
  warcry1 = { kind = "intimidate", chance = 25, mod = "warcry", defense = "intimidate_defense", radius = 15 },
  warcry2 = { kind = "intimidate", chance = 40, mod = "warcry", defense = "intimidate_defense", radius = 20 },
  taunt = { kind = "taunt", chance = 50, mod = "taunt", range = 15 },
}
-- How long an intimidation lasts, in seconds (the effects' usual duration).
rules.INTIMIDATE_DURATION = 30
-- No move is certain, nor hopeless. Reconstructed.
rules.MIN_MOVE_CHANCE = 5
rules.MAX_MOVE_CHANCE = 95


-- Self buffs: Berserk and Center of Being -------------------------------------

-- Berserk: to hit, speed and damage up, defence down (p. 46 of the 2004
-- guide; states, p. 84). Its strength is the level's own plus the brawler's
-- berserk skill mod (Master Brawler +20); duration in seconds. The guide
-- gives no numbers: all reconstructed.
rules.BERSERK = {
  berserk1 = { strength = 10, duration = 20 },
  berserk2 = { strength = 20, duration = 30 },
}
-- Per point of strength: to-hit gained, to-hit attackers gain against the
-- berserker, damage added (fraction) and attack time taken off (fraction).
rules.BERSERK_TO_HIT = 1
rules.BERSERK_DEFENSE_PENALTY = 1
rules.BERSERK_DAMAGE = 0.01
rules.BERSERK_SPEED = 0.01
-- Center of Being: defence up while fighting bare-handed or with a melee
-- weapon. Its strength is the <group>_center_of_being_efficacy mod of the
-- weapon held, its duration the center_of_being_duration_<group> mod times
-- this many seconds. Not in the guide; the duration scale and the defence
-- per point are reconstructed.
rules.CENTER_OF_BEING_GROUPS = { unarmed = true, onehandmelee = true, twohandmelee = true, polearm = true }
rules.CENTER_OF_BEING_SECONDS_PER_POINT = 3
rules.CENTER_OF_BEING_DEFENSE = 1

-- A hit wounds with the weapon's wound chance (p. 389: "Wound%"); a wound
-- lowers the pool's maximum until healed (p. 86). The guide gives no wound
-- size: a tenth of the damage, at least 1. Reconstructed.
rules.WOUND_FRACTION = 0.1
-- The wound chance of a weapon without one, by group, in percent: bare
-- hands use the vibroknuckler's 8% (p. 390). Reconstructed.
rules.DEFAULT_WOUND_CHANCE = { unarmed = 8 }
-- Battle fatigue is gained through combat; it does not affect combat but
-- hampers the healing of wounds (p. 86). The guide gives no rate: each hit
-- taken adds FATIGUE_PER_HIT (a quarter: four hits make 1, out of 1000)
-- times FATIGUE_MODIFIER. The server keeps the fractions. Reconstructed;
-- FATIGUE_MODIFIER is the knob for a server that wants it faster (2) or
-- slower (0.5), 0 turns it off.
rules.FATIGUE_PER_HIT = 0.25
rules.FATIGUE_MODIFIER = 1
-- Every wound makes the wounded pool bleed ("All moves that cause Health,
-- Action, or Mind wounds also cause bleeding to the affected stat", p. 75).
-- Bleeding slowly drains the pool until it is cured or runs its course,
-- and can incapacitate (p. 85). The guide gives no amounts: every
-- BLEED_INTERVAL seconds for BLEED_DURATION seconds the pool loses
-- BLEED_FRACTION of the wound, at least 1. Reconstructed.
rules.BLEED_FRACTION = 0.5
rules.BLEED_INTERVAL = 5
rules.BLEED_DURATION = 30


-- Three incaps within any 10 minutes kill (p. 85).
rules.INCAP_WINDOW = 600     -- seconds
rules.INCAP_DEATH = 3        -- the incap in the window that kills
-- How long each incap in the window lasts, in seconds; past the end of the
-- list the last entry repeats. The guide's picture of the incap timer
-- (p. 85) reads 52 s, so it is not a fixed 30 s; how it grows is
-- reconstructed: the first incap is short, the next one longer.
rules.INCAP_TIMES = { 30, 60 }

-- Attack time ----------------------------------------------------------------

-- Each point of speed mod takes this fraction off the weapon's attack
-- time. Reconstructed.
rules.SPEED_PER_POINT = 0.01
-- No attack comes sooner than this, in seconds. Reconstructed.
rules.MIN_DELAY = 1.0
-- Attack time for a weapon without one, in seconds (the old fixed 2 s).
rules.DEFAULT_SPEED = 2.0

-- Formulas -------------------------------------------------------------------

local function sumMods(side, names)
  local total = 0
  local mods = side and side.skill_mods
  if mods and names then
    for _, name in ipairs(names) do
      total = total + (mods[name] or 0)
    end
  end
  return total
end

function rules.isRanged(weapon)
  return weapon ~= nil and rules.RANGED[weapon.group] == true
end

-- The weapon's range data, with the group's defaults where it has none.
function rules.ranges(weapon)
  local defaults = rules.DEFAULT_RANGES[weapon.group] or rules.DEFAULT_RANGES.melee
  local mid = weapon.mid_range
  local max = weapon.max_range
  if not mid or mid <= 0 then mid = defaults.mid_range end
  if not max or max <= 0 then max = defaults.max_range end
  if max < mid then max = mid end
  return mid, max
end

-- Whether a target this far away can be attacked with the weapon.
function rules.inRange(weapon, distance)
  local _, max = rules.ranges(weapon)
  return distance <= max + rules.RANGE_ALLOWANCE
end

-- The weapon's accuracy modifier at a distance: from the point blank
-- modifier at 0 m to the ideal one at the ideal range, then to the max
-- one at the max range, in straight lines (p. 66). Beyond max range, the
-- max modifier.
function rules.rangeMod(weapon, distance)
  local zero = weapon.zero_mod or 0
  local mid = weapon.mid_mod or 0
  local max = weapon.max_mod or 0
  local midRange, maxRange = rules.ranges(weapon)
  if distance <= 0 then
    return zero
  end
  if distance <= midRange then
    return zero + (mid - zero) * distance / midRange
  end
  if distance >= maxRange or maxRange <= midRange then
    return max
  end
  return mid + (max - mid) * (distance - midRange) / (maxRange - midRange)
end

-- The attack's accuracy modifier: what the target window would show.
function rules.accuracy(a)
  local weapon = a.weapon
  local attacker = a.attacker or {}
  local defender = a.defender or {}
  local total = 0
  if weapon then
    total = total + rules.rangeMod(weapon, math.max((a.distance or 0) - rules.RANGE_ALLOWANCE, 0))
    total = total + sumMods(attacker, rules.ACCURACY_MODS[weapon.group])
  end
  total = total + (rules.ATTACKER_POSTURE[attacker.posture or "standing"] or 0)
  local ranged = rules.isRanged(weapon)
  local byPosture = ranged and rules.DEFENDER_POSTURE_RANGED or rules.DEFENDER_POSTURE_MELEE
  total = total + (byPosture[defender.posture or "standing"] or 0)
  total = total - sumMods(defender, ranged and rules.RANGED_DEFENSE_MODS or rules.MELEE_DEFENSE_MODS)
  -- A special's to-hit modifier (the special moves table, pp. 76-83).
  total = total + (a.accuracy_bonus or 0)
  -- The defender's states (intimidated...).
  for state in pairs(defender.states or {}) do
    total = total + (rules.DEFENDER_STATE_MODS[state] or 0)
  end
  -- Self buffs: the attacker's berserk; the defender's berserk and Center
  -- of Being.
  local attackerBuffs = attacker.buffs or {}
  local defenderBuffs = defender.buffs or {}
  total = total + (attackerBuffs.berserk or 0) * rules.BERSERK_TO_HIT
  total = total + (defenderBuffs.berserk or 0) * rules.BERSERK_DEFENSE_PENALTY
  total = total - (defenderBuffs.center_of_being or 0) * rules.CENTER_OF_BEING_DEFENSE
  return total
end

-- Chance to hit in percent.
function rules.hitChance(a)
  local defender = a.defender or {}
  if defender.kind == "object" then
    -- Lairs and debris do not dodge.
    return rules.MAX_TO_HIT
  end
  local base = rules.BASE_TO_HIT
  local attacker = a.attacker or {}
  if attacker.to_hit then
    base = attacker.to_hit
  end
  local chance = base + rules.accuracy(a) * rules.TO_HIT_PER_POINT
  if chance < rules.MIN_TO_HIT then chance = rules.MIN_TO_HIT end
  if chance > rules.MAX_TO_HIT then chance = rules.MAX_TO_HIT end
  return chance
end

-- Damage multiplier from the weapon's and the armour's ratings (p. 388).
function rules.armorRatingFactor(pierce, armor)
  pierce = pierce or 0
  armor = armor or 0
  if pierce > armor then
    return (1 + rules.ARMOR_PIERCE_BONUS) ^ (pierce - armor)
  elseif pierce < armor then
    return (1 - rules.ARMOR_PIERCE_PENALTY) ^ (armor - pierce)
  end
  return 1
end

-- The part of the damage the defender's armour lets through: its
-- effectiveness for the damage type, in percent (pp. 394-395).
function rules.armorPassThrough(defender, damageType)
  local armor = defender and defender.armor
  local effectiveness = armor and damageType and armor[damageType] or 0
  if effectiveness < 0 then effectiveness = 0 end
  if effectiveness > 100 then effectiveness = 100 end
  return 1 - effectiveness / 100
end

-- Damage of a hit; roll in [0, 1) picks it within the weapon's range.
function rules.damage(a, roll)
  local weapon = a.weapon or {}
  local min = weapon.min_damage or 1
  local max = weapon.max_damage or min
  if max < min then max = min end
  local damage = min + (max - min) * roll
  damage = damage * (a.damage_multiplier or 1)
  local buffs = a.attacker and a.attacker.buffs or {}
  damage = damage * (1 + (buffs.berserk or 0) * rules.BERSERK_DAMAGE)
  local defender = a.defender or {}
  damage = damage * rules.armorRatingFactor(weapon.armor_piercing, defender.armor_rating)
  damage = damage * rules.armorPassThrough(defender, weapon.damage_type)
  damage = math.floor(damage + 0.5)
  if damage < 1 then damage = 1 end
  return damage
end

-- The pool a hit lands on; roll in [0, 1).
function rules.pool(a, roll)
  if a.pool then
    return a.pool
  end
  local total = 0
  for _, entry in ipairs(rules.POOL_WEIGHTS) do
    total = total + entry[2]
  end
  local pick = roll * total
  for _, entry in ipairs(rules.POOL_WEIGHTS) do
    if pick < entry[2] then
      return entry[1]
    end
    pick = pick - entry[2]
  end
  return rules.POOL_WEIGHTS[#rules.POOL_WEIGHTS][1]
end

-- Seconds until the attacker's next attack: the weapon's attack speed,
-- less its speed mods, times the command's delay multiplier.
function rules.delay(a)
  local weapon = a.weapon
  local speed = weapon and weapon.speed
  if not speed or speed <= 0 then
    speed = rules.DEFAULT_SPEED
  end
  if weapon then
    local mods = sumMods(a.attacker, rules.SPEED_MODS[weapon.group])
    speed = speed * (1 - mods * rules.SPEED_PER_POINT)
  end
  speed = speed * (a.delay_multiplier or 1)
  local buffs = a.attacker and a.attacker.buffs or {}
  speed = speed * (1 - (buffs.berserk or 0) * rules.BERSERK_SPEED)
  if speed < rules.MIN_DELAY then
    speed = rules.MIN_DELAY
  end
  return speed
end

-- What the server tells the client about the weapon (WeaponRanges, object
-- controller 0x140), for the accuracy the target window shows. The wiki
-- gives the message as five INTs without names; worked out against the
-- target window (docs/COMBAT.md): the ideal and max range in metres
-- (floats), then the point blank, ideal and max modifiers (integers).
-- Return nothing to send no message.
--
-- The first value is the layout: one letter per field, "i" a 4-byte
-- integer, "f" a 4-byte float (at most 8 fields).

function rules.clientRanges(weapon)
  local midRange, maxRange = rules.ranges(weapon)
  return "ffiii", midRange, maxRange, weapon.zero_mod or 0, weapon.mid_mod or 0, weapon.max_mod or 0
end

-- What a special costs: its cost modifiers x the weapon's attack costs
-- (p. 389). Default attacks cost nothing.
function rules.costs(a)
  if not a.special then
    return 0, 0, 0
  end
  local weapon = a.weapon or {}
  local m = a.cost_multipliers or {}
  local function cost(base, multiplier)
    return math.floor((base or 0) * (multiplier or 0) + 0.5)
  end
  local h, ac, mi = weapon.health_cost or 0, weapon.action_cost or 0, weapon.mind_cost or 0
  local default = rules.DEFAULT_ATTACK_COSTS[weapon.group]
  if h == 0 and ac == 0 and mi == 0 and default then
    h, ac, mi = default.health, default.action, default.mind
  end
  return cost(h, m.health), cost(ac, m.action), cost(mi, m.mind)
end

-- An effect's chance against this defender, in percent.
function rules.effectChance(a, effect, chance)
  local defence = sumMods(a.defender, rules.EFFECT_DEFENSE_MODS[effect])
  chance = chance - defence * rules.EFFECT_DEFENSE_PER_POINT
  if chance < 0 then chance = 0 end
  if chance > 100 then chance = 100 end
  return chance
end

-- The effects a hit applies; roll() gives numbers in [0, 1).
function rules.effects(a, roll)
  local landed = {}
  local effects = a.effects or {}
  for _, name in ipairs(rules.EFFECT_ORDER) do
    local effect = effects[name]
    if effect and (effect.chance or 0) > 0 then
      if roll() * 100 < rules.effectChance(a, name, effect.chance) then
        local duration = effect.duration
        if (not duration or duration <= 0) and rules.POSTURE_EFFECTS[name] then
          duration = rules.POSTURE_EFFECT_DURATION
        elseif not duration or duration <= 0 then
          duration = rules.DEFAULT_EFFECT_DURATION
        end
        landed[#landed + 1] = { name = name, duration = duration }
      end
    end
  end
  return landed
end

function rules.attack(a)
  local chance = rules.hitChance(a)
  local result = { chance = chance, delay = rules.delay(a) }
  result.hit = math.random() * 100 < chance
  if result.hit then
    result.damage = rules.damage(a, math.random())
    result.pool = rules.pool(a, math.random())
    result.effects = rules.effects(a, math.random)
    result.wound = rules.wound(a, result.damage, math.random())
    result.bleed, result.bleed_interval, result.bleed_duration = rules.bleed(a, result.pool, result.wound)
    result.fatigue = rules.FATIGUE_PER_HIT * rules.FATIGUE_MODIFIER
  else
    result.damage = 0
  end
  return result
end

-- The wound a hit for this much damage leaves, given a roll in [0, 1).
function rules.wound(a, damage, roll)
  local weapon = a.weapon or {}
  local chance = weapon.wound_chance or 0
  if chance <= 0 then
    chance = rules.DEFAULT_WOUND_CHANCE[weapon.group] or 0
  end
  if damage <= 0 or roll * 100 >= chance then
    return 0
  end
  return math.max(1, math.floor(damage * rules.WOUND_FRACTION))
end

-- A brawler move's kind ("intimidate", "taunt"), its range to the target
-- and its radius around the attacker, in metres; nil for an unknown move.
function rules.moveReach(name)
  local move = rules.MOVES[name]
  if not move then
    return nil
  end
  return move.kind, move.range or 0, move.radius or 0
end

-- Whether a brawler move lands on a.defender, given a roll in [0, 1); its
-- chance in percent; and how long, in seconds, its state lasts. a.base_chance
-- (the command table's) replaces the move's own chance when set.
function rules.move(name, a, roll)
  local move = rules.MOVES[name]
  if not move then
    return false, 0, 0
  end
  local chance = (a.base_chance and a.base_chance > 0) and a.base_chance or move.chance
  chance = chance + sumMods(a.attacker, move.mod and { move.mod })
  chance = chance - sumMods(a.defender, move.defense and { move.defense })
  chance = math.max(rules.MIN_MOVE_CHANCE, math.min(rules.MAX_MOVE_CHANCE, chance))
  local duration = move.kind == "intimidate" and rules.INTIMIDATE_DURATION or 0
  return roll * 100 < chance, chance, duration
end

-- The bleeding a wound causes: what the pool loses per tick, the seconds
-- between ticks and how long it lasts. No wound, no bleeding.
function rules.bleed(a, pool, wound)
  if not wound or wound <= 0 then
    return 0, 0, 0
  end
  return math.max(1, math.floor(wound * rules.BLEED_FRACTION)), rules.BLEED_INTERVAL, rules.BLEED_DURATION
end

-- Scout harvesting ------------------------------------------------------------

-- A corpse yields the creature table's quantity of the meat, hide or bone
-- picked (both guides' tables; mmoserverdb/reference/). Each point of the
-- scout's creature_harvesting skill mod adds HARVEST_PER_POINT of it (Novice
-- Scout has 15); a harvest earns HARVEST_XP_PER_UNIT scout XP per unit.
-- Neither is in the guides at hand: reconstructed.
rules.HARVEST_PER_POINT = 0.01
rules.HARVEST_XP_PER_UNIT = 1

-- How many units a harvest of a table quantity gives, and the scout XP.
function rules.harvest(quantity, skill)
  if not quantity or quantity <= 0 then
    return 0, 0
  end
  local amount = math.floor(quantity * (1 + (skill or 0) * rules.HARVEST_PER_POINT))
  amount = math.max(1, amount)
  return amount, amount * rules.HARVEST_XP_PER_UNIT
end

-- A self buff (Berserk, Center of Being) for a.attacker holding a.weapon:
-- the buff ("berserk", "center_of_being"), its strength and its duration in
-- seconds; or nil and why not ("melee": Center of Being needs bare hands or
-- a melee weapon; "skill": no skill mods for it; "unknown").
function rules.selfBuff(name, a)
  local berserk = rules.BERSERK[name]
  if berserk then
    return "berserk", berserk.strength + sumMods(a.attacker, { "berserk" }), berserk.duration
  end
  if name == "centerofbeing" then
    local group = a.weapon and a.weapon.group or "unarmed"
    if not rules.CENTER_OF_BEING_GROUPS[group] then
      return nil, "melee"
    end
    local mods = a.attacker and a.attacker.skill_mods or {}
    local efficacy = mods[group .. "_center_of_being_efficacy"] or 0
    local duration = (mods["center_of_being_duration_" .. group] or 0) * rules.CENTER_OF_BEING_SECONDS_PER_POINT
    if efficacy <= 0 or duration <= 0 then
      return nil, "skill"
    end
    return "center_of_being", efficacy, duration
  end
  return nil, "unknown"
end

-- The count-th incap within the window (1 for the first): whether it kills,
-- and if not, how many seconds it lasts.
function rules.incap(count)
  if count >= rules.INCAP_DEATH then
    return true, 0
  end
  local times = rules.INCAP_TIMES
  return false, times[math.min(count, #times)] or 30
end

-- How long, in seconds, incaps are counted for before the count starts again.
function rules.incapWindow()
  return rules.INCAP_WINDOW
end

return rules
