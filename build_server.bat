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

%PROJECT_DRIVE%
cd %PROJECT_BASE%

if %SKIPHEIGHTMAPS% == false (
call :DOWNLOAD_DATA_FILES
)

call :BUILD_PROJECT

if not %ERRORLEVEL% == 0 (
if %HALT_ON_ERROR% == true (set /p halt=*** BUILD FAILED... PRESS ENTER TO CONTINUE ***)
exit /b %ERRORLEVEL%
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

set DEPENDENCIES_VERSION=0.4.3
set DEPENDENCIES_FILE=mmoserver-deps-%DEPENDENCIES_VERSION%.tar.bz2
set DEPENDENCIES_URL=http://github.com/downloads/swganh/mmoserver/%DEPENDENCIES_FILE%
set "PROJECT_BASE=%~dp0"
set "PROJECT_DRIVE=%~d0"
set PATH=%PROJECT_BASE%tools\windows;%PATH%
set ALLHEIGHTMAPS=false
set ERRORLEVEL=0
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
	echo "    /clean                         Cleans the generated files"
)

if "%~0" == "/clean" (
	call :CLEAN_BUILD
	goto :eof
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

if exist "build" rmdir /S /Q "build"

goto :eof
rem --- End of CLEAN_BUILD -----------------------------------------------------
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
	if not exist "data\heightmaps\%1.hmpw.7z" (
		echo ** Downloading Heightmap for %1 **
		echo.
		"wget" --no-check-certificate http://github.com/downloads/anhstudios/swg-heightmaps/%1.hmpw.7z -O data\heightmaps\%1.hmpw.7z
		echo ** Downloading heightmap complete **
	)

	"7z" x -y -odata\heightmaps data\heightmaps\%1.hmpw.7z 
)

goto :eof
rem --- End of DOWNLOAD_HEIGHTMAP ----------------------------------------------
rem ----------------------------------------------------------------------------


rem ----------------------------------------------------------------------------
rem --- Start of BUILD_PROJECT -------------------------------------------------
rem --- Builds the actual project.                                           ---
:BUILD_PROJECT

if not exist "%PROJECT_BASE%build" (
    mkdir "%PROJECT_BASE%build"    
)
cd "%PROJECT_BASE%build"

cmake -G "Visual Studio 10" -DENABLE_TEST_REPORT=ON ..

cmake --build .

goto :eof
rem --- End of BUILD_PROJECT ---------------------------------------------------
rem ----------------------------------------------------------------------------

ENDLOCAL