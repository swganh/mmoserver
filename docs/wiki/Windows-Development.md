Windows is for developing the servers: building, running and debugging them
with Visual Studio. Running a community server is done on Linux
([[Linux Quick Start]]).

**Status:** the servers build with Visual Studio 2022 or later (MSVC and
vcpkg), pass the unit tests, and have been played with a client, with the
servers on Windows and the database on a Linux machine. Still to do: a local
MariaDB on the Windows PC, so no second machine is needed, and step-by-step
tutorials for working on the code in Visual Studio.

The full instructions are in
[docs/WINDOWS.md](https://github.com/swganh/mmoserver/blob/master/docs/WINDOWS.md).
In short:

1. Install Visual Studio 2022 or later with "Desktop development with C++",
   and Git for Windows.
2. `.\build_server.ps1 -Test -Heightmaps tatooine` builds everything (the
   first build compiles the libraries with vcpkg and takes about an hour).
3. On the Linux machine, let the PC use the database:
   `sudo ./mmoserverdb/setup.sh allow-host <PC address>` (see [[Database]]),
   and set `DBServer` and `DBPass` in
   `build\windows-debug\bin\config\general.cfg` on the PC.
4. `.\server.ps1 start tutorial tatooine` runs the servers.
