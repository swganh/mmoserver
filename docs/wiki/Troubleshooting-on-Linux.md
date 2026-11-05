Common problems and what to check first. Logs are in
`build/linux-debug/bin/logs/`.

## The galaxy says "Loading..."

The tutorial and Tatooine zones must both be running before the galaxy
accepts logins. Zones take a minute or two to start:

```bash
./server.sh status
grep -l "startup complete" build/linux-debug/bin/logs/*.out
```

A zone that is not running usually lacks its terrain
(`./build_server.sh --heightmaps tatooine`): its log says so.

## The client gets no answer

Check the client's `login.cfg` points at the server's address
([[Game Client]]). Then watch for the client's packets on the server while
logging in:

```bash
sudo tcpdump -ni any udp port 44990
```

- **No packets**: a network problem between the machines (wrong address,
  different network).
- **Packets but no reply**: usually the firewall:
  `sudo ufw allow 44990:44992/udp`.

If login works but the client then hangs, the galaxy may point at the wrong
address. Set it to the address clients use and restart:

```bash
sudo ./mmoserverdb/setup.sh set-address 192.168.1.10
./server.sh restart
```

## A login or character creation is refused

`connection.out` says why, on a line starting `Refusing account`. Common
reasons: the account is logged in already (a quick reconnect after a client
crash takes over the old session), or the account's character limit is
reached (see [[Database]]).

## The servers cannot reach the database

The logs show a database error at startup. Check MariaDB runs
(`systemctl status mariadb`), and that the settings in
`build/linux-debug/bin/config/general.cfg` match `mmoserverdb/local.cfg`
(`./mmoserverdb/setup.sh apply-config build/linux-debug/bin/config` copies
them again).

## A server crashed

`./server.sh status` shows it as not running; the end of its log has the
last thing it did. For a backtrace, run it again under gdb:

```bash
./server.sh stop
./server.sh debug -g tutorial tatooine
```

When it crashes, the backtrace appears in its tmux window and its log.
Please report it (see [[Contributing]]) with the log attached.

## Known problems in game

Some things are known not to work yet, for example creature lairs never
spawn, and after a fight you can stay in combat (type `/peace`). The full
list, with what has been fixed, is in
[Known issues](https://github.com/swganh/mmoserver/blob/master/docs/KNOWN_ISSUES.md).
