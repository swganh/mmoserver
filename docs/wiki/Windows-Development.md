Windows is for developing the servers: building, running and debugging them
with Visual Studio. Running a community server is done on Linux
([[Linux Quick Start]]).

**Status: in progress.** The servers build with Visual Studio 2022 (MSVC and
vcpkg) and have been played with a client, with the servers on Windows and
the database on a Linux machine. The work is on the `windows-dev-env` branch
and will move to `master` when it is ready; this page will then have a full
step-by-step guide.

Until then, the current instructions are in
[docs/WINDOWS.md](https://github.com/obi-two/mmoserver/blob/windows-dev-env/docs/WINDOWS.md)
on that branch. In short:

1. Visual Studio 2022 or later with "Desktop development with C++", and Git
   for Windows.
2. `.\build_server.ps1 -Test -Heightmaps tatooine` builds everything.
3. `.\server.ps1 start tutorial tatooine` runs the servers.

Still to do: a local MariaDB on the Windows PC, so no second machine is
needed, and beginner tutorials for working on the code in Visual Studio.
