@echo off
setlocal

rem Detect Visual Studio 2022 tools
if not defined VS170COMNTOOLS (
    set "VS170COMNTOOLS=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools"
)

rem Path to MSBuild
set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"

rem Build settings
set "ARCH=x64"
set "DEPS_BASE=%~dp0"
set "DEPS_SRC=%DEPS_BASE%src"

rem Initialize developer command prompt
call "%VS170COMNTOOLS%\VsDevCmd.bat" -arch=%ARCH%

rem --- Boost ---
if exist "%DEPS_SRC%\boost" (
    pushd "%DEPS_SRC%\boost"
    if not exist b2.exe (
        if exist bootstrap.bat (
            echo Bootstrapping Boost...
            call bootstrap.bat >nul
        )
    )
    if exist b2.exe (
        b2 --with-log --with-program_options --with-date_time --with-thread --with-regex --with-system ^
           variant=debug,release link=static runtime-link=shared threading=multi ^
           define=_SCL_SECURE_NO_WARNINGS=0 toolset=msvc-14.3
    ) else (
        echo Boost build tool not found. Skipping Boost build.
    )
    popd
) else (
    echo Boost directory not found. Skipping build.
)

rem --- MySQL ---
if exist "%DEPS_SRC%\mysql" (
    pushd "%DEPS_SRC%\mysql"
    if exist build rmdir /s /q build
    cmake -S . -B build -G "Visual Studio 17 2022" -A %ARCH%
    "%MSBUILD%" build\ALL_BUILD.vcxproj /p:Configuration=Release
    popd
) else (
    echo MySQL directory not found. Skipping build.
)

rem --- MySQL++ ---
if exist "%DEPS_SRC%\mysqlpp" (
    pushd "%DEPS_SRC%\mysqlpp"
    if exist build rmdir /s /q build
    cmake -S . -B build -G "Visual Studio 17 2022" -A %ARCH%
    "%MSBUILD%" build\mysqlpp.sln /p:Configuration=Release
    popd
) else (
    echo MySQL++ directory not found. Skipping build.
)

rem --- zlib ---
if exist "%DEPS_SRC%\zlib" (
    pushd "%DEPS_SRC%\zlib"
    if exist build rmdir /s /q build
    cmake -S . -B build -G "Visual Studio 17 2022" -A %ARCH%
    "%MSBUILD%" build\zlib.sln /p:Configuration=Release
    popd
) else (
    echo zlib directory not found. Skipping build.
)

rem --- TBB ---
if exist "%DEPS_SRC%\tbb" (
    pushd "%DEPS_SRC%\tbb"
    if exist build rmdir /s /q build
    cmake -S . -B build -G "Visual Studio 17 2022" -A %ARCH%
    "%MSBUILD%" build\tbb.sln /p:Configuration=Release
    popd
) else (
    echo TBB directory not found. Skipping build.
)

rem --- Tolua++ ---
if exist "%DEPS_SRC%\tolua" (
    pushd "%DEPS_SRC%\tolua"
    if exist build rmdir /s /q build
    cmake -S . -B build -G "Visual Studio 17 2022" -A %ARCH%
    if exist build\tolua.sln (
        "%MSBUILD%" build\tolua.sln /p:Configuration=Release
    ) else if exist build\ALL_BUILD.vcxproj (
        "%MSBUILD%" build\ALL_BUILD.vcxproj /p:Configuration=Release
    ) else (
        echo Tolua++ solution not found after CMake generation.
    )
    popd
) else (
    echo Tolua++ directory not found. Skipping build.
)

rem --- GTest ---
if exist "%DEPS_SRC%\gtest" (
    pushd "%DEPS_SRC%\gtest"
    if exist build rmdir /s /q build
    cmake -S . -B build -G "Visual Studio 17 2022" -A %ARCH%
    "%MSBUILD%" build\gtest.sln /p:Configuration=Release
    popd
) else (
    echo GTest directory not found. Skipping build.
)

endlocal
