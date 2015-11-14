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

%PROJECT_DRIVE%
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

if not "%BUILDNUMBER%" == "false" (
    if "%BUILD_TYPE%" == "debug" (
        echo %BUILDNUMBER% >> build\bin\Debug\VERSION
    )

    if "%BUILD_TYPE%" == "release" (
        echo %BUILDNUMBER% >> build\bin\Release\VERSION
    )

    if "%BUILD_TYPE%" == "all" (
        echo %BUILDNUMBER% >> build\bin\Debug\VERSION
        echo %BUILDNUMBER% >> build\bin\Release\VERSION
    )
)

echo.
echo Server Successfully Built^^!

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

set DEPENDENCIES_VERSION=0.6.0
set DEPENDENCIES_FILE=mmoserver-deps-%DEPENDENCIES_VERSION%.tar.bz2
set DEPENDENCIES_URL=https://github.com/obi-two/Unofficial_Hope/releases/download/Downloads/%DEPENDENCIES_FILE%
set "PROJECT_BASE=%~dp0"
set "PROJECT_DRIVE=%~d0"
set PATH=%PROJECT_BASE%tools\windows;%PATH%
set BUILD_TYPE=debug
set REBUILD=build
set MSVC_VERSION=12
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

if exist "deps" rmdir /S /Q "deps"
if exist "build" rmdir /S /Q "build"

goto :eof
rem --- End of CLEAN_BUILD -----------------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of BUILD_ENVIRONMENT ---------------------------------------------
:BUILD_ENVIRONMENT

if not exist "%VS120COMNTOOLS%" (
  set "VS120COMNTOOLS=%PROGRAMFILES(X86)%\Microsoft Visual Studio 12.0\Common7\Tools"
  if not exist "!VS120COMNTOOLS!" (
  	  set "VS120COMNTOOLS=%PROGRAMFILES%\Microsoft Visual Studio 12.0\Common7\Tools"
  	  if not exist "!VS120COMNTOOLS!" (          
  		    rem TODO: Allow user to enter a path to their base visual Studio directory.
         
    	    echo ***** Microsoft Visual Studio 12.0 required *****
    	    exit /b 1
  	  )
  )
)

set "MSBUILD=%WINDIR%\Microsoft.NET\Framework\v4.0.30319\msbuild.exe"

call "%VS120COMNTOOLS%\vsvars32.bat" >NUL

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
	if not exist "data\heightmaps\%1.hmpw.zip" (
		echo ** Downloading Heightmap for %1 **
		echo.
		"wget" --no-check-certificate https://github.com/obi-two/Unofficial_Hope/releases/download/Downloads/%1.hmpw.zip -O data\heightmaps\%1.hmpw.zip
		echo ** Downloading heightmap complete **
	)

	"7z" x -y -odata\heightmaps data\heightmaps\%1.hmpw.zip 
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

call "%PROJECT_BASE%\deps\build_deps.bat"

echo ** Building dependencies complete **

goto :eof
rem --- End of BUILD_DEPENDENCIES ----------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of DOWNLOAD_DEPENDENCIES -----------------------------------------
rem --- Downloads the dependency package for the current version of the source -
:DOWNLOAD_DEPENDENCIES

if not exist "%DEPENDENCIES_FILE%" (
	"wget" --no-check-certificate !DEPENDENCIES_URL! -O "%DEPENDENCIES_FILE%"
)

if exist "%DEPENDENCIES_FILE%" (
	echo Extracting dependencies ...

	"tar" -xvjf "%DEPENDENCIES_FILE%"
	echo Complete!
	echo.
)

goto :eof
rem --- End of DOWNLOAD_DEPENDENCIES -------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of BUILD_PROJECT -------------------------------------------------
rem --- Builds the actual project.                                           ---
:BUILD_PROJECT

if not exist "%PROJECT_BASE%build" (
    mkdir "%PROJECT_BASE%build"    
)
cd "%PROJECT_BASE%build"

cmake -G "Visual Studio 12" -DCMAKE_INSTALL_PREFIX=%PROJECT_BASE% -DENABLE_TEST_REPORT=ON ..

if exist "*.cache" del /S /Q "*.cache" >NUL

if "%BUILD_TYPE%" == "debug" (
	"%MSBUILD%" "mmoserver.sln" /t:%REBUILD% /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1
	"%MSBUILD%" "RUN_TESTS.vcxproj" /t:%REBUILD% /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "release" (
	"%MSBUILD%" "mmoserver.sln" /t:%REBUILD% /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1	
	"%MSBUILD%" "RUN_TESTS.vcxproj" /t:%REBUILD% /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1
	if exist "*.cache" del /S /Q "*.cache" >NUL
)

if "%BUILD_TYPE%" == "all" (
	"%MSBUILD%" "mmoserver.sln" /t:%REBUILD% /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1	
	"%MSBUILD%" "RUN_TESTS.vcxproj" /t:%REBUILD% /p:Platform=Win32,Configuration=Debug,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1
	if exist "*.cache" del /S /Q "*.cache" >NUL

	"%MSBUILD%" "mmoserver.sln" /t:%REBUILD% /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
	if errorlevel 1 exit /b 1	
	"%MSBUILD%" "RUN_TESTS.vcxproj" /t:%REBUILD% /p:Platform=Win32,Configuration=Release,VCBuildAdditionalOptions="/useenv"
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
