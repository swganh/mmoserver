Day-to-day use of a server installed with the [[Linux Quick Start]]. All
commands run in the `mmoserver` directory.

## Starting and stopping

```bash
./server.sh start                    # tutorial and tatooine
./server.sh start tutorial naboo     # or pick the zones
./server.sh status
./server.sh stop
./server.sh restart
```

`start` launches the Connection, Chat, Login and Ping servers, then the
zones, in that order (the others register with the ConnectionServer). Each
runs in the background, so you can log out of the machine. The tutorial and
Tatooine must both be running before the galaxy accepts logins.

Zones available: `tutorial`, `tatooine`, `naboo`, `corellia`, `dantooine`,
`dathomir`, `endor`, `lok`, `rori`, `talus`, `yavin4`. Every zone except the
tutorial needs its terrain: `./build_server.sh --heightmaps lok`. Each zone
uses 100-300 MB of memory, so start only the ones you need. Travel between
planets works when both zones are running.

## Logs

Each server writes to `build/linux-debug/bin/logs/<server>.out`
(`connection`, `login`, `chat`, `ping`, or the zone name). The previous run's
log is kept as `<server>.out.1`.

```bash
tail -f build/linux-debug/bin/logs/tatooine.out
```

`./server.sh status` shows each log's last line.

## Updating

```bash
./server.sh stop
git pull
./build_server.sh
sudo ./mmoserverdb/setup.sh update
./server.sh start
```

`update` loads changed stored procedures and applies new data changes
(migrations) to the database; accounts and characters are kept.

## Debugging

```bash
sudo apt install tmux gdb
./server.sh debug                    # each server in its own tmux window
./server.sh debug -g tutorial        # each under gdb
```

`debug` starts the same servers in a tmux session, one window per server
with its output live. Ctrl-b n / Ctrl-b p switch windows, Ctrl-b d detaches
(the servers keep running; `tmux attach -t mmoserver` comes back). With
`-g`, a server that crashes stops in gdb and prints the backtrace, which is
also saved in its log: attach that log when reporting the crash.

## Release build

`./build_server.sh -r` builds an optimised version into
`build/linux-release`; run it with `./server.sh -r start`. The debug build
is the default because it gives better crash reports while the project is
being fixed.

## Ports

| Port | Used by | Open to clients? |
|---|---|---|
| UDP 44990 | LoginServer | yes |
| UDP 44991 | ConnectionServer | yes |
| UDP 44992 | PingServer | yes |
| UDP 5000 | ConnectionServer, for the other servers | no |
| UDP 5001-5011, 5200 | zones and ChatServer | no |

```bash
sudo ufw allow 44990:44992/udp
```

Keep the others closed: the servers only use them among themselves.

## Watching memory

To keep an eye on the zones' memory over a long run:

```bash
nohup bash -c 'while true; do echo "$(date +%T) $(ps -o rss=,args= -C ZoneServer | tr "\n" " ")"; sleep 60; done' >> ~/zone-mem.log 2>&1 &
tail ~/zone-mem.log
```

Sizes are in KB. A zone stays at a steady size when idle; one that keeps
growing is worth reporting.
