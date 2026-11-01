# MMOServer #

MMOServer is a server emulator for [Star Wars Galaxies][1], aiming at the
Pre-Combat Upgrade (pre-CU) experience. It was originally developed by the
SWG:ANH team. The servers are written in C++ with Lua scripting.

The project is being revived. See [docs/CODE_REVIEW_2026.md](docs/CODE_REVIEW_2026.md)
for the current state of the code and the roadmap.

## Building on Linux ##

Linux is the primary platform. These steps are tested on Ubuntu 24.04 LTS and
26.04 LTS, and run in CI on every push.

### 1. Install the dependencies ###

Ubuntu 24.04:

    sudo apt install build-essential cmake ninja-build \
        libboost-all-dev libtbb-dev libmysqlcppconn-dev liblua5.1-0-dev \
        libtolua++5.1-dev libnoise-dev libglm-dev libgtest-dev \
        libmysqlclient-dev zlib1g-dev

Ubuntu 26.04 no longer has `libtolua++5.1-dev`. Leave it out; the build
downloads tolua++ (checksum-verified) and compiles it itself:

    sudo apt install build-essential cmake ninja-build \
        libboost-all-dev libtbb-dev libmysqlcppconn-dev liblua5.1-0-dev \
        libnoise-dev libglm-dev libgtest-dev \
        libmysqlclient-dev zlib1g-dev

The script engine needs Lua **5.1**; newer Lua versions do not work.

### 2. Build ###

    ./build_server.sh -t

This checks the prerequisites (and prints the `apt install` line for anything
missing), configures and builds with CMake presets, runs the unit tests (`-t`),
and stages `config/`, `script/` and `logs/` next to the servers in
`build/linux-debug/bin`. Run `./build_server.sh -h` for all options:

| Option | Effect |
|---|---|
| `-r` | Release build (`build/linux-release`, RelWithDebInfo) |
| `-c` | Delete the build directory first |
| `-t` | Run the unit tests |
| `--vcpkg` | Build the dependencies with vcpkg instead of using system packages |
| `--heightmaps LIST` | Download heightmaps: `tatooine,naboo` or `all` |

Existing files in `bin/config` are kept on rebuilds, so local settings such as
database credentials survive.

### Building with CMake directly ###

The script is a thin wrapper. Any of these work on their own:

    cmake -B build && cmake --build build && ctest --test-dir build

    cmake --workflow --preset linux-debug      # configure + build + test
    cmake --preset linux-release && cmake --build --preset linux-release

Presets: `linux-debug`, `linux-release`, `linux-vcpkg-debug`,
`linux-vcpkg-release`. Binaries end up in `<build dir>/bin`.

### Building with vcpkg ###

[vcpkg][2] builds every dependency from source, so it works on distributions
without the packages above and will be the route for Windows builds. Lua 5.1
and tolua++ are not in the vcpkg registry; they come from the overlay ports in
`tools/vcpkg/ports`.

    sudo apt install build-essential cmake ninja-build git curl zip unzip tar pkg-config libtirpc-dev
    ./build_server.sh --vcpkg -t

This uses `$VCPKG_ROOT` if it is set, and otherwise clones vcpkg into
`build/vcpkg`. The first build compiles Boost, MySQL Connector/C++ and the
rest, which takes a while.

### Heightmaps ###

The zone servers read terrain from `heightmaps/<planet>.hmpw`. The files are
large (150 to 230 MB to download and about 470 MB unpacked, per planet), so
they are not downloaded by default:

    ./build_server.sh --heightmaps tatooine
    tools/heightmaps/download.sh tatooine naboo   # or on its own; no arguments = all

The archives come from the [v0.6.0 release][3] and are checked against
`tools/heightmaps/SHA256SUMS` before they are unpacked into `data/heightmaps`.

## Running ##

### 1. Set up the database ###

The servers use MariaDB (tested with 10.11 and 11.8; MySQL 8 is not supported
yet). Install it and create the databases:

    sudo apt install mariadb-server
    sudo ./mmoserverdb/setup.sh install

This imports the schema and game data, removes the sample accounts, and
creates a `swganh` database user with a random password. The connection
settings go into `mmoserverdb/local.cfg` (not in git) and into
`bin/config/general.cfg` of every build; later builds pick them up
automatically.

If game clients connect from other machines, pass the address they should
use: `--public-address 192.168.1.10`. To change it later without
reinstalling, run `sudo ./mmoserverdb/setup.sh set-address 192.168.1.10` and
restart the servers. To start over, add `--force` to `install`. That drops
the databases, including all accounts and characters.

Create an account to log in with:

    sudo ./mmoserverdb/setup.sh add-account <name>

### 2. Start the servers ###

    ./server.sh start                    # tutorial and tatooine
    ./server.sh start tutorial naboo     # or pick the zones
    ./server.sh status
    ./server.sh stop

`start` launches the Connection, Chat, Login and Ping servers, then the
zones, each detached from the terminal and logging to
`build/linux-debug/bin/logs/<name>.out`. Zones other than tutorial need their
heightmap (`./build_server.sh --heightmaps tatooine`) and take a minute or
two to load. Add `-r` to run the release build.

For debugging, `debug` starts the same servers in a tmux session
(`sudo apt install tmux gdb`), one window per server showing its output live:

    ./server.sh debug                    # tutorial and tatooine
    ./server.sh debug -g tutorial        # each server under gdb

Switch windows with Ctrl-b n / Ctrl-b p (window 0 shows the status), detach
with Ctrl-b d and come back with `tmux attach -t mmoserver`. A server that
exits or crashes keeps its window open. With `-g`, a crash stops in gdb and
prints the backtrace, which is also saved in the server's log. Type `q` and
Enter in a server's window to quit it; `./server.sh stop` stops them all and
closes the session.

Game clients log in on UDP port 44990; the ConnectionServer uses 44991 and
the PingServer 44992. If the firewall is on (`sudo ufw status`), open them
for clients on other machines:

    sudo ufw allow 44990:44992/udp

Keep the internal ports (5000 and the zones' 50xx) closed to the outside;
the servers only use them among themselves.

### 3. Connect a client ###

Point the client's `login.cfg` at the server and log in with an account
created by `setup.sh add-account`:

    [ClientGame]
    loginServerAddress0=192.168.1.10
    loginServerPort0=44990

If the client gets no response, run `sudo tcpdump -ni any udp port 44990`
on the server while logging in. No packets means a network problem between
the machines; packets arriving with no reply usually means the firewall.

## Windows ##

Windows support will move to the same CMake presets and vcpkg manifest. Until
then, `build_server.bat` is known to be broken (see section 2 of the code
review).

  [1]: https://en.wikipedia.org/wiki/Star_Wars_Galaxies
  [2]: https://vcpkg.io
  [3]: https://github.com/swganh/mmoserver/releases/tag/v0.6.0
