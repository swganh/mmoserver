# Dependency Build Scripts

This directory contains scripts for building third-party libraries required by the
server. Place the source code for each dependency under `deps/src` and run
`build_deps.bat` from a Visual Studio Developer Command Prompt.

```
deps/
 ├─ build_deps.bat
 ├─ README.md
 └─ src/
    ├─ boost/
    ├─ mysql/
    ├─ mysqlpp/
    ├─ zlib/
    ├─ tbb/
    ├─ tolua/
    └─ gtest/
```

The script detects each subdirectory, generates Visual Studio 2022 projects with
CMake when required, bootstraps Boost if `b2.exe` is missing, and invokes
MSBuild to compile the libraries. Dependencies that are not present are skipped
with a message.
