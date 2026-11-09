#!/usr/bin/env python3
"""Compare the client's skill table with the database's skills.

    python3 -I tools/client/skills_compare.py datatables/skill/skills.iff [--sql]

Reads the client's datatables/skill/skills.iff (extracted from your own
client) and the swganh database (the `mariadb` client, default options),
and prints every difference in skill mods, commands, XP cost and skill
points. With --sql, prints SQL that makes the database match the client.
Nothing from the client is stored in the repository.
"""

import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import datatable  # noqa: E402


def query(sql):
    out = subprocess.run(["mariadb", "-N", "-B", "swganh", "-e", sql],
                         capture_output=True, text=True, check=True).stdout
    return [line.split("\t") for line in out.splitlines()]


def client_skills(path):
    columns, rows = datatable.read(path)
    index = {name: i for i, name in enumerate(columns)}
    skills = {}
    for row in rows:
        name = row[index["NAME"]]
        # name=value pairs; the client's table has at least one missing
        # comma ("...=20heavy_particle_beam_accuracy=10"), so match pairs.
        mods = {key: int(value) for key, value in
                re.findall(r"([A-Za-z_][A-Za-z0-9_]*)=(-?\d+)", str(row[index["SKILL_MODS"]]))}
        commands = set(filter(None, (c.strip() for c in str(row[index["COMMANDS"]]).split(","))))
        skills[name] = {
            "points": int(row[index["POINTS_REQUIRED"]]),
            "xp_cost": int(row[index["XP_COST"]]),
            "money": int(row[index["MONEY_REQUIRED"]]),
            "mods": mods,
            "commands": commands,
        }
    return skills


def database_skills():
    skills = {}
    for sid, name, points, xp, money in query(
            "select skill_id, skill_name, skill_points_required, skill_xp_cost, skill_money_required from skills"):
        skills[name] = {"id": int(sid), "points": int(points or 0), "xp_cost": int(xp or 0),
                        "money": int(money or 0), "mods": {}, "commands": set()}
    by_id = {s["id"]: s for s in skills.values()}
    for sid, mod, value in query(
            "select sm.skill_id, m.skillmod_name, sm.value from skills_skillmods sm"
            " join skillmods m on m.skillmod_id = sm.skillmod_id"):
        if int(sid) in by_id:
            by_id[int(sid)]["mods"][mod] = int(value)
    for sid, command in query(
            "select sc.skill_id, c.name from skills_skillcommands sc"
            " join skillcommands c on c.id = sc.skillcommand_id"):
        if int(sid) in by_id:
            by_id[int(sid)]["commands"].add(command)
    return skills


def sql_string(text):
    return "'" + text.replace("\\", "\\\\").replace("'", "''") + "'"


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    want_sql = "--sql" in sys.argv[2:]
    client = client_skills(sys.argv[1])
    database = database_skills()
    # skillmods has no auto-increment and the server indexes mods by id, so
    # new names are left to a hand-written migration.
    known_mods = {row[0] for row in query("select skillmod_name from skillmods")}
    known_commands = {row[0].lower() for row in query("select name from skillcommands")}
    sql = []
    report = []

    for name in sorted(client):
        c = client[name]
        d = database.get(name)
        if d is None:
            report.append("%s: not in the database" % name)
            continue
        lines = []
        for field in ("points", "xp_cost", "money"):
            if c[field] != d[field]:
                lines.append("  %s: client %d, database %d" % (field, c[field], d[field]))
                column = {"points": "skill_points_required", "xp_cost": "skill_xp_cost",
                          "money": "skill_money_required"}[field]
                sql.append("UPDATE skills SET %s = %d WHERE skill_name = %s;"
                           % (column, c[field], sql_string(name)))
        for mod in sorted(set(c["mods"]) | set(d["mods"])):
            cv, dv = c["mods"].get(mod), d["mods"].get(mod)
            # A mod of 0 is the same as none.
            if (cv or 0) == (dv or 0):
                continue
            lines.append("  mod %s: client %s, database %s" % (mod, cv, dv))
            if cv is None:
                sql.append("DELETE sm FROM skills_skillmods sm JOIN skills s ON s.skill_id = sm.skill_id"
                           " JOIN skillmods m ON m.skillmod_id = sm.skillmod_id"
                           " WHERE s.skill_name = %s AND m.skillmod_name = %s;"
                           % (sql_string(name), sql_string(mod)))
            else:
                if mod not in known_mods:
                    report.append("  WARNING: skill mod %s is not in skillmods; add it first" % mod)
                sql.append("INSERT INTO skills_skillmods (skill_id, skillmod_id, value)"
                           " SELECT s.skill_id, m.skillmod_id, %d FROM skills s, skillmods m"
                           " WHERE s.skill_name = %s AND m.skillmod_name = %s"
                           " ON DUPLICATE KEY UPDATE value = VALUES(value);"
                           % (cv, sql_string(name), sql_string(mod)))
        # Command names are case-insensitive (startDance, startdance).
        client_commands = {x.lower(): x for x in c["commands"]}
        database_commands = {x.lower(): x for x in d["commands"]}
        for key in sorted(set(client_commands) ^ set(database_commands)):
            command = client_commands.get(key) or database_commands[key]
            where = "client" if key in client_commands else "database"
            lines.append("  command %s: only in the %s" % (command, where))
            if where == "database":
                sql.append("DELETE sc FROM skills_skillcommands sc JOIN skills s ON s.skill_id = sc.skill_id"
                           " JOIN skillcommands c ON c.id = sc.skillcommand_id"
                           " WHERE s.skill_name = %s AND c.name = %s;"
                           % (sql_string(name), sql_string(command)))
            else:
                if command.lower() not in known_commands:
                    report.append("  WARNING: command %s is not in skillcommands; add it first" % command)
                sql.append("INSERT IGNORE INTO skills_skillcommands (skill_id, skillcommand_id)"
                           " SELECT s.skill_id, c.id FROM skills s, skillcommands c"
                           " WHERE s.skill_name = %s AND c.name = %s;"
                           % (sql_string(name), sql_string(command)))
        if lines:
            report.append(name + ":")
            report.extend(lines)
    for name in sorted(set(database) - set(client)):
        # Skills only on the server (admin_skill...) are left as they are.
        report.append("%s: only in the database (left as it is)" % name)

    print("\n".join(sql if want_sql else report))


if __name__ == "__main__":
    main()
