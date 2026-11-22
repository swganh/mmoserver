"""What staff can do in game, and giving characters the admin commands.

The servers have two kinds of in-game staff power:

* The account's CSR flag (account.account_csr), set on the account page.
  The client is told the character is a CSR; the account may log in while
  the galaxy is locked; and the Character Builder terminals show the CSR
  menu (CharacterBuilderTerminal.cpp).

* The hidden admin profession: skills admin_skill_* in the skills table,
  whose skill commands are the abilities the admin slash commands need
  (command_table.characterability, checked by EVAbility). Nothing gives
  these skills in game; staff give them here, as character_skills rows.
  The ZoneServer reads a character's skills when it logs in.

Only the commands in COMMANDS have server code (OCAdminHandlers.cpp); the
client knows many more with the 'admin' ability that do nothing yet.
"""
from . import db

# The admin skills that grant working commands, in the order they build on
# each other; every other one needs admin_skill_novice.
NOVICE = "admin_skill_novice"
SKILLS = {
    "admin_skill_novice": "Messages: /broadcast and /admin_sys_msg",
    "admin_skill_01_01": "Planet and galaxy broadcasts",
    "admin_skill_02_01": "Galaxy shutdown and cancelling it",
    "admin_skill_03_01": "Warping yourself anywhere",
}

# (command, skill that grants it, syntax, what it does)
COMMANDS = [
    ("/broadcast", "admin_skill_novice", "/broadcast <message>",
     "System message to every player on your planet (your zone server)."),
    ("/broadcastPlanet", "admin_skill_01_01", "/broadcastPlanet <planet> <message>",
     "System message to every player on the planet named; part of a name is enough (e.g. tat)."),
    ("/broadcastGalaxy", "admin_skill_01_01", "/broadcastGalaxy <message>",
     "System message to every player in the galaxy."),
    ("/shutdownGalaxy", "admin_skill_02_01", "/shutdownGalaxy <minutes> <reason>",
     "Announces a shutdown of the galaxy in that many minutes, with the reason."),
    ("/cancelShutdownGalaxy", "admin_skill_02_01", "/cancelShutdownGalaxy <reason>",
     "Cancels a scheduled shutdown and tells the players why."),
    ("/admin_warp_self", "admin_skill_03_01", "/admin_warp_self <x> <z> [planet]",
     "Moves you to x, z on this planet, or on another (by name)."),
    ("/admin_sys_msg", "admin_skill_novice", "/admin_sys_msg <command> <arguments>",
     "Runs one of the commands above by name, which may be shortened, e.g. "
     "/admin_sys_msg broadcastG Server restart soon. Still needs that command's skill."),
]

CSR_FLAG_EFFECTS = [
    "The client is told the character is a CSR.",
    "The account can log in while the galaxy is locked (the ConnectionServer's 'l' key).",
    "Character Builder terminals show the CSR menu: Personal Blue Frog, experience, "
    "credits, buffs, items, resources, get an item by id, professions, wounds, states, "
    "and teleport to X,Y on the planet.",
]


class NoAccess(Exception):
    """The portal's database user may not read or change skills: install.sh
    install grants that."""


def _guarded(fn):
    def inner(*args, **kwargs):
        import pymysql

        try:
            return fn(*args, **kwargs)
        except pymysql.err.OperationalError as e:
            # 1142: command denied, 1143: column denied
            if e.args and e.args[0] in (1142, 1143):
                raise NoAccess(str(e)) from e
            raise
        except pymysql.err.ProgrammingError as e:
            if e.args and e.args[0] == 1146:   # table missing
                raise NoAccess(str(e)) from e
            raise

    return inner


@_guarded
def skill_ids():
    """{skill name: skill_id} for the admin skills in SKILLS."""
    marks = ",".join(["%s"] * len(SKILLS))
    rows = db.all(f"SELECT skill_id, skill_name FROM {{game}}.skills WHERE skill_name IN ({marks})", *SKILLS)
    return {r["skill_name"]: r["skill_id"] for r in rows}


@_guarded
def held(character_ids):
    """{character_id: set of admin skill names held}."""
    if not character_ids:
        return {}
    marks = ",".join(["%s"] * len(character_ids))
    rows = db.all(
        "SELECT cs.character_id, s.skill_name FROM {game}.character_skills cs "
        "JOIN {game}.skills s ON s.skill_id = cs.skill_id "
        f"WHERE s.skill_name LIKE 'admin\\_skill%%' AND cs.character_id IN ({marks})",
        *character_ids,
    )
    out = {}
    for r in rows:
        out.setdefault(r["character_id"], set()).add(r["skill_name"])
    return out


@_guarded
def holders():
    """Characters with any admin skill, with their account."""
    return db.all(
        "SELECT c.id, c.firstname, c.lastname, c.account_id, a.account_username, "
        "GROUP_CONCAT(s.skill_name ORDER BY s.skill_id) AS skills "
        "FROM {game}.character_skills cs "
        "JOIN {game}.skills s ON s.skill_id = cs.skill_id "
        "JOIN {game}.characters c ON c.id = cs.character_id "
        "JOIN {game}.account a ON a.account_id = c.account_id "
        "WHERE s.skill_name LIKE 'admin\\_skill%' "
        "GROUP BY c.id ORDER BY a.account_username, c.firstname"
    )


@_guarded
def set_skills(character_id, wanted):
    """Gives the character exactly the admin skills in `wanted` (of SKILLS),
    adding the novice skill the others build on. Returns (added, removed)."""
    wanted = {w for w in wanted if w in SKILLS}
    if wanted:
        wanted.add(NOVICE)
    ids = skill_ids()
    have = held([character_id]).get(character_id, set()) & set(SKILLS)
    added, removed = wanted - have, have - wanted
    with db.transaction():
        for name in removed:
            db.run("DELETE FROM {game}.character_skills WHERE character_id = %s AND skill_id = %s",
                   character_id, ids[name])
        for name in added:
            db.run("INSERT IGNORE INTO {game}.character_skills (character_id, skill_id) VALUES (%s, %s)",
                   character_id, ids[name])
    return added, removed


@_guarded
def client_only_commands():
    """Admin commands the client knows that the servers do not carry out yet."""
    working = {c[0].lstrip("/").lower() for c in COMMANDS}
    rows = db.all(
        "SELECT commandname FROM {game}.command_table WHERE characterability LIKE 'admin%' ORDER BY commandname"
    )
    return [r["commandname"] for r in rows if r["commandname"].lower() not in working]
