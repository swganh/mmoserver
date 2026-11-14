From a fresh Ubuntu machine to logged in with a game client. Takes about
half an hour, most of it the first build.

## What you need

- **Ubuntu 26.04 or 24.04 LTS**, on a spare PC or a virtual machine
  (VirtualBox, Hyper-V, Proxmox...). Other distributions work with the
  equivalent packages, or with the vcpkg build in the repository README.
- **2 or more CPU cores, 4 GB of RAM** (8 GB is comfortable), **20 GB of
  disk**. Each zone uses 100-300 MB of memory; the build needs more than
  running does.
- A **fixed address** for the machine on your network (a static IP or a DHCP
  reservation in your router), so game clients can always find it.
- A **game client**: see [[Game Client]].

The examples use `192.168.1.10` for the server; use your own address
(`ip -4 addr` shows it).

## 1. Get the code

```bash
sudo apt install git
git clone https://github.com/swganh/mmoserver.git
cd mmoserver
```

## 2. Install the build tools and libraries

Ubuntu 26.04:

```bash
sudo apt install build-essential cmake ninja-build \
    libboost-all-dev libtbb-dev libmysqlcppconn-dev libsodium-dev liblua5.1-0-dev \
    libnoise-dev libglm-dev libgtest-dev libmysqlclient-dev zlib1g-dev
```

Ubuntu 24.04: the same line plus `libtolua++5.1-dev`. (26.04 no longer has
that package; the build downloads tolua++ and compiles it itself.)

If you miss a package, `build_server.sh` says which and prints the
`apt install` line for it.

## 3. Build

```bash
./build_server.sh -t --heightmaps tatooine
```

This builds the servers into `build/linux-debug/bin`, runs the unit tests
(`-t`) and downloads the Tatooine terrain (about 200 MB, checked against a
checksum). The tutorial needs no terrain; every other planet does
(`--heightmaps tatooine,naboo` or `--heightmaps all`).

## 4. Install the database

```bash
sudo apt install mariadb-server
sudo ./mmoserverdb/setup.sh install --public-address 192.168.1.10
```

`--public-address` is the address game clients use to reach this server;
leave it out if the client runs on the same machine. The installer imports
the game data, spawns harvestable resources, and creates a database user
with a random password. The servers pick up those settings automatically.
More in [[Database]].

## 5. Create an account

```bash
sudo ./mmoserverdb/setup.sh add-account myname
```

It asks for a password (8 characters or more).

## 6. Start the servers

```bash
./server.sh start
./server.sh status
```

This starts the Connection, Chat, Login and Ping servers and two zones,
tutorial and Tatooine. Zones take a minute or two to load; the galaxy shows
as "Loading..." in the client until both are up.

## 7. Open the firewall

Only needed if the firewall is on (`sudo ufw status`) and the client runs on
another machine:

```bash
sudo ufw allow 44990:44992/udp
```

## 8. Log in

Point the client at the server (see [[Game Client]]) and log in with the
account from step 5. New characters start in the tutorial, or can skip it
and travel straight to Tatooine.

Next: [[Running the Servers on Linux]] for day-to-day use, updating and
debugging. If something does not work: [[Troubleshooting on Linux]].
