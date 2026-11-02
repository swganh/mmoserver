# Windows development environment

Build, run and debug the servers on Windows with Visual Studio, against the
database on your Linux server (or VM). Running a community server is done on
Linux (README); Windows is for development.

## What you need

- Windows 10 or 11, 64-bit.
- **Visual Studio 2022 or later** (Community is fine) with the **Desktop
  development with C++** workload. It includes the MSVC compiler, CMake,
  Ninja and vcpkg.
- **Git for Windows**.
- About 20 GB free: vcpkg builds Boost, MySQL Connector/C++ and the other
  dependencies from source the first time (30-60 minutes); later builds
  reuse them.
- The database installed on the Linux machine (`mmoserverdb/setup.sh
  install`, README).

## Build

From PowerShell, in the repository:

```powershell
.\build_server.ps1 -Test -Heightmaps tatooine
```

This finds Visual Studio, builds the dependencies with vcpkg, builds the
servers (`windows-debug` preset), runs the unit tests and downloads the
Tatooine heightmap. Options: `-Release` (the `windows-release` preset),
`-Clean`, `-Heightmaps tatooine,lok` or `-Heightmaps all`. From `cmd.exe`,
`build_server.bat` takes the same options. If PowerShell refuses to run the
script, run it as `powershell -ExecutionPolicy Bypass -File .\build_server.ps1`.

The servers and everything they load end up in `build\windows-debug\bin`:
`config\`, `script\`, `logs\` and a link to `data\heightmaps`. Every build
refreshes `script\` and adds missing configs, and never overwrites a config
you edited.

**In Visual Studio:** *File > Open > Folder* on the repository. Visual Studio
reads `CMakePresets.json`; pick **Windows Debug (MSVC, vcpkg)** in the
configuration list on the toolbar, then *Build > Build All*. The first
configure runs vcpkg, which takes as long as above.

**A Visual Studio solution:** to work with a `.sln` and projects instead,

```powershell
.\build_server.ps1 -Solution
```

generates `build\windows-vs\mmoserver.sln` (`.slnx` with Visual Studio
2026; one project per server and library) and builds its Debug configuration; open the solution in Visual
Studio afterwards. The servers and their `config\`, `script\` and `logs\`
are in `build\windows-vs\bin\Debug`. ZoneServer is the startup project and
F5 starts the tutorial zone there; change the zone in the project's
*Properties > Debugging > Command Arguments*. Use `.\server.ps1 -Solution
...` to start the other servers from this build. Regenerate after pulling
changes that add or remove source files.

**Visual Studio picks the compiler:** `build_server.ps1` builds with the
newest MSVC in the newest Visual Studio, the one vcpkg builds the libraries
with, even when a developer prompt is set to an older one. Mixing them fails
to link (see Troubleshooting).

## Connect to the database

The servers need the MariaDB database on the Linux machine. Two ways:

### On a trusted home network: allow the Windows PC

On the Linux machine, with your Windows PC's address:

```bash
sudo ./mmoserverdb/setup.sh allow-host 192.168.1.128
```

This lets the `swganh` database user connect from that address only, with the
same password as the local servers, and prints the rest: MariaDB listens only
on the Linux machine by default, so add its LAN address to `bind-address`
while keeping `127.0.0.1` for the local servers
(`bind-address = 127.0.0.1,192.168.1.239`), and with `ufw` enabled the port
has to be opened to the PC. `deny-host` undoes it. Never open port 3306
to the internet.

### Anywhere else: an SSH tunnel

Nothing changes on the Linux machine. On Windows (OpenSSH is built in), keep
this running while you work:

```powershell
ssh -N -L 3307:127.0.0.1:3306 swganh@192.168.1.239
```

The database then appears on the Windows PC at `127.0.0.1` port `3307`, and
the Linux machine sees the connections as local ones.

### Settings

Edit `build\windows-debug\bin\config\general.cfg`:

```ini
DBServer = 192.168.1.239    # or 127.0.0.1 with the tunnel
DBPort = 3306               # or 3307 with the tunnel
DBUser = swganh
DBPass = <DBPass from mmoserverdb/local.cfg on the Linux machine>
```

## Run

One database is one galaxy, and only one set of servers should run against
it. Before starting the servers on Windows:

1. Stop the servers on the Linux machine: `./server.sh stop`.
2. Point the galaxy at the Windows PC, so the login server sends clients
   there: `sudo ./mmoserverdb/setup.sh set-address 192.168.1.128` (or
   `127.0.0.1` when the game client runs on the same PC). Set it back to the
   Linux machine's address before starting its servers again.

Then, in the repository:

```powershell
.\server.ps1 start tutorial tatooine
.\server.ps1 status
.\server.ps1 stop
```

Output goes to `build\windows-debug\bin\logs\<server>.out` (and `.out.err`).
With `-Console`, each server opens its own console window instead. The first
time, Windows Defender Firewall asks whether to allow the servers: allow
them on private networks (game clients use UDP 44990-44992).

`stop` ends the processes immediately; zones save players periodically, so
the last minutes of play may not be saved.

## Debug a server

Start the servers you are not debugging with the script, then run the one
you are from Visual Studio. For example, to debug the Tatooine zone:

```powershell
.\server.ps1 start tutorial
```

In Visual Studio, pick **ZoneServer.exe** as the startup item, then *Debug >
Debug and Launch Settings for ZoneServer* and set the working directory and
arguments:

```json
{
  "type": "default",
  "project": "CMakeLists.txt",
  "projectTarget": "ZoneServer.exe",
  "name": "ZoneServer (tatooine)",
  "currentDir": "${workspaceRoot}\\build\\windows-debug\\bin",
  "args": [ "--ZoneName", "tatooine" ]
}
```

Press F5. Visual Studio keeps this in `.vs\launch.vs.json`, which is not in
version control.

## Troubleshooting

**`unresolved external symbol __std_search_1`** (or `__std_find_end_1`,
`_Thrd_sleep_for`) when linking: the vcpkg libraries were built by a newer
MSVC than the one compiling the servers. Visual Studio can hold several MSVC
versions side by side, and a developer prompt may be set to an older one.
`build_server.ps1` now always builds with the newest one, as vcpkg does, and
reconfigures a build directory made with another; run it again (with
`-Clean` if in doubt). When building from Visual Studio itself, make sure the
newest "MSVC v143 build tools (Latest)" component is the default toolset.

**`-Solution` fails to link with the same errors** when an older MSVC is
installed side by side and is Visual Studio's default toolset: MSBuild uses
the default rather than the newest. Not solved yet; use the Ninja build
(`build_server.ps1` without `-Solution`), or remove the older "MSVC v143
build tools (v14.xx)" component in the Visual Studio Installer if nothing
else needs it.
