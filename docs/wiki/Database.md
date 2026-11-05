The servers use **MariaDB**, on Linux and on Windows (tested with 10.11 and
11.8). MySQL is not supported: the schema and stored procedures rely on
MariaDB's defaults, and moving to MySQL would mean schema-wide changes (the
reasons are in the
[Roadmap](https://github.com/swganh/mmoserver/blob/master/docs/ROADMAP.md)).

Everything is done with `mmoserverdb/setup.sh`, run from the `mmoserver`
directory. It logs in to MariaDB as root through the local socket, so it
runs with `sudo`. `./mmoserverdb/setup.sh -h` lists every command.

## Installing

```bash
sudo apt install mariadb-server
sudo ./mmoserverdb/setup.sh install --public-address 192.168.1.10
```

This creates the databases, imports the game data, spawns the first set of
harvestable resources, removes the sample accounts, and creates a `swganh`
database user with a random password, allowed to connect from this machine
only. The settings go into `mmoserverdb/local.cfg` (not in git) and into
`config/general.cfg` of every build; later builds pick them up
automatically.

`--public-address` is the address game clients use to reach the server
(default `127.0.0.1`: clients on the same machine only).

`install` refuses to touch existing databases. `--force` drops and recreates
them: **all accounts and characters are lost**.

## Accounts

```bash
sudo ./mmoserverdb/setup.sh add-account myname
```

Asks for a password of at least 8 characters. Each account may have 2
characters by default; to allow more for one account:

```bash
sudo mariadb -e "UPDATE swganh.account SET account_characters_allowed = 4 WHERE account_username = 'myname'"
```

## The galaxy's address

The LoginServer tells clients where to connect. If the server's address
changes, or clients connect from other machines after an install without
`--public-address`:

```bash
sudo ./mmoserverdb/setup.sh set-address 192.168.1.10
./server.sh restart
```

## Updating the stored procedures

After a `git pull` that changed the procedures:

```bash
sudo ./mmoserverdb/setup.sh update-procedures
```

Only the procedures and functions are reloaded; game data, accounts and
characters are not touched.

## Resources

`install` spawns the harvestable resources. Each one expires after 1 to 22
days, and the zones replace it with a new resource of the same type at
startup and every 10 minutes, so nothing needs doing while the servers run.
For a database installed before this existed, or to start a new set:

```bash
sudo ./mmoserverdb/setup.sh spawn-resources            # if there are none
sudo ./mmoserverdb/setup.sh spawn-resources --replace  # retire the current set
```

Restart the zones afterwards.

## Backups

The installer has no backup command yet; `mariadb-dump` does it:

```bash
sudo mariadb-dump --routines --databases swganh swganh_config swganh_logs > mmoserver-backup.sql
sudo mariadb < mmoserver-backup.sql                     # restore
```

Stop the servers first for a consistent copy.

## Letting another machine use the database

For developing on a Windows PC against the database on the Linux machine
([[Windows Development]]):

```bash
sudo ./mmoserverdb/setup.sh allow-host 192.168.1.20    # the PC's address
```

This lets the `swganh` user connect from that one address and prints the
remaining steps (MariaDB's `bind-address`, the firewall). `deny-host` undoes
it. Never open port 3306 to the internet.

## Where things are

Almost everything is in the `swganh` database: `account`, `characters`,
`galaxy` (the address clients use), `resources`, the game data tables and
the server settings (`config_server`, `config_zone_scripts`). See
[[Architecture]].
