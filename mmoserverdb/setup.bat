@ECHO off
:: ---------------------------------------------------------------------------------------
:: This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)
:: 
:: For more information, visit http://www.swganh.com
:: 
:: Copyright (c) 2006 - 2010 The SWG:ANH Team
:: ---------------------------------------------------------------------------------------
:: This library is free software; you can redistribute it and/or
:: modIFy it under the terms of the GNU Lesser General Public
:: License as published by the Free Software Foundation; either
:: version 2.1 of the License, or (at your option) any later version.
:: 
:: This library is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
:: Lesser General Public License for more details.
:: 
:: You should have received a copy of the GNU Lesser General Public
:: License along with this library; IF not, write to the Free Software
:: Foundation, Inc., 51 Franklin Street, FIFth Floor, Boston, MA  02110-1301  USA
:: ---------------------------------------------------------------------------------------


:: -- Prepare the Command Processor
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION
MODE con:cols=80 lines=58

:: Set initial VARS


CALL :SET_DEFAULTS

REM --
REM Main
REM --

GOTO :PROCESS_ARGS
:CONTINUE_FROM_PROCESS_ARGUMENTS

ECHO. Project Base: %PROJECT_BASE%
ECHO. Install Type Selected: %INSTALL_TYPE%
ECHO. Backup Type Selected: %BACKUP_TYPE%
ECHO.
ECHO. User: %db_user%
ECHO. Password: %db_pass%
ECHO. IP: %db_host%

::ECHO.
::ECHO.
::ECHO. We're Done!




GOTO :EOF


REM --
REM Functions
REM --

:SET_DEFAULTS

	SET INSTALL_TYPE=full
	SET BACKUP_TYPE=full
	SET "PROJECT_BASE=%~dp0"
	CALL :READ_CFG

	GOTO :eof

:PROCESS_ARGS
	IF "%~0" == "" GOTO :CONTINUE_FROM_PROCESS_ARGUMENTS

	IF "%~0" == "-h" (
		ECHO SWG:ANH Database Installer Help
		ECHO.
		ECHO.
		ECHO.  /install [option]       Install the SWG:ANH Database Schema
		ECHO.    all                   Install the full Database
		ECHO.    swganh                Install the swganh Database
		ECHO.    config                Install the Config Database
		ECHO.    logging               Install the Logging Database
		ECHO.    astromech             Install the AstroMech Database
		ECHO.    archive               Install the Archive Database
		ECHO.
		ECHO.  /backup [option]        Backup the SWG:ANH Database Schema
		ECHO.    all                   Backup the full suite of SWG:ANH Databases
		ECHO.    swganh                Backup the swganh Database
		ECHO.    config                Backup the Config Database
		ECHO.    logging               Backup the Logging Database
		ECHO.    astromech             Backup the AstroMech Database
		ECHO.    archive               Backup the Archive Databse
		ECHO.
		ECHO.  /update                 Update the database
		ECHO.
		ECHO.  /checkdbversion         Check the database version
		ECHO.
		ECHO.  /checkscriptversion     Check the script version
		ECHO.
		ECHO.  /clean                  Clean the Databases
		ECHO.
		ECHO.  /generateresources      Generate the initial resource spawn
		ECHO.
		ECHO.  /shiftresourcespawn     ShIFt the current resource spawn
		ECHO.
		ECHO.  /resetglobaltimer       Reset the Global Server Timer
		ECHO.
		ECHO.  /checkconfig            Check the config file values
	)

	::
	:: Check for Install Options
	::
	
	IF "%~0" == "/install" (
		set INSTALL_TYPE=%~1
		CALL :INSTALL_DB
    shift
	)

	::
	:: Check for Backup Options
	::

	IF "%~0" == "/backup" (
	  set BACKUP_TYPE=%~1
	  CALL :BACKUP_DB
	)
	
	::
	:: Check for Update
	::
	

	IF "%~0" == "/update" (
		CALL :UPDATE_DB
		GOTO :eof
	)

	::
	:: Check for DB Version
	::
	

	IF "%~0" == "/checkdbversion" (
		CALL :CHECK_DB_VERSION
	)

	::
	:: Check for Script Version
	::
	

	IF "%~0" == "/checkscriptversion" (
		CALL :CHECK_SCRIPT_VERSION
	)

	::
	:: Check for Clean
	::

	IF "%~0" == "/clean" (
		CALL :CLEAN_DB
		GOTO :eof
	)
	
	::
	:: Check for Generate Resources
	::

	IF "%~0" == "/generateresources" (
		CALL :GENERATE_RESOURCES
		GOTO :EOF
	)

	::
	:: Check for Shift Resources
	::
	

	IF "%~0" == "/shiftresourcespawn" (
		CALL :SHIFT_RESOURCES
	)

	::
	:: Check for Reset Global Timer
	::
	
	IF "%~0" == "/resetglobaltimer" (
		CALL :RESET_GLOBAL_TIMER
		GOTO :eof
	)

	::
	:: Check Config File Values
	::
	
	IF "%~0" == "/checkconfig" (
		CALL :RESET_GLOBAL_TIMER
		GOTO :eof
	)
	
	SHIFT

GOTO :PROCESS_ARGS

:READ_CFG
	FOR /F "tokens=2 delims==" %%a IN ('find "username" ^< setup.cfg') DO SET db_user=%%a
	FOR /F "tokens=2 delims==" %%a IN ('find "password" ^< setup.cfg') DO SET db_pass=%%a
	FOR /F "tokens=2 delims==" %%a IN ('find "host" ^< setup.cfg') DO SET db_host=%%a
	
	GOTO :EOF

:CLEAN_DB
	ECHO.
	ECHO. CLEAN DB CALLED
	
	GOTO :EOF
	
:GENERATE_RESOURCES
	ECHO.
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh --execute="CALL sp_ResourceInitialSpawn()"
	ECHO.
	ECHO. Intial Resources Spawn generated.
	
	GOTO :EOF
	
:RESET_GLOBAL_TIMER
	ECHO.
	ECHO. RESET GLOBAL TIMER CALLED
	ECHO.
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh --execute="UPDATE galaxy SET Global_Tick_Count = 0;"
	
	GOTO :EOF
	
:INSTALL_DB
	ECHO.
	ECHO. INSTALL DB CALLED
	
	:: Install ALL Databases
	
	IF "%INSTALL_TYPE%" == "all" (
		cd "%PROJECT_BASE%swganh"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create_users.sql"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_archive"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_astromech"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_config"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_logs"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%"
		cd "%PROJECT_BASE%swganh"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh\functions"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh\procedures"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_archive"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_archive\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_archive\functions"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_archive\procedures"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_astromech"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_astromech\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_astromech\functions"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_astromech\procedures"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_config"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_config\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_config\functions"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_config\procedures"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_logs"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_logs\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_logs\functions"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_logs\procedures"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
	)
	
	::
	:: Install swganh Database
	::
	
	IF "%INSTALL_TYPE%" == "swganh" (
		cd "%PROJECT_BASE%swganh"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create_users.sql"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh\functions"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh\procedures"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%"				
	)
	
	::
	:: Install swganh_archive Database
	::
	
	IF "%INSTALL_TYPE%" == "archive" (
		cd "%PROJECT_BASE%swganh_archive"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_archive\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_archive\functions"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_archive\procedures"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%"				
	)
	
	::
	:: Install swganh_astromech Database
	::
	
	IF "%INSTALL_TYPE%" == "astromech" (
		cd "%PROJECT_BASE%swganh_astromech"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_astromech\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_astromech\functions"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_astromech\procedures"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%"				
	)
	
	::
	:: Install swganh_config Database
	::
	
	IF "%INSTALL_TYPE%" == "config" (
		cd "%PROJECT_BASE%swganh_config"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_config\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_config\functions"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_config\procedures"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%"				
	)

	::
	:: Install swganh_config Database
	::
	
	IF "%INSTALL_TYPE%" == "logging" (
		cd "%PROJECT_BASE%swganh_logs"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
		cd "%PROJECT_BASE%swganh_logs\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_logs\functions"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%swganh_logs\procedures"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%"				
	)
	
	GOTO :EOF
	
:BACKUP_DB
	ECHO.
	ECHO. BAKCUP DB CALLED
	ECHO. %BACKUP_TYPE%

	IF "%BACKUP_TYPE%" == "swganh" (
		mkdir backup
		cd backup
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_config.bak
		cd ..
        goto :eof
    )
	
	IF "%BACKUP_TYPE%" == "config" (
		mkdir backup
		cd backup
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh_config --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_config.bak
		cd ..
        goto :eof
    )
	
	IF "%BACKUP_TYPE%" == "astromech" (
		mkdir backup
		cd backup
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh_astromech --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_astromech.bak
		cd ..
        goto :eof
    )
	
	IF "%BACKUP_TYPE%" == "logging" (
		mkdir backup
		cd backup
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh_astromech --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_logging.bak
		cd ..
        goto :eof
    )
	
	IF "%BACKUP_TYPE%" == "archive" (
		mkdir backup
		cd backup
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh_astromech --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_archive.bak
		cd ..
        goto :eof
    )
	
	IF "%BACKUP_TYPE%" == "all" (
		mkdir backup
		cd backup
		ECHO. Backing up the swganh database
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh --create-options --extended-insert --routines --dump-date --triggers --comments > swganh.bak
		ECHO. Backing up the config database
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh_config --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_config.bak
		ECHO. Backing up the logs database
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh_logs --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_logs.bak
		ECHO. Backing up the AstroMech database
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh_astromech --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_astromech.bak
		ECHO. Backing up the archive database
		mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh_archive --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_archive.bak
		cd ..
        goto :eof
    )
	
	GOTO :EOF
	
:CHECKDBVERSION
	ECHO.
	
	SET db_rev_major=0
	
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh_config --execute="SELECT revisionMajor FROM swganh_config.swganh_version" >> output.log
	
	FOR /f "tokens=1,2,3 delims= " %%G IN (output.log) DO SET db_rev_major=%%G
	DEL output.log
	
	SET db_rev_minor=0
	
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh_config --execute="SELECT revisionMinor FROM swganh_config.swganh_version" >> output.log
	
	FOR /f "tokens=1,2,3 delims= " %%G IN (output.log) DO SET db_rev_minor=%%G
	DEL output.log
	
	ECHO. Server Database - Major Revision is %db_rev_major%
	ECHO. Server Database - Minor Revision is %db_rev_minor%
	
	GOTO :EOF
	
:SHIFT_RESOURCES

	ECHO.
	ECHO. SHIFT RESOURCES CALLED
	
	GOTO :EOF
	
ENDLOCAL