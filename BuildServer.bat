@echo off
SETLOCAL EnableDelayedExpansion

rem BuildServer.bat
rem
rem This is the build script used for building SWGANH and it's dependencies
rem using Microsoft tools.

rem ----------------------------------------------------------------------------
rem ---Start of Main Execution -------------------------------------------------

rem Initialize environment variable defaults
call :SET_DEFAULTS

rem Process command line arguments first
goto :PROCESS_ARGUMENTS
:CONTINUE_FROM_PROCESS_ARGUMENTS

rem Build the environment and bail out if it fails
call :BUILD_ENVIRONMENT
if x%environment_built% == x goto :eof

cd %PROJECT_BASE%

if %SKIPHEIGHTMAPS% == false (
call :DOWNLOAD_DATA_FILES
)

call :BUILD_DEPENDENCIES

if not exist "deps" (
	echo Missing SWGANH dependencies!
	echo.
	echo Download the dependencies from %DEPENDENCIES_URL% and unpack it
	echo into the root project directory, then run this script again.
	exit /b 1
)

if %DEPENDENCIESONLY% == true goto :eof

call :BUILD_PROJECT

if not %ERRORLEVEL% == 0 (
if %HALT_ON_ERROR% == true (set /p halt=*** BUILD FAILED... PRESS ENTER TO CONTINUE ***)
exit /b %ERRORLEVEL%
)

if "%DBINSTALL%" == "true" (
	call :INITIALIZE_DATABASE
	if not %ERRORLEVEL% == 0 (
		if %HALT_ON_ERROR% == true (set /p halt=*** BUILD FAILED... PRESS ENTER TO CONTINUE ***)
		exit /b %ERRORLEVEL%
	)
)


if not "%BUILDNUMBER%" == "false" (
	if "%BUILD_TYPE%" == "debug" (
		echo %BUILDNUMBER% >> bin\Debug\VERSION
	)

	if "%BUILD_TYPE%" == "release" (
		echo %BUILDNUMBER% >> bin\Release\VERSION
	)

	if "%BUILD_TYPE%" == "all" (
		echo %BUILDNUMBER% >> bin\Debug\VERSION
		echo %BUILDNUMBER% >> bin\Release\VERSION
	)
)

echo.
echo Server Successfully Built^^!
echo This window will close shortly.
call :SLEEP 10

goto :eof
rem --- End of Main Execution --------------------------------------------------
rem ----------------------------------------------------------------------------



rem ----------------------------------------------------------------------------
rem ----------------------------------------------------------------------------
rem --- Helper Functions -------------------------------------------------------
rem ----------------------------------------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of SET_DEFAULTS --------------------------------------------------
:SET_DEFAULTS

set DEPENDENCIES_VERSION=0.1.2
set DEPENDENCIES_FILE=mmoserver-deps-%DEPENDENCIES_VERSION%.zip
set DEPENDENCIES_URL=http://github.com/downloads/swganh/mmoserver/%DEPENDENCIES_FILE%
set "PROJECT_BASE=%~dp0"
set BUILD_TYPE=debug
set MSVC_VERSION=
set REBUILD=build
set ALLHEIGHTMAPS=false
set SKIPHEIGHTMAPS=false
set DEPENDENCIESONLY=false
set BUILDNUMBER=0
set BUILD_ERROR=false
set HALT_ON_ERROR=true
set halt=

goto :eof
rem --- End of SET_DEFAULTS ----------------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of PROCESS_ARGUMENTS ---------------------------------------------
:PROCESS_ARGUMENTS

if "%~0" == "" goto :CONTINUE_FROM_PROCESS_ARGUMENTS


if "%~0" == "-h" (
	echo msvc_build.cmd Help
	echo.
	echo "    /builddeps                     Builds only the project dependencies"
	echo "    /allheightmaps                 Downloads all of the heightmaps"
	echo "    /nohaltonerror                 Skips halting on errors"
	echo "    /skipheightmaps                Skips downloading heightmap files"
	echo "    /rebuild                       Rebuilds the projects instead of incremental build"
	echo "    /clean                         Cleans the generated files"
	echo "    /build [debug-release-all]     Specifies the build type, defaults to debug"
	echo "    /msvc-version [vc9|vc10]       Specifies the msvc version and project files to use"
	echo "    /buildnumber [num]             Specifies a build number to be set rather than commit hash"
)

if "%~0" == "/clean" (
	call :CLEAN_BUILD
	goto :eof
)

if "%~0" == "/builddeps" (
	set DEPENDENCIESONLY=true
)

if "%~0" == "/allheightmaps" (
	set ALLHEIGHTMAPS=true
)

if "%~0" == "/skipheightmaps" (
	set SKIPHEIGHTMAPS=true
)

if "%~0" == "/nohaltonerror" (
	set HALT_ON_ERROR=false
)

rem Check for /rebuild then set REBUILD
if "%~0" == "/rebuild" (
	set REBUILD=rebuild
)


rem Check for /buildnumber x format and then set BUILDNUMBER
if "%~0" == "/buildnumber" (
	set BUILDNUMBER=%~1
	shift
)

rem Check for /build:x format and then set BUILD_TYPE
if "%~0" == "/build" (
	set BUILD_TYPE=%~1
	shift
)


rem Check for /msvc-version:x format and then set MSVC_VERSION
if "%~0" == "/msvc-version" (
	rem Only set if it's an allowed version
	if "%~1" == "vc9" (
		set MSVC_VERSION=9
	)

	if "%~1" == "vc10" (
		set MSVC_VERSION=10
	)

	shift
)

shift

goto :PROCESS_ARGUMENTS
rem --- End of PROCESS_ARGUMENTS -----------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of CLEAN_BUILD ---------------------------------------------------
rem --- Cleans all output created by the build process, restoring the        ---
rem --- project to it's original state like a fresh checkout.                ---
:CLEAN_BUILD

echo Cleaning the build environment

if exist "bin" (
	if exist "%PROJECT_BASE%bin/Debug" (
	cd "%PROJECT_BASE%bin/Debug"

		if exist *.exe del /Q *.exe
		if exist *.ilk del /Q *.ilk
		if exist *.pdb del /Q *.pdb
		if exist *.map del /Q *.map

	cd "%PROJECT_BASE%"
	)

	if exist "%PROJECT_BASE%bin/Release" (
	cd "%PROJECT_BASE%bin/Release"

		if exist *.exe del /Q *.exe
		if exist *.ilk del /Q *.ilk
		if exist *.pdb del /Q *.pdb
		if exist *.map del /Q *.map

	cd "%PROJECT_BASE%"
	)
)

if exist "deps" rmdir /S /Q "deps"
if exist "build-aux" rmdir /S /Q "build-aux"

goto :eof
rem --- End of CLEAN_BUILD -----------------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of BUILD_ENVIRONMENT ---------------------------------------------
:BUILD_ENVIRONMENT

if "%MSVC_VERSION%" == "" (
	if exist "%VS100COMNTOOLS%" (
		set MSVC_VERSION=10
	) else if exist "%VS90COMNTOOLS%" (
		set MSVC_VERSION=9
	) else (
		echo ***** Microsoft Visual Studio 9.0 or 10.0 required *****
		exit
	)
)

if %MSVC_VERSION% == 10 (
	set "VSCOMMONTOOLS=%VS100COMNTOOLS%"
	set "MSBUILD=%WINDIR%\Microsoft.NET\Framework\v4.0.30319\msbuild.exe"
) else if %MSVC_VERSION% == 9 (
	set "VSCOMMONTOOLS=%VS90COMNTOOLS%"
	set "MSBUILD=%WINDIR%\Microsoft.NET\Framework\v3.5\msbuild.exe"
) else (
	echo ***** Microsoft Visual Studio 9.0 or 10.0 required *****
	exit
)

call "%VSCOMMONTOOLS%\vsvars32.bat" >NUL

set environment_built=yes

goto :eof
rem --- End of BUILD_ENVIRONMENT -----------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of DOWNLOAD_DATA_FILES -------------------------------------------
rem --- Downloads datafiles such as heightmaps needed to run the project.    ---
:DOWNLOAD_DATA_FILES

echo ** Checking data file dependencies **
echo.

if not exist "data\heightmaps" (
	mkdir data\heightmaps
)

call :DOWNLOAD_HEIGHTMAP tatooine

if %ALLHEIGHTMAPS% == true (
	call :DOWNLOAD_HEIGHTMAP corellia
	call :DOWNLOAD_HEIGHTMAP dantooine
	call :DOWNLOAD_HEIGHTMAP dathomir
	call :DOWNLOAD_HEIGHTMAP endor
	call :DOWNLOAD_HEIGHTMAP lok
	call :DOWNLOAD_HEIGHTMAP naboo
	call :DOWNLOAD_HEIGHTMAP rori
	rem call :DOWNLOAD_HEIGHTMAP taanab
	call :DOWNLOAD_HEIGHTMAP talus
	call :DOWNLOAD_HEIGHTMAP yavin4
)
echo ** Checking data file dependencies complete **

goto :eof
rem --- End of DOWNLOAD_DATA_FILES ---------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of DOWNLOAD_HEIGHTMAP --------------------------------------------
rem --- Downloads datafiles such as heightmaps needed to run the project.    ---
:DOWNLOAD_HEIGHTMAP

if not exist "data\heightmaps\%1.hmpw" (
	if not exist "data\heightmaps\%1.zip" (
		echo ** Downloading Heightmap for %1 **
		echo.

		"tools\wget.exe" http://swganh.com/^^!^^!planets^^!^^!/%1.zip -O data\heightmaps\%1.zip

		echo ** Downloading heightmap complete **
	)

	"tools\unzip.exe" data\heightmaps\%1.zip -d data\heightmaps >NUL
	move "%PROJECT_BASE%data\heightmaps\%1.hmp" "%PROJECT_BASE%data\heightmaps\%1.hmpw"

	if exist "data\heightmaps\%1.hmpw" (
		del data\heightmaps\%1.zip
	)
)

goto :eof
rem --- End of DOWNLOAD_HEIGHTMAP ----------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of BUILD_DEPENDENCIES --------------------------------------------
rem --- Builds all external dependencies needed by the project.              ---
:BUILD_DEPENDENCIES

echo ** Building dependencies necessary for this project **
echo.

if not exist "deps" call :DOWNLOAD_DEPENDENCIES

if not exist "deps\VERSION" (
	echo ** Dependencies out of date -- Updating now **
	call :CLEAN_BUILD
	call :DOWNLOAD_DEPENDENCIES
	echo ** Dependencies updated **
)

set /p current_version=<"deps\VERSION"

if not %current_version% == %DEPENDENCIES_VERSION% (
	echo ** Dependencies out of date -- Updating now **

	rem Need to do a full rebuild after updating dependenceies
	set REBUILD=rebuild

	call :CLEAN_BUILD
	call :DOWNLOAD_DEPENDENCIES
	echo ** Dependencies updated **
)

if exist "deps\boost" call :BUILD_BOOST
if exist "deps\gtest" call :BUILD_GTEST
if exist "deps\gmock" call :BUILD_GMOCK
if exist "deps\lua" call :BUILD_LUA
if exist "deps\noise" call :BUILD_NOISE
if exist "deps\spatialindex" call :BUILD_SPATIALINDEX
if exist "deps\tolua++" call :BUILD_TOLUA
if exist "deps\zlib" call :BUILD_ZLIB

echo ** Building dependencies complete **

goto :eof
rem --- End of BUILD_DEPENDENCIES ----------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of DOWNLOAD_DEPENDENCIES -----------------------------------------
rem --- Downloads the dependency package for the current version of the source -
:DOWNLOAD_DEPENDENCIES

if not exist "%DEPENDENCIES_FILE%" (
	"tools\wget.exe" !DEPENDENCIES_URL! -O "%DEPENDENCIES_FILE%"
)

if exist "%DEPENDENCIES_FILE%" (
	echo Extracting dependencies ...
	"tools\unzip.exe" "%DEPENDENCIES_FILE%" >NUL
	echo %DEPENDENCIES_VERSION% >"deps\VERSION"
	echo Complete!
	echo.
)

goto :eof
rem --- End of DOWNLOAD_DEPENDENCIES -------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of BUILD_BOOST ---------------------------------------------------
rem --- Builds the boost library for use with this project.                  ---
:BUILD_BOOST

echo BUILDING: Boost - http://www.boost.org/

cd "%PROJECT_BASE%deps\boost"

rem Only build boost if it hasn't been built already.
if "%BUILD_TYPE%" == "debug" (
	if exist "stage\lib\libboost_*-mt-sgd.lib" (
		echo Boost libraries already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "release" (
	if exist "stage\lib\libboost_*-mt-s.lib" (
		echo Boost libraries already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "all" (
	if exist "stage\lib\libboost_*-mt-sgd.lib" (
		if exist "stage\lib\libboost_*-mt-s.lib" (
			echo Boost libraries already built ... skipping
			echo.
			cd "%PROJECT_BASE%"
			goto :eof
		)
	)
)

rem Build BJAM which is needed to build boost.
if not exist "tools\jam\src\bin.ntx86\bjam.exe" (
	cd "tools\jam\src"
	cmd /q /c build.bat >NUL 2>&1
	cd "%PROJECT_BASE%deps\boost"
)

rem Build the boost libraries we need.

if "%BUILD_TYPE%" == "debug" (
	cmd /c "tools\jam\src\bin.ntx86\bjam.exe" --toolset=msvc-%MSVC_VERSION%.0 --with-log --with-date_time --with-thread --with-regex --with-system variant=debug link=static runtime-link=static threading=multi >NUL
)

if "%BUILD_TYPE%" == "release" (
	cmd /c "tools\jam\src\bin.ntx86\bjam.exe" --toolset=msvc-%MSVC_VERSION%.0 --with-log --with-date_time --with-thread --with-regex --with-system variant=release link=static runtime-link=static threading=multi >NUL
)

if "%BUILD_TYPE%" == "all" (
	cmd /c "tools\jam\src\bin.ntx86\bjam.exe" --toolset=msvc-%MSVC_VERSION%.0 --with-log --with-date_time --with-thread --with-regex --with-system variant=debug,release link=static runtime-link=static threading=multi >NUL
)

cd "%PROJECT_BASE%"

goto :eof
rem --- End of BUILD_BOOST -----------------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of BUILD_GTEST ---------------------------------------------------
rem --- Builds all googletest library used for unit testing.                 ---
:BUILD_GTEST

echo BUILDING: Google Test - http://code.google.com/p/googletest/

cd "%PROJECT_BASE%deps\gtest\msvc"

rem Only build gtest if it hasn't been built already.
if "%BUILD_TYPE%" == "debug" (
	if exist "debug\gtestd.lib" (
		echo Google Test library already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "release" (
	if exist "release\gtest.lib" (
		echo Google Test library already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)

if "%BUILD_TYPE%" == "all" (
	if exist "debug\gtestd.lib" (
		if exist "release\gtest.lib" (
			echo Google Test library already built ... skipping
			echo.
			cd "%PROJECT_BASE%"
			goto :eof
		)
	)
)

if exist "*.cache" del /S /Q "*.cache" >NUL

if "%BUILD_TYPE%" == "debug" (
	"%MSBUILD%" "gtest_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "release" (
	"%MSBUILD%" "gtest_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "all" (
	"%MSBUILD%" "gtest_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL

	"%MSBUILD%" "gtest_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

cd "%PROJECT_BASE%"

goto :eof
rem --- End of BUILD_GTEST -----------------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of BUILD_GMOCK ---------------------------------------------------
rem --- Builds all googlemock library used for unit testing.                 ---
:BUILD_GMOCK

echo BUILDING: Google Mock - http://code.google.com/p/googlemock/

cd "%PROJECT_BASE%deps\gmock\msvc"

rem Only build mock if it hasn't been built already.
if "%BUILD_TYPE%" == "debug" (
	if exist "debug\gmock.lib" (
		echo Google Mock library already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "release" (
	if exist "release\gmock.lib" (
		echo Google Mock library already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)

if "%BUILD_TYPE%" == "all" (
	if exist "debug\gmock.lib" (
		if exist "release\gmock.lib" (
			echo Google Mock library already built ... skipping
			echo.
			cd "%PROJECT_BASE%"
			goto :eof
		)
	)
)

if exist "%PROJECT_BASE%deps\gmock\msvc\*.cache" del /S /Q "%PROJECT_BASE%deps\gmock\msvc\*.cache" >NUL

if "%BUILD_TYPE%" == "debug" (
	"%MSBUILD%" "gmock_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "release" (
	"%MSBUILD%" "gmock_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "all" (
	"%MSBUILD%" "gmock_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL

	"%MSBUILD%" "gmock_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

cd "%PROJECT_BASE%"

goto :eof
rem --- End of BUILD_GMOCK -----------------------------------------------------
rem ----------------------------------------------------------------------------



rem ----------------------------------------------------------------------------
rem --- Start of BUILD_LUA -----------------------------------------------------
rem --- Builds the lua library for use with this project.                    ---
:BUILD_LUA

echo BUILDING: Lua - http://www.lua.org/

cd "%PROJECT_BASE%deps\lua"

rem Only build lua if it hasn't been built already.
if "%BUILD_TYPE%" == "debug" (
	if exist "lib\lua5.1d.lib" (
		echo Lua already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "release" (
	if exist "lib\lua5.1.lib" (
		echo Lua already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "all" (
	if exist "lib\lua5.1d.lib" (
		if exist "lib\lua5.1.lib" (
			echo Lua already built ... skipping
			echo.
			cd "%PROJECT_BASE%"
			goto :eof
		)
	)
)

rem Build the lua libraries we need.

if "%BUILD_TYPE%" == "debug" (
	call :COMPILE_LUA debug >NUL
)

if "%BUILD_TYPE%" == "release" (
	call :COMPILE_LUA release >NUL
)

if "%BUILD_TYPE%" == "all" (
	call :COMPILE_LUA debug >NUL
	call :COMPILE_LUA release >NUL
)

cd "%PROJECT_BASE%"

goto :eof
rem --- End of BUILD_LUA -------------------------------------------------------
rem ----------------------------------------------------------------------------



rem ----------------------------------------------------------------------------
rem --- Start of COMPILE_LUA ---------------------------------------------------
rem --- Compiles the lua library.                                            ---
:COMPILE_LUA

if "%1" == "debug" (
	set buildcmd=cl /nologo /MTd /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE
	set libname=lua5.1d.lib
)

if "%1" == "release" (
	set buildcmd=cl /nologo /MT /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE
	set libname=lua5.1.lib
)

if "%buildcmd%" == "" (
	set buildcmd=cl /nologo /MTd /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE
	set libname=lua5.1d.lib
)

rem Create the output directory if it does not yet exist.
if not exist "lib" (
	mkdir "lib"
)

cd src

%buildcmd% *.c

del lua.obj luac.obj

lib /out:../lib/%libname% *.obj

del *.obj

cd ..

goto :eof
rem --- End of COMPILE_LUA -----------------------------------------------------
rem ----------------------------------------------------------------------------



rem ----------------------------------------------------------------------------
rem --- Start of BUILD_NOISE ---------------------------------------------------
rem --- Builds the noise library for use with this project.                  ---
:BUILD_NOISE

echo BUILDING: Noise - http://libnoise.sourceforge.net/

cd "%PROJECT_BASE%deps\noise"

rem Only build noise if it hasn't been built already.
if "%BUILD_TYPE%" == "debug" (
	if exist "win32\Debug\libnoise.lib" (
		echo Noise already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "release" (
	if exist "win32\Release\libnoise.lib" (
		echo Noise already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "all" (
	if exist "win32\Debug\libnoise.lib" (
		if exist "win32\Release\libnoise.lib" (
			echo Noise already built ... skipping
			echo.
			cd "%PROJECT_BASE%"
			goto :eof
		)
	)
)

rem Build the noise libraries we need.

rem VS likes to create these .cache files and then complain about them existing afterwards.
rem Removing it as it's not needed.
if exist "*.cache" del /S /Q "*.cache" >NUL

if "%BUILD_TYPE%" == "debug" (
	"%MSBUILD%" "libnoise_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "release" (
	"%MSBUILD%" "libnoise_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "all" (
	"%MSBUILD%" "libnoise_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL

	"%MSBUILD%" "libnoise_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

cd "%PROJECT_BASE%"

goto :eof
rem --- End of BUILD_NOISE -----------------------------------------------------
rem ----------------------------------------------------------------------------



rem ----------------------------------------------------------------------------
rem --- Start of BUILD_SPATIALINDEX --------------------------------------------
rem --- Builds the spatial index library for use with this project.          ---
:BUILD_SPATIALINDEX

echo BUILDING: SpatialIndex - http://research.att.com/~marioh/spatialindex/

cd "%PROJECT_BASE%deps\spatialindex"

rem Only build spatial index if it hasn't been built already.
if "%BUILD_TYPE%" == "debug" (
	if exist "Debug\spatialIndex_d.lib" (
		echo SpatialIndex already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "release" (
	if exist "Release\spatialIndex.lib" (
		echo SpatialIndex already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "all" (
	if exist "Debug\spatialIndex_d.lib" (
		if exist "Release\spatialIndex.lib" (
			echo SpatialIndex already built ... skipping
			echo.
			cd "%PROJECT_BASE%"
			goto :eof
		)
	)
)

rem Build the spatial index libraries we need.

rem VS likes to create these .cache files and then complain about them existing afterwards.
rem Removing it as it's not needed.
if exist "*.cache" del /S /Q "*.cache" >NUL

if "%BUILD_TYPE%" == "debug" (
	"%MSBUILD%" "spatialindex_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "release" (
	"%MSBUILD%" "spatialindex_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "all" (
	"%MSBUILD%" "spatialindex_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL

	"%MSBUILD%" "spatialindex_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

cd "%PROJECT_BASE%"

goto :eof
rem --- End of BUILD_SPATIALINDEX ----------------------------------------------
rem ----------------------------------------------------------------------------



rem ----------------------------------------------------------------------------
rem --- Start of BUILD_TOLUA ---------------------------------------------------
rem --- Builds the tolua++ library for use with this project.                ---
:BUILD_TOLUA

echo BUILDING: tolua++ - http://www.codenix.com/~tolua/

cd "%PROJECT_BASE%deps\tolua++\win32\vc9"

rem Only build tolua++ if it hasn't been built already.
if "%BUILD_TYPE%" == "debug" (
	if exist "withLua51_Debug\toluapp.lib" (
		echo tolua++ already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "release" (
	if exist "withLua51_Release\toluapp.lib" (
		echo tolua++ already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "all" (
	if exist "withLua51_Debug\toluapp.lib" (
		if exist "withLua51_Release\toluapp.lib" (
			echo tolua++ already built ... skipping
			echo.
			cd "%PROJECT_BASE%"
			goto :eof
		)
	)
)

rem Build the tolua++ libraries we need.

rem VS likes to create these .cache files and then complain about them existing afterwards.
rem Removing it as it's not needed.
if exist "*.cache" del /S /Q "*.cache" >NUL

if "%BUILD_TYPE%" == "debug" (
	"%MSBUILD%" "toluapp_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=withLua51_Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "release" (
	"%MSBUILD%" "toluapp_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=withLua51_Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "all" (
	"%MSBUILD%" "toluapp_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=withLua51_Debug,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL

	"%MSBUILD%" "toluapp_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration=withLua51_Release,VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

cd "%PROJECT_BASE%"

goto :eof
rem --- End of BUILD_TOLUA -----------------------------------------------------
rem ----------------------------------------------------------------------------



rem ----------------------------------------------------------------------------
rem --- Start of BUILD_ZLIB ----------------------------------------------------
rem --- Builds the zlib library for use with this project.                   ---
:BUILD_ZLIB

echo BUILDING: zlib - http://www.zlib.net/

cd "%PROJECT_BASE%deps\zlib\projects\visualc6"

rem Only build zlib if it hasn't been built already.
if "%BUILD_TYPE%" == "debug" (
	if exist "Win32_LIB_Debug\zlibd.lib" (
		echo zlib library already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "release" (
	if exist "Win32_LIB_Release\zlib.lib" (
		echo zlib library already built ... skipping
		echo.
		cd "%PROJECT_BASE%"
		goto :eof
	)
)
if "%BUILD_TYPE%" == "all" (
	if exist "Win32_LIB_Debug\zlibd.lib" (
		if exist "Win32_LIB_Release\zlib.lib" (
			echo zlib library already built ... skipping
			echo.
			cd "%PROJECT_BASE%"
			goto :eof
		)
	)
)

rem Build the zlib library.

rem VS likes to create these .cache files and then complain about them existing afterwards.
rem Removing it as it's not needed.
if exist "zlib.sln.cache" del /S /Q "zlib.sln.cache" >NUL

if "%BUILD_TYPE%" == "debug" (
	"%MSBUILD%" "zlib_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration="LIB Debug",VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "release" (
	"%MSBUILD%" "zlib_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration="LIB Release",VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "all" (
	"%MSBUILD%" "zlib_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration="LIB Debug",VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL

	"%MSBUILD%" "zlib_vc%MSVC_VERSION%.sln" /t:build /p:Platform=Win32,Configuration="LIB Release",VCBuildAdditionalOptions="/useenv"
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

cd "%PROJECT_BASE%"

goto :eof
rem --- End of BUILD_ZLIB ------------------------------------------------------
rem ----------------------------------------------------------------------------



rem ----------------------------------------------------------------------------
rem --- Start of BUILD_PROJECT -------------------------------------------------
rem --- Builds the actual project.                                           ---
:BUILD_PROJECT

cd "%PROJECT_BASE%"

if exist "*.cache" del /S /Q "*.cache" >NUL
if exist "build-aux\*.xml" del /S /Q "build-aux\*.xml" >NUL

if "%BUILD_TYPE%" == "debug" (
	"%MSBUILD%" "mmoserver_vc%MSVC_VERSION%.sln" /t:%REBUILD% /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "release" (
	"%MSBUILD%" "mmoserver_vc%MSVC_VERSION%.sln" /t:%REBUILD% /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "all" (
	"%MSBUILD%" "mmoserver_vc%MSVC_VERSION%.sln" /t:%REBUILD% /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1
	if exist "*.cache" del /S /Q "*.cache" >NUL

	"%MSBUILD%" "mmoserver_vc%MSVC_VERSION%.sln" /t:%REBUILD% /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

goto :eof
rem --- End of BUILD_PROJECT ---------------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of SLEEP ---------------------------------------------------------
rem --- Waits some seconds before returning.                                 ---
:SLEEP
ping -n %1 -w 1 127.0.0.1>NUL
goto :eof
rem --- End of SLEEP -----------------------------------------------------------
rem ----------------------------------------------------------------------------

ENDLOCAL