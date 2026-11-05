# MMOServer Code Review & Revival Plan (October 2026)

A full review of the repository as of `184b326`. The goal is a pre-CU (Pre-Combat
Upgrade, ~2003–2005) Star Wars Galaxies emulator that the community can build and
run, and that developers can work on with a modern toolchain.

Everything marked **verified** was reproduced. Ubuntu 24.04 was used: GCC 13,
CMake 3.28, Boost 1.83, oneTBB 2021.11, MySQL Connector/C++ 1.1 (legacy API),
Lua 5.1.5, tolua++ 1.0.93, libnoise 1.0, GLM 0.9.9 and GoogleTest 1.14.
No Windows machine was available, so findings about the `.bat` scripts come
from reading them, not from running them.

---

## 1. Executive summary

| Area | State |
|---|---|
| Windows first build (`build_server.bat`) | **Broken: cannot succeed on a clean machine** (section 2) |
| Dependency package | Exists, but at a different URL than the scripts use. It holds 2014-era Boost 1.58, TBB 4.x, GLM 0.9.0 and VS2010/2013 Win32-only project files |
| Source on a modern compiler | **Good news: verified.** All 5 servers compile and link with GCC 13 after 6 small source fixes (in this branch) plus CMake work-arounds |
| Unit tests | 127 tests. Before the fix, 21 hung or crashed. **All 127 pass after the `ActiveObject` fix** |
| Critical runtime bug | `utils::ActiveObject` silently dropped queued work on current Boost (fixed here, section 3.1) |
| Security | Pre-auth remote stack overflow in LoginServer. Unbounded packet reads everywhere. Plaintext passwords (section 4) |
| Pre-CU gameplay coverage | Strong on the "world": travel, crafting, harvesting, structures, entertainers, missions, chat. Thin on combat (~1.5k lines). **Absent:** factions/GCW, Jedi, loot, guilds (section 5) |

The codebase is worth reviving. About 185k lines of C++ represent a lot of
protocol and gameplay knowledge, and it is closer to building than its build
scripts suggest. The build scripts are the main thing standing between the
community and a running server.

---

## 2. Phase one: `build_server.bat` and the dependency pipeline

### 2.1 Blocking defects (a clean build fails at each one)

| # | Where | Problem |
|---|---|---|
| B1 | `build_server.bat:86`, `:258`; `build_server.sh` | Every download URL (`obi-two/Unofficial_Hope/.../Downloads/...`) returns **404**. The files actually live on this repo's own release: `https://github.com/swganh/mmoserver/releases/download/v0.6.0/`. That release has `mmoserver-deps-0.6.0.tar.bz2`, `tatooine.hmpw.zip`, and the other planets named `<planet>.zip` (**not** `<planet>.hmpw.zip`). |
| B2 | `build_server.bat:207` | Calls `vsvars32.bat`, which VS2017 and later do not ship. VS2022 uses `VsDevCmd.bat`. The failure is ignored and `environment_built` is set anyway. |
| B3 | `build_server.bat:205` | `MSBUILD` points at the .NET Framework 4.0 MSBuild. It cannot build v143 (VS2022) C++ projects. VS2022 ships its own MSBuild, or use `cmake --build`. |
| B4 | `build_server.bat:193-195` | Only finds the **Community** edition, at a hard-coded path. Professional, Enterprise, Build Tools and custom install locations all fail. Use `vswhere.exe` instead. |
| B5 | `build_server.bat:32`, `:278-280`, `:180` | `deps/` is now **committed** (`README.md`, `build_deps.bat`). So `if not exist "deps"` never fires. Then `deps\VERSION` is missing, which triggers `CLEAN_BUILD`, which `rmdir /S /Q deps`: it **deletes tracked files** and replaces them with the tarball's 2014 `build_deps.bat`. |
| B6 | `deps/build_deps.bat` (committed) | Written for a layout that doesn't exist. It looks for `src\mysql`, `src\mysqlpp` (MySQL++ is not used by this project), `src\tolua` and `src\gtest`. The tarball contains `mysql-connector-c`, `mysql-connector-cpp`, `tolua++`, `lua`, `glm` and `noise`. Lua, libnoise and Connector/C++ are never built. |
| B7 | `CMakeLists.txt:190` | Requires **Boost ≥ 1.70**, but the dependency tarball ships **Boost 1.58**. Even with B1–B6 fixed, configure fails. |
| B8 | Tarball contents | TBB 4.x (`build\vc12`), libnoise (`win32\*.sln`) and tolua++ (`win32\vc10`) ship only VS2010/2013 **Win32** solutions. The build was switched to x64 in `7a1f38b`, but nothing builds x64 versions of these libraries. |
| B9 | `tools/windows/postbuild.bat:20-31` | Copies TBB DLLs from `vc17\ia32` (32-bit) into an x64 build. The DLLs are missing, so the servers won't start. |
| B10 | Unit tests (`RUN_TESTS`, `build_server.bat:348`) | On any modern Boost, `Utils_tests`/`Common_tests` **hang forever** because of the `ActiveObject` bug (section 3.1). The build script runs these tests, so the build never finishes. **Fixed in this branch.** |

### 2.2 Non-blocking defects

- `:113` `-h` prints help but **doesn't exit**, so a full build starts.
- `:96` `BUILDNUMBER` defaults to `0`, so the `!= "false"` check is always true and `0` is appended to every `VERSION` file.
- `:92` `MSVC_VERSION=12` is dead.
- `:289` `if not %current_version% == ...` is a syntax error when `VERSION` is empty.
- `:341` `CMAKE_INSTALL_PREFIX=%PROJECT_BASE%` is unquoted and ends in a backslash. It breaks on paths with spaces.
- No error checks after downloading or building dependencies: failures cascade into confusing CMake errors.
- `--no-check-certificate` on downloads, with no checksum verification.
- Relies on `wget`, which isn't in `tools/windows`. Windows 10+ ships `curl.exe` and `tar.exe`.
- `tools/windows/` and `src/ScriptEngine/glue_files/` commit **unsigned third-party executables** (`7z.exe`, `tar.exe`, `bzip2.exe`, `msys-1.0.dll`, `gitversion.exe`, `tolua++.exe`). This is a supply-chain risk, and none of them are needed on Windows 10+.

### 2.3 CMake defects (affect every platform)

- `CMakeLists.txt:163`: if a `deps/` directory exists, every `*_ROOT` points into it. Because `deps/` is committed, system and vcpkg packages are **never found**. (Verified: configure fails on TBB.)
- The custom `tools/cmake/Find*.cmake` modules report "found" even when include paths are `NOTFOUND`. The errors only appear later, with an unhelpful message.
- `FindTBB.cmake` can't find oneTBB. The custom FindBoost-era variables (`Boost_*_LIBRARY_DEBUG`) are empty with system Boost, so test executables fail to link.
- Static libraries are linked in the wrong order (`Utils`, `Common` and `MessageLib` come before the libraries that use them). This works under MSVC but fails with GNU `ld`.
- `:48` appends `-std=c++0x` on GCC, which contradicts `CMAKE_CXX_STANDARD 17`.
- `cmake_minimum_required(3.2)` and the `FindBoost` module are deprecated or removed (CMP0167 in CMake 3.30+).

### 2.4 Recommended direction for phase one

**Replace the dependency tarball with [vcpkg](https://vcpkg.io) in manifest mode.**
One `vcpkg.json` then drives Windows (MSVC x64), Linux and macOS identically, and a
CMake preset replaces most of `build_server.bat`.

| Dependency | vcpkg port | Notes |
|---|---|---|
| Boost (thread, log, regex, date_time, program_options, system, pool, asio) | `boost-*` 1.9x | Code compiles against 1.83 (verified) |
| TBB | `tbb` | Only `concurrent_queue` is used now. The `tbb::atomic` usage was ported to `std::atomic` in this branch |
| MySQL Connector/C++ | `mysql-connector-cpp[jdbc]` | The `jdbc` feature is the legacy 1.1 API this code uses |
| GLM, GoogleTest, zlib, libnoise | `glm`, `gtest`, `zlib`, `libnoise` | Available as-is |
| **Lua 5.1** | — | vcpkg only has 5.4/5.5. The code is tied to 5.1 (`LUA_GLOBALSINDEX`, `luaL_register`) |
| **tolua++** | — | Not in vcpkg |

Lua 5.1.5 and tolua++ are small C libraries. Ship them as vcpkg **overlay ports**
in the repo (or vendor them as in-tree CMake targets). That also removes the
committed `tolua++.exe`.

The rewritten `build_server.bat` then only needs to:

1. Locate VS2022 via `vswhere` and call `VsDevCmd.bat -arch=x64`.
2. Bootstrap vcpkg (as a git submodule or a pinned clone).
3. Run `cmake --preset windows-x64` and `cmake --build --preset ... --config Debug|Release`.
4. Run `ctest`.
5. Optionally download heightmaps with `curl.exe` from the correct URLs, verifying SHA-256.
6. Stage configs, scripts and DLLs (vcpkg copies the DLLs automatically). This replaces `postbuild.bat`.

Add a **GitHub Actions** workflow that runs the same presets on `windows-2022` and
`ubuntu-24.04`, so the "first build" can't silently rot again.

The alternative is to keep and repair the 2014 tarball. That means hand-porting the
VS2010/2013 Win32 solutions for TBB 4, libnoise and tolua++ to x64/v143, and
building Boost 1.58 with a toolset it predates. It also keeps the project pinned to
decade-old libraries. **Not recommended.**

---

## 3. Correctness bugs

### 3.1 `utils::ActiveObject` drops messages: **critical, verified, fixed in this branch**

`src/Utils/ActiveObject.cpp` popped from the queue **inside the condition-variable
wait predicate**. Boost may evaluate a predicate more than once per wakeup, and on
current Boost it does. Each extra evaluation popped another message into the same
variable, overwriting the previous one before it ran.

A standalone reproduction posted 5 messages: 5 were popped, **2 executed**. The
destructor's shutdown message was among those lost, so `join()` hung forever.
`Send()` also notified without holding the mutex, which risks a lost wakeup. The
1 ms polling timeout that masked this costs about 1,000 wakeups per second per
object.

`ActiveObject` backs `SocketWriteThread` (outgoing packets), `DatabaseWorkerThread`
(every DB job), `SpatialIndexManager`, `EventDispatcher` and `ApplicationService`.
On a modern build the server would randomly drop packets, DB writes and events.

The fix pushes under the lock and uses a side-effect-free `!empty()` predicate. It
pops and runs the message outside the lock, so a message can call `Send()` itself,
and it drops the polling. Result: 127/127 tests pass, stable across repeated runs.

Also in this file: on Windows, `SetPriorityClass(thread_handle, REALTIME_PRIORITY_CLASS)`
passes a *thread* handle to a *process* API. It fails, which is lucky: realtime
priority on busy-waiting server threads can starve the OS. Remove it.

### 3.2 Other fixes in this branch

| File | Fix |
|---|---|
| `Utils/logger.cpp` | `boost/log/utility/empty_deleter.hpp` was removed from Boost. Now uses `boost/core/null_deleter.hpp` (Boost ≥ 1.56, so it works with the old tarball too) |
| `Utils/ConcurrentQueue.h` | `tbb::atomic`/`fetch_and_store` (removed in oneTBB) → `std::atomic`/`exchange` |
| `ZoneServer/ResourceManager.h` | Missing `#include <string>` |
| `ZoneServer/MissionManager.cpp` | `glm::gtx::random::vecRand3` exists only in GLM 0.9.0, and `gtc/random` only in newer GLM. Replaced with a small helper using the existing `gRandom`, so both GLM versions work. Destinations are now always 50–500 m away horizontally. Previously, zeroing `y` after a 3D random pick could put them closer than 50 m. |
| `ZoneServer/HarvesterObject.cpp` | `if(result > 0)` (a pointer compared with 0, so always true) was meant to be `if(error > 0)`. **Discarding resources from a harvester always failed.** Also fixed `asyncContainer->command = asyncContainer->command` (self-assignment, so the command bytes were lost) and a leaked `DataBinding`. |
| `LoginServer/LoginManager.cpp` | A failed login logged the plaintext password right after a `********` placeholder. It now logs only the username. |

### 3.3 Further correctness work (not changed here)

- Compiling surfaced **454 warnings** with GCC 13. Many are real, like the pointer comparison above. Phase two should build with `-Wall -Wextra` / `/W4`, fix the warnings, and remove the blanket `/wd` suppressions in `CMakeLists.txt`.
- `Message` read helpers use unaligned type-punned loads (`*(uint32*)&mData[i]`). This is undefined behaviour; use `memcpy`.
- About 47 grep matches (some may be in comments) for APIs deprecated or removed in C++17 (`std::auto_ptr`, `bind1st`/`bind2nd`, `register`, `throw()`, `random_shuffle`).

---

## 4. Security

Anyone running a public server needs these fixed first.

1. **Pre-authentication remote stack overflow in LoginServer.**
   `NetworkManager/Message.h`: none of the `get*()` readers check against
   `mSize`. A client chooses a string length of up to 65,535 bytes.
   `LoginManager.cpp:260-301` then escapes the client's username and password into
   `int8 sql[512]`. Escaping can double the length, with no bound. The same
   pattern appears with `name[50]` in `EntertainerManager.cpp:355`,
   `TreasuryManager.cpp:365`, and `name[40]` in `ChatServer/TradeManagerChat.cpp:986`.
   **Fix:** give `Message` bounds-checked reads that fail the packet, and replace
   fixed `int8[]` SQL buffers with `std::string` plus prepared statements.
2. **SQL built with `sprintf`.** There are about 131 call sites. Most escape their
   input, but not consistently, and escaping into fixed buffers is the overflow
   above. Move to Connector/C++ prepared statements incrementally, starting with
   anything that takes player-supplied text.
3. **`LIKE` with player input.** `TreasuryManager.cpp:369` (bank transfer by name)
   uses `firstname like '<name>'`. A name of `%` matches an arbitrary character,
   so credits go to the wrong player. Use `=`.
4. **Plaintext passwords.** `sp_ReturnUserAccount` compares the password in SQL.
   Store an Argon2id/bcrypt hash and verify it in the LoginServer. Use a
   migration that re-hashes on next successful login.
5. **Default credentials.** Every config ships with DB user/password `swganh`.
   The installer should generate per-install secrets.

---

## 5. Pre-CU feature coverage (ZoneServer, ~125k lines)

Approximate, from module size and a read of the managers:

| System | Present | Notes |
|---|---|---|
| Character creation, tutorial, travel/shuttles, badges, waypoints | ✅ | Reasonable |
| Resources, surveying, sampling, harvesters | ✅ | Core loop exists |
| Crafting, schematics, experimentation | ✅ | Largest gameplay area (~6.7k lines) |
| Entertainer, image design, buffs | ✅ | ~5k lines in the entertainer manager |
| Structures, housing, permissions | ✅ | ~6.4k lines |
| Missions (destroy/deliver/etc.) | ◐ | Terminals and generation exist |
| Medic/healing, camps | ◐ | |
| Creatures/NPC AI, spawning | ◐ | AI framework exists, but spawn tables are thin |
| Groups, trade, bank, chat, mail | ◐ | Chat and mail live in ChatServer (~15k lines) |
| **Combat** | ⚠️ thin | `CombatManager.cpp` is ~1.5k lines, with TODOs in damage and weapon data. Pre-CU combat (HAM pools, wounds/battle fatigue, posture, states, DoTs, specials, armor/encumbrance, accuracy/defense mods) is the heart of the era and mostly needs building |
| **Factions / GCW** | ❌ | No overt/covert/TEF or faction rank logic (`CombatManager.cpp:207` TODO) |
| **Jedi** (pre-CU hologrinder unlock, Force powers) | ❌ | A few Force-related files, no system |
| **Loot** | ❌ | |
| **Guilds** | ❌ | |
| Bazaar / vendors | ⚠️ stub | `Bazaar*` ~100 lines |
| Vehicles, mounts, pets | ⚠️ stub | |
| Script engine | Lua 5.1 + tolua++ | Fine for content, but glue generation needs the committed `tolua++.exe` |

---

## 6. Suggested roadmap

**Phase 1: reproducible first build (this effort)**
1. Land the source fixes in this branch.
2. Add `vcpkg.json` and overlay ports for Lua 5.1 and tolua++. Replace the custom
   `Find*.cmake` modules with imported targets, and fix link ordering.
3. Add `CMakePresets.json` (windows-x64 and linux presets).
4. Rewrite `build_server.bat` as a thin wrapper (section 2.4). Delete
   `deps/build_deps.bat` and `postbuild.bat`, and replace `build_server.sh` the
   same way.
5. Fix heightmap downloads (correct URLs and names, `curl.exe`, SHA-256).
6. Add a GitHub Actions CI matrix covering Windows and Linux builds plus `ctest`.
7. Update the README: it still says VS2013, `bootstrap.sh`, and points at dead
   swganh.com links.

**Phase 2: runnable server for the community**
- Database installer for MySQL 8 / MariaDB that is non-interactive and
  scriptable. Fold `mmoserverdb/setup.bat` into the build or a `docker compose`
  stack.
- One-command local launch, replacing `ServerStart.bat`'s `TIMEOUT` chain.
- Security items 4.1–4.5.

**Phase 3: modernisation**
- Warnings to zero, sanitizers (ASan/UBSan) in CI, and a fuzz harness for `Message` parsing.
- Replace `boost::thread`/`mutex`/`future` with `std::` equivalents.
- Prepared statements throughout.

**Phase 4: pre-CU gameplay**
- Combat, then factions/GCW, loot, then Jedi. Use the existing crafting and
  entertainer systems as the model for how a "complete" system looks in this
  codebase.
