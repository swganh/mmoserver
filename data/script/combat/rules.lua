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
--
-- What it returns: { hit, chance, damage, pool, delay } (delay in seconds).

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

-- Distances are measured between centres; this much more is allowed for
-- the bodies. Reconstructed.
rules.RANGE_ALLOWANCE = 1

-- Damage ---------------------------------------------------------------------

-- Armour rating (p. 388): a weapon rated above the armour does 25% more
-- damage per level, below it 50% less per level (100 becomes 156 for two
-- levels above, 25 for two below).
rules.ARMOR_PIERCE_BONUS = 0.25
rules.ARMOR_PIERCE_PENALTY = 0.5

-- Where a default attack lands. The guide shows damage on one pool or
-- more (p. 85) but not how often each; reconstructed.
rules.POOL_WEIGHTS = { { "health", 50 }, { "action", 30 }, { "mind", 20 } }

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
    total = total + rules.rangeMod(weapon, a.distance or 0)
    total = total + sumMods(attacker, rules.ACCURACY_MODS[weapon.group])
  end
  total = total + (rules.ATTACKER_POSTURE[attacker.posture or "standing"] or 0)
  local ranged = rules.isRanged(weapon)
  local byPosture = ranged and rules.DEFENDER_POSTURE_RANGED or rules.DEFENDER_POSTURE_MELEE
  total = total + (byPosture[defender.posture or "standing"] or 0)
  total = total - sumMods(defender, ranged and rules.RANGED_DEFENSE_MODS or rules.MELEE_DEFENSE_MODS)
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
  if speed < rules.MIN_DELAY then
    speed = rules.MIN_DELAY
  end
  return speed
end

-- What the server tells the client about the weapon (WeaponRanges, object
-- controller 0x140), for the accuracy the target window shows. The wiki
-- gives the message as five INTs without names; the order below is a
-- guess (point blank at 0 m needs no distance of its own), to be checked
-- against the target window. Return five integers, or nothing to send no
-- message.
function rules.clientRanges(weapon)
  local midRange, maxRange = rules.ranges(weapon)
  local function int(x) return math.floor((x or 0) + 0.5) end
  return int(weapon.zero_mod), int(midRange), int(weapon.mid_mod), int(maxRange), int(weapon.max_mod)
end

function rules.attack(a)
  local chance = rules.hitChance(a)
  local result = { chance = chance, delay = rules.delay(a) }
  result.hit = math.random() * 100 < chance
  if result.hit then
    result.damage = rules.damage(a, math.random())
    result.pool = rules.pool(a, math.random())
  else
    result.damage = 0
  end
  return result
end

return rules
