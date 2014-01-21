:: ---------------------------------------------------------------------------------------
:: This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)
:: 
:: For more information, visit http://www.swganh.com
:: 
:: Copyright (c) 2006 - 2010 The SWG:ANH Team
:: ---------------------------------------------------------------------------------------
:: This library is free software; you can redistribute it and/or
:: modify it under the terms of the GNU Lesser General Public
:: License as published by the Free Software Foundation; either
:: version 2.1 of the License, or (at your option) any later version.
:: 
:: This library is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
:: Lesser General Public License for more details.
:: 
:: You should have received a copy of the GNU Lesser General Public
:: License along with this library; if not, write to the Free Software
:: Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
:: ---------------------------------------------------------------------------------------


:: -- Prepare the Command Processor
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION
MODE con:cols=72 lines=38

:: Set initial VARS
SET db_user=----
SET db_pass=----
SET db_host=----

:: Set the window title 
SET title=%~n0
TITLE %title%

::
:: Start
::

@ECHO OFF
	call:ScreenIntro
	call:Setup
	call:MainMenu

	::
	:: Functions
	::

:ScreenIntro
	CLS
	ECHO.
	ECHO.                MIIIIIIIIIIIINNNNNNNNNNNNNNNDIIIIIIIINNN
	ECHO.                MIIIIIIIIIIINNNNNNNNNNNNNNNNNNNNNIIIINNN
	ECHO.                MIIIIIIIIONNNNNNNNN,,NNNNNNNNNNNNNNNINNN
	ECHO.                MIIIIIIINNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
	ECHO.                MIIIIIINNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
	ECHO.                MIIIIIDNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
	ECHO.                MIIIIINNNNNNN=...............NNNNNNNNNNN
	ECHO.                MIIIIIINNNN8NNNNN......N+NNNNNN...DNNNNN
	ECHO.                MIIIIIIONNNNNNNN~,......DNNNNNNNNN..NNNN
	ECHO.                MIIIIIINNNNNNNN.N....... NNNNNNNNN....NN
	ECHO.                MIIIIINNNNNNN$.NN .........NNNNNN.....NN
	ECHO.                MIIINNNNNNNN.~NN8DNZ..................NN
	ECHO.                MINNNNNNNNN$NNNNNNNN............MN....NN
	ECHO.                MIINNNNNNNNNNNNN...:NN...........MN...NN
	ECHO.                MIINNNNNNNNNNNN,..... N..........N....NN
	ECHO.                MIINNNNNNNNNNNN.........D.............NN
	ECHO.                MIINNNNNNNNNNN8.......................NN
	ECHO.                MIINNNNNNNNNN ........................NN
	ECHO.                MIINNNNNNNND...NNN..................DNNN
	ECHO.                M8N80NNNNNNM...NNNNN................~.NN
	ECHO.                M0000NNNNNN ..NNNNNNN.................NN
	ECHO.                M0NNNNNNNNNN..NNNNNNN . NNN .........=NN
	ECHO.                M000NNNNNNNNN.NNNNNNNN..NNNN........NNNN
	ECHO.                M00NNNNNNNNNNNNNNNNNN7.............NNNNN
	ECHO.                M0000NNNNNNNNNNNNNNNNNN?.........NNNNNNN
	ECHO.                M0000NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
	ECHO.                M00000000NNNNNNNNNNN..D..N..NDN.:NNM..NN
	ECHO.                M00000000NNNNNNNN7,NN.N..N.N,,,8.Z.:NNNN
	ECHO.                M00000000NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
	ECHO.                M88888888888888888888888888888888888888N
	ECHO.
	ECHO.                     SWG:ANH - Database Installer
	ECHO.
	ECHO.
	CALL:sleep 5
	GOTO:EOF

:ShortMenu
	CLS
	ECHO. ----------------------------------------------------------------------
	ECHO.  SWGANH Database Install Script                              (v.0.02)
	ECHO. ----------------------------------------------------------------------
	ECHO.  DB IP: %db_host%     DB Username: %db_user%    DB Password: %db_pass%
	ECHO. ----------------------------------------------------------------------
	ECHO.
	ECHO.
	GOTO:EOF

:MainMenu
	CLS
	ECHO. ----------------------------------------------------------------------
	ECHO.  SWGANH Database Install Script                              (v.0.02)
	ECHO. ----------------------------------------------------------------------
	ECHO.  DB IP: %db_host%     DB Username: %db_user%    DB Password: %db_pass%
	ECHO. ----------------------------------------------------------------------
	ECHO.                                   ^|
	ECHO.           Database Setup          ^|       Database Maintenance
	ECHO.                                   ^|
	ECHO.   (1) Complete DB Install         ^|  (a) Complete Database Backup
	ECHO.   (2) Setup Databases ^& Users     ^|  (b) Main Database Backup
	ECHO.   (3) Setup Main Database         ^|  (c) AstroMech Database Backup
	ECHO.   (4) Setup Stored Procedures     ^|  (d) Remove Databases
	ECHO.   (5) Setup Stored Functions      ^| 
	ECHO.                                   ^|  
	ECHO.   (6) Setup Admin Scripts         ^|          Script Settings
	ECHO.   (7) Setup AstroMech Scripts     ^|
	ECHO.                                   ^|  (e) Username Change
	ECHO.        Server Configuration       ^|  (f) Password Change
	ECHO.                                   ^|  (g) IP Change
	ECHO.   (8) Generate Resources          ^|
	ECHO.   (9) Generate Bots               ^|               Help
	ECHO.   (0) Reset GlobalTimer           ^|
	ECHO.   (,) Change / Set MotD           ^|  (h) Help
	ECHO.                                   ^|  (s) Stats
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO. ----------------------------------------------------------------------
	SET /P Choice=Make a choice or ^(q^) quit : 

IF /I '%Choice%'=='1' GOTO :DatabaseCompleteSetup 

IF /I '%Choice%'=='2' GOTO :DatabaseSetup 

IF /I '%Choice%'=='3' GOTO :SchemaSetup

IF /I '%Choice%'=='4' GOTO :ProcedureSetup

IF /I '%Choice%'=='5' GOTO :FunctionSetup

IF /I '%Choice%'=='6' GOTO :AdminScriptSetup

IF /I '%Choice%'=='7' GOTO :AstroMechSetup

IF /I '%Choice%'=='8' GOTO :GenerateResources

IF /I '%Choice%'=='9' GOTO :GenerateBots

IF /I '%Choice%'=='0' GOTO :ResetGlobalTimer

IF /I '%Choice%'==',' GOTO :SetMOTD

IF /I '%Choice%'=='a' GOTO :CompleteBackup 

IF /I '%Choice%'=='b' GOTO :SchemaBackup

IF /I '%Choice%'=='c' GOTO :AstroMechBackup

IF /I '%Choice%'=='d' GOTO :DatabaseRemove

IF /I '%Choice%'=='e' GOTO :UserChange 

IF /I '%Choice%'=='f' GOTO :PassChange 

IF /I '%Choice%'=='g' GOTO :IPChange

IF /I '%Choice%'=='h' GOTO :Help 

IF /I '%Choice%'=='s' GOTO :Stats

IF /I '%Choice%'=='Q' GOTO :end 

GOTO:MainMenu

:DatabaseCompleteSetup 
	call:ShortMenu
	cd Create Scripts
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "swganh_logs.sql"
	TITLE %title%
	ECHO.
	ECHO.
	ECHO.
	ECHO.                      Database Creation Complete
	ECHO.
	ECHO.                           [Please Wait]
	cd ..
	call:sleep 5
	
	:: Load Main Schema
	
	call:ShortMenu
	cd Server Scripts
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%

	for /F %%A IN ('dir /b "*.sql"') do (
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO.Installing %%A [Done]
		call:doProgress
	)
	
	call:sleep 2
	TITLE %title%
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                          Database Loaded
	ECHO.
	ECHO.                           [Please Wait]
	cd ..
	call:sleep 5
	
	:: Load SPs
	
	call:ShortMenu
	cd Server Scripts
	cd Procedures
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
	
	for /F %%A IN ('dir /b "*.sql"') do (
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO.Installing %%A [Done]
		call:doProgress
	)
	
	cd ..
	cd ..
	call:sleep 2
	TITLE %title%
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                       Stored Procedures Loaded
	ECHO.
	ECHO.                            [Please Wait]
	call:sleep 5
	
	:: Load Stored Functions
	
	call:ShortMenu
	cd Server Scripts
	cd Functions
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
	
	for /F %%A IN ('dir /b "*.sql"') do (
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO.Installing %%A [Done]
		call:doProgress
	)

	cd ..
	cd ..
	call:sleep 2
	TITLE %title%
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                        Stored Functions Loaded
	ECHO.
	ECHO.                            [Please Wait]
	call:sleep 5
	
	:: Complete
	
	
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                        Database Load Completed
	ECHO.
	call:sleep 5
	
	GOTO:MainMenu
	
:DatabaseSetup 
	call:ShortMenu
	cd Create Scripts
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "swganh_logs.sql"
	TITLE %title%
	call:ScreenClear
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                      Database Creation Complete
	cd ..
	call:sleep 5
	GOTO:MainMenu
	
:SchemaSetup
	call:ShortMenu
	cd Server Scripts
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%

	for /F %%A IN ('dir /b "*.sql"') do (
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO.Installing %%A [Done]
		call:doProgress
	)
	
	call:sleep 2
	TITLE %title%
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                        Database Load Complete
	cd ..
	call:sleep 5
	GOTO:MainMenu
	
:ProcedureSetup
	call:ShortMenu
	cd Server Scripts
	cd Procedures
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
	
	for /F %%A IN ('dir /b "*.sql"') do (
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO.Installing %%A [Done]
		call:doProgress
	)
	
	cd ..
	cd ..
	call:sleep 2
	TITLE %title%
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                    Stored Procedures Load Complete
	call:sleep 5
	GOTO:MainMenu
	
:FunctionSetup
	call:ShortMenu
	cd Server Scripts
	cd Functions
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
	
	for /F %%A IN ('dir /b "*.sql"') do (
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO.Installing %%A [Done]
		call:doProgress
	)

	cd ..
	cd ..
	call:sleep 2
	TITLE %title%
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                     Stored Functions Load Complete
	call:sleep 5
	GOTO:MainMenu
	
:AdminScriptSetup
	call:ShortMenu
	cd AdminScripts
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
	
	for /F %%A IN ('dir /b "*.sql"') do (
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		call:doProgress
	)
	
	cd ..
	call:sleep 2
	TITLE %title%
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                    Admin Scripts Load Complete
	call:sleep 5
	GOTO:MainMenu

:AstroMechSetup
	call:ShortMenu
	cd AstroMech Scripts
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
	
	for /F %%A IN ('dir /b "*.sql"') do (
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		call:doProgress
	)
	
	cd ..
	call:sleep 2
	TITLE %title%
	call:ShortMenu
	ECHO.
	ECHO.
	ECHO.
	ECHO.                  AstroMech Scripts Load Complete
	call:sleep 5
	GOTO:MainMenu

:GenerateResources
	call:ShortMenu
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh --execute="CALL sp_ResourceInitialSpawn()"
	ECHO.
	ECHO.
	ECHO.                   Resource Generation is completed.
	ECHO.
	ECHO.
	call:sleep 5
	GOTO:MainMenu

:GenerateBots
	call:ShortMenu
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh --execute="CALL sp_CreateSWGANHBots(100);"
	ECHO.
	ECHO.
	ECHO.          100 bots and their accounts have been generated.
	ECHO.
	ECHO.
	call:sleep 5
	GOTO:MainMenu

:ResetGlobalTimer	call:ShortMenu
	call:ShortMenu
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh --execute="UPDATE galaxy SET Global_Tick_Count = 0;"
	ECHO.
	ECHO.
	ECHO.                  Global timer has been reset to zero.
	ECHO.
	ECHO.
	call:sleep 5
	GOTO:MainMenu

:SetMOTD
	call:ShortMenu
	ECHO.
	SET /P MoTD=Enter new MotD:
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh --execute="UPDATE config_zones SET motd ='%MoTD%';"
	ECHO.
	ECHO.
	ECHO.                  The new Message of the Day is:
	ECHO.
	ECHO.  %MoTD%
	ECHO.
	call:sleep 5
	GOTO:MainMenu
	
:CompleteBackup
	call:ShortMenu
	mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh swganh_archive swganh_astromech swganh_logs --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_complete.bak
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.                           Backup Complete
	call:sleep 5
	GOTO:MainMenu
	
:SchemaBackup
	call:ShortMenu
	mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh --create-options --extended-insert --routines --dump-date --triggers --comments > swganh.bak
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.                           Backup Complete
	call:sleep 5
	GOTO:MainMenu
	
:AstroMechBackup
	call:ShortMenu
	mysqldump --password=%db_pass% --host=%db_host% --user=%db_user% --databases swganh_astromech --create-options --extended-insert --routines --dump-date --triggers --comments > swganh_AstroMech.bak
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.                           Backup Complete
	call:sleep 5
	GOTO:MainMenu

:DatabaseRemove
	call:ShortMenu
	ECHO.
	ECHO.                         Removing Databases
	ECHO.
	call:sleep 2
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --execute="DROP DATABASE swganh;"
	ECHO.               Database swganh removed...
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --execute="DROP DATABASE swganh_AstroMech;"
	ECHO.               Database swganh_AstroMech removed...
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --execute="DROP DATABASE swganh_archive"
	ECHO.               Database swganh_archive removed...
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --execute="DROP DATABASE swganh_logs"
	ECHO.               Database swganh_logs removed...
	call:sleep 8
	GOTO:MainMenu

:UserChange
	call:ShortMenu
	SET /P db_user=Please enter the username with root access:  
	call:sleep
	GOTO:MainMenu	

:PassChange
	call:ShortMenu 
	SET /P db_pass=Please enter the password for the user: 
	call:sleep
	GOTO:MainMenu	

:IPChange
	call:ShortMenu
	SET /P db_host=Please enter the IP / hostname of the database: 
	call:sleep
	GOTO:MainMenu	

:ScreenClear
CLS
GOTO:EOF

:Help
	call:ScreenClear
	ECHO.
	ECHO. Welcome the the help section of the database installer....
	ECHO.
	ECHO.
	call:sleep
	goto:MainMenu
	
:Stats
	call:ShortMenu
	ECHO.                         Current Script VARS
	ECHO.
	ECHO.  The IP/Hostname of the database server is  -------^> %db_host%
	ECHO.
	ECHO.  The root privileged user for the database is -----^> %db_user%
	ECHO.
	ECHO.  The root privileged user password is -------------^> %db_pass%
	ECHO.
	ECHO. ----------------------------------------------------------------------
	ECHO.
	ECHO.                         Database Details
	ECHO.
	cd Server Scripts
	call:GetFileCount
	cd ..
	ECHO.  Number of SQL scripts for the main schema is  --------^> %maxcnt%
	ECHO.
	set maxcnt=0
	cd Server Scripts
	cd Functions
	call:GetFileCount
	cd ..
	cd ..
	ECHO.  Number of Stored Procedures for the main schema is  --^> %maxcnt%
	ECHO.
	set maxcnt=0
	cd Server Scripts
	cd Procedures
	call:GetFileCount
	cd ..
	cd ..
	ECHO.  Number of Stored Functions for the main schema is  ---^> %maxcnt%
	ECHO.
	ECHO. ----------------------------------------------------------------------
	ECHO.
	ECHO.
	ECHO.
	PAUSE
	GOTO:MainMenu

:Setup
call:ShortMenu
ECHO.
ECHO.
ECHO.                     Install Script - Setup Process
ECHO.
ECHO.
SET /P db_host=Please enter the IP / hostname of the database: 
SET /P db_user=Please enter the username with root access: 
SET /P db_pass=Please enter the password for the user: 
GOTO:MainMenu

:sleep -? waits some seconds before returning
::     -- %~1 ? in, number of seconds to wait
FOR /l %%a in (%~1,-1,1) do (ping -n 2 -w 1 127.0.0.1>NUL)
goto:EOF

:GetFileCount
set maxcnt=0
	for /F %%D IN ('dir /b "*.sql"') do (
		SET /A maxcnt += 1
	)
GOTO:EOF

:initProgress -- initialize an internal progress counter and display the progress in percent
::            -- %~1: in  - progress counter maximum, equal to 100 percent
::            -- %~2: in  - title string formatter, default is '[P] completed.'
set /a ProgressCnt=-1
set /a ProgressMax=%~1
set ProgressFormat=%~2
if "%ProgressFormat%"=="" set ProgressFormat=[PPPP]
set ProgressFormat=!ProgressFormat:[PPPP]=[P] completed.!
call :doProgress
GOTO:EOF


:doProgress -- display the next progress tick
set /a ProgressCnt+=1
SETLOCAL
set /a per=100*ProgressCnt/ProgressMax
set per=!per!%%
title %ProgressFormat:[P]=!per!%
GOTO:EOF

:end
GOTO:EOF