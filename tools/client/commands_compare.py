#!/usr/bin/env python3
"""Compare the client's command table with the database's command_table.

    python3 -I tools/client/commands_compare.py datatables/command/command_table.iff [--sql]

Reads the client's datatables/command/command_table.iff (extracted from your
own client) and the swganh database (the `mariadb` client, default
options), and prints the differences in default time and combat queue use,
and the commands only one of them has. With --sql, prints SQL that makes
the database's times and queue use match the client.

Not compared: the maximum range (the client's 0 means the weapon's range;
the server checks the weapon's range itself) and the posture and state
masks. Nothing from the client is stored in the repository.
"""

import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import datatable  # noqa: E402


def query(sql):
    out = subprocess.run(["mariadb", "-N", "-B", "swganh", "-e", sql],
                         capture_output=True, text=True, check=True).stdout
    return [line.split("\t") for line in out.splitlines()]


def sql_string(text):
    return "'" + text.replace("\\", "\\\\").replace("'", "''") + "'"


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    want_sql = "--sql" in sys.argv[2:]

    columns, rows = datatable.read(sys.argv[1])
    index = {name: i for i, name in enumerate(columns)}
    client = {}
    for row in rows:
        # Some names carry a trailing space in the client's table.
        name = str(row[index["commandName"]]).strip()
        client[name.lower()] = {
            "name": name,
            "time": int(round(float(row[index["defaultTime"]]) * 1000)),
            "queue": int(row[index["addToCombatQueue"]]),
        }

    database = {}
    for name, time, queue in query("select commandname, defaulttime, addtocombatqueue from command_table"):
        database[name.lower()] = {"name": name, "time": int(float(time)), "queue": int(queue)}

    report, sql = [], []
    for key in sorted(set(client) & set(database)):
        c, d = client[key], database[key]
        if c["time"] != d["time"]:
            report.append("%s: default time client %d ms, database %d ms" % (d["name"], c["time"], d["time"]))
            sql.append("UPDATE command_table SET defaulttime = %d WHERE commandname = %s;"
                       % (c["time"], sql_string(d["name"])))
        if c["queue"] != d["queue"]:
            report.append("%s: combat queue client %d, database %d" % (d["name"], c["queue"], d["queue"]))
            sql.append("UPDATE command_table SET addtocombatqueue = %d WHERE commandname = %s;"
                       % (c["queue"], sql_string(d["name"])))
    for key in sorted(set(client) - set(database)):
        report.append("%s: only in the client" % client[key]["name"])
    for key in sorted(set(database) - set(client)):
        report.append("%s: only in the database (left as it is)" % database[key]["name"])

    print("\n".join(sql if want_sql else report))


if __name__ == "__main__":
    main()
