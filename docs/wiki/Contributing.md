Help is welcome: testing with a client, fixing bugs, writing content and
scripts, improving these pages.

## Reporting a problem

Open an issue on [GitHub](https://github.com/swganh/mmoserver/issues) with:

- what you did and what happened (and what you expected);
- the platform (Ubuntu version, MariaDB version) and the commit you built
  (`git log --oneline -1`);
- the end of the relevant server log from `build/linux-debug/bin/logs/`. For
  a crash, the backtrace from `./server.sh debug -g` ([[Troubleshooting on Linux]]).

Check [Known issues](https://github.com/swganh/mmoserver/blob/master/docs/KNOWN_ISSUES.md)
first: the problem may be known, with a workaround.

## Changing the code

1. Fork the repository on GitHub and clone your fork.
2. Create a branch for the change: `git checkout -b fix-something`.
3. Build and run the tests: `./build_server.sh -t`. All tests must pass.
4. Test the change with a client where it affects gameplay, and say how in
   the pull request.
5. Push the branch and open a pull request against `master`.

Every push runs CI: builds on Ubuntu 24.04 and 26.04 (system packages and
vcpkg), Windows (MSVC with vcpkg, and the Visual Studio solution), and a
database install on MariaDB 10.11 and 11.8. CI must pass,
and pull requests are reviewed before they are merged.

Keep changes focused: one fix or feature per pull request, and explain the
cause in the commit message, not only the change. When a change fixes or
finds a problem, update `docs/KNOWN_ISSUES.md`; when it is confirmed with a
client, `docs/TEST_PLAN.md`.

## Rules

- **Content comes from the client's own files and public material** (player
  guides, wikis, recordings from the time). The leaked SOE server source is
  never used, for reference or otherwise: it puts the project and everyone
  hosting it at legal risk.
- **The code is GPL**; contributions are under the same licence.
- **Lua stays at 5.1**, which the script bindings are written for.
- **The database is MariaDB** ([[Database]]).

## Where the project is going

The [Roadmap](https://github.com/swganh/mmoserver/blob/master/docs/ROADMAP.md)
has the phases: a reproducible build (Linux done, Windows in progress), a
runnable server (in progress), moving content into Lua scripts, then
content and pre-CU gameplay. Good first areas: testing with a client and
reporting results, the open items in Known issues, and the tutorial's Lua
script ([Scripting](https://github.com/swganh/mmoserver/blob/master/docs/SCRIPTING.md)).
