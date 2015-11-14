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

SET "PROJECT_BASE=%~dp0"

::SET PROJECT_BASE=----
:: Set the window title 
SET title=%~n0
TITLE %title%
Project Base: %PROJECT_BASE%
::
:: Start
::

@ECHO OFF
	call:ScreenIntro
	CALL :READ_CFG
	::call:Setup
	call:MainMenu

	::
	:: Functions
	::
:READ_CFG
	FOR /F "tokens=2 delims==" %%a IN ('find "username" ^< setup.cfg') DO SET db_user=%%a
	FOR /F "tokens=2 delims==" %%a IN ('find "password" ^< setup.cfg') DO SET db_pass=%%a
	FOR /F "tokens=2 delims==" %%a IN ('find "host" ^< setup.cfg') DO SET db_host=%%a
	
	GOTO :EOF
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
	ECHO.                     SWG:ANH ^| Database Installer ^&
	ECHO.                             ^| Administration tool
	ECHO.
	CALL:sleep 5
	GOTO:EOF

:ShortMenu
	CLS
	ECHO. ----------------------------------------------------------------------
	ECHO.  SWGANH Database Install Script                              (v.0.03)
	ECHO. ----------------------------------------------------------------------
	ECHO.  DB IP: %db_host%     DB Username: %db_user%    DB Password: %db_pass%
	ECHO. ----------------------------------------------------------------------
	ECHO.
	ECHO.
	GOTO:EOF

:MainMenu
	CLS
	ECHO. ----------------------------------------------------------------------
	ECHO.  SWGANH Database Install Script                              (v.0.03)
	ECHO. ----------------------------------------------------------------------
	ECHO.  DB IP: %db_host%     DB Username: %db_user%    DB Password: %db_pass%
	ECHO. ----------------------------------------------------------------------
	ECHO.                                   ^|
	ECHO.           Database Setup          ^|       Database Maintenance
	ECHO.                                   ^|
	ECHO.   (0) First time install          ^|  (a) Complete Database Backup
	ECHO.   (1) Complete DB Install         ^|  (b) Check DB Version  
	ECHO.   (2) Setup Databases ^& Users     ^|  (c) Shift Resources
	ECHO.   (3) Setup Main Database         ^|  (d) Remove Databases
	ECHO.   (4) Setup Stored Procedures     ^|
	ECHO.   (5) Setup Stored Functions      ^|          Script Settings 
	ECHO.   (6) Setup tools                 ^|  
	ECHO.                                   ^|  (e) Username Change
	ECHO.   (7) Setup AstroMech Scripts     ^|  (f) Password Change
	ECHO.                                   ^|  (g) IP Change
	ECHO.        Server Configuration       ^|
	ECHO.                                   ^|
	ECHO.   (8) Generate Resources          ^|
	ECHO.   (9) Generate Bots               ^|               Help
	ECHO.   (r) Reset GlobalTimer           ^|
	ECHO.   (m) Change / Set MotD           ^|  (h) Help
	ECHO.   (i) Change galaxy IP            ^|  (s) Stats
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO.                                   ^|
	ECHO. ----------------------------------------------------------------------
	SET /P Choice=Make a choice or ^(q^) quit : 

if /I '%Choice%'=='0' GOTO :FirstTime

IF /I '%Choice%'=='1' GOTO :DatabaseCompleteSetup 

IF /I '%Choice%'=='2' GOTO :DatabaseSetup 

IF /I '%Choice%'=='3' GOTO :SchemaSetup

IF /I '%Choice%'=='4' GOTO :ProcedureSetup

IF /I '%Choice%'=='5' GOTO :FunctionSetup

IF /I '%Choice%'=='6' GOTO :ToolsSetup

IF /I '%Choice%'=='7' GOTO :AstroMechSetup

IF /I '%Choice%'=='8' GOTO :GenerateResources

IF /I '%Choice%'=='9' GOTO :GenerateBots

IF /I '%Choice%'=='r' GOTO :ResetGlobalTimer

IF /I '%Choice%'=='m' GOTO :SetMOTD

If /I '%Choice%'=='i' GOTO :ChangeServerIP

IF /I '%Choice%'=='a' GOTO :CompleteBackup 

IF /I '%Choice%'=='b' GOTO :CHECKDBVERSION

IF /I '%Choice%'=='c' GOTO :SHIFT_RESOURCES

IF /I '%Choice%'=='d' GOTO :DatabaseRemove

IF /I '%Choice%'=='e' GOTO :UserChange 

IF /I '%Choice%'=='f' GOTO :PassChange 

IF /I '%Choice%'=='g' GOTO :IPChange

IF /I '%Choice%'=='h' GOTO :Help 

IF /I '%Choice%'=='s' GOTO :Stats

IF /I '%Choice%'=='Q' GOTO :end 

GOTO:MainMenu

:FirstTime
  call:ShortMenu
  call:UserChange
  call:PassChange
	ECHO.
	ECHO.
	ECHO.
	ECHO.                      Sorry not in use at this time
	ECHO.
	ECHO.                           [Please Wait]
	cd ..
	call:sleep 5
  GOTO:MainMenu
:DatabaseCompleteSetup 
	call:ShortMenu
  
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
		cd "%PROJECT_BASE%swganh_tools"
		mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
				)
				
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
	cd "%PROJECT_BASE%swganh\scripts"
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	
		
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
	cd "%PROJECT_BASE%swganh_tools\scripts"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		cd "%PROJECT_BASE%"				
	)
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
	call:initProgress %maxcnt%
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
		
	call:doProgress
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
	cd "%PROJECT_BASE%swganh"
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
cd "%PROJECT_BASE%swganh\scripts"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
	cd "%PROJECT_BASE%"				

	
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
	cd "%PROJECT_BASE%"
	
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
			
		
		cd "%PROJECT_BASE%swganh\procedures"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		
		::cd "%PROJECT_BASE%swganh_archive\procedures"
		::for /F %%A IN ('dir /b "*.sql" ^| sort') do (
		::mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		::ECHO. Installing %%A [Done]
		::)
		
		cd "%PROJECT_BASE%swganh_astromech\procedures"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
	
		cd "%PROJECT_BASE%swganh_config\procedures"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)

		::cd "%PROJECT_BASE%swganh_logs\procedures"
		::for /F %%A IN ('dir /b "*.sql"') do (
		::mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		::ECHO. Installing %%A [Done]
		::call:doProgress
		::)
	
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
	cd "%PROJECT_BASE%
		call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:sleep 2
	call:initProgress %maxcnt%
	cd "%PROJECT_BASE%swganh\functions"
		for /F %%A IN ('dir /b "*.sql" ^| sort') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		
		::cd "%PROJECT_BASE%swganh_archive\functions"
		::for /F %%A IN ('dir /b "*.sql" ^| sort') do (
		::	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		::ECHO. Installing %%A [Done]
		::)
		
		cd "%PROJECT_BASE%swganh_astromech\functions"
		for /F %%A IN ('dir /b "*.sql"') do (
			mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		ECHO. Installing %%A [Done]
		)
		
		::cd "%PROJECT_BASE%swganh_config\functions"
		::for /F %%A IN ('dir /b "*.sql"') do (
		::	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		::ECHO. Installing %%A [Done]
		::)
		
		::cd "%PROJECT_BASE%swganh_logs\functions"
		::for /F %%A IN ('dir /b "*.sql"') do (
		::	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "%%A"
		::ECHO. Installing %%A [Done]
		::)
		call:doProgress


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
	
:ToolsSetup
	call:ShortMenu
	cd "%PROJECT_BASE%swganh_tools"
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
	cd "%PROJECT_BASE%swganh_tools"
      mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 < "create.sql"
				)
				cd scripts"
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
	ECHO.                    Tool Scripts Load Complete
	call:sleep 5
	GOTO:MainMenu

:AstroMechSetup
	call:ShortMenu
	cd "%PROJECT_BASE%swganh_astromech"
	call:GetFileCount
	ECHO.                Starting installation for %maxcnt% files...
	ECHO.
	ECHO.
	call:initProgress %maxcnt%
	
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
	mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh --execute="CALL sp_CreateSWGANHBots(20, 001, '');"
	ECHO.
	ECHO.
	ECHO.          20 bots and their accounts have been generated in bestine.
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
:ChangeServerIP
 call:shortMenu
 SET /P address=Enter new IP address: 
 mysql --password=%db_pass% --host=%db_host% --user=%db_user% --default-character-set=utf8 --database=swganh --execute="UPDATE galaxy SET address ='%address%';"
 ECHO.
	ECHO.
	ECHO.                  The new server IP address is:
	ECHO.
	ECHO.  %address%
	ECHO.
	call:sleep 5
	GOTO:MainMenu
:CompleteBackup
	call:ShortMenu
	mkdir backup
	cd backup
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.                   Backing up the swganh database
	
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
 
    )
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
	 
:CHECKDBVERSION
	call:ShortMenu
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
	
 ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.
	ECHO.                           Version Check Compleate
	call:sleep 5
	GOTO:MainMenu
:SHIFT_RESOURCES
call:ShortMenu
	ECHO.
	ECHO.                         Shifting Resources
	ECHO.
	
	ECHO. SHIFT RESOURCES CALLED
	
	
  call:sleep
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
	cd "%PROJECT_BASE%swganh\scripts"
	call:GetFileCount
	cd ..
	
	ECHO.  Number of SQL scripts for the main schema is  --------^> %maxcnt%
	ECHO.
	set maxcnt=0
	
	cd "%PROJECT_BASE%swganh\procedures"
	call:GetFileCount
	cd ..
	cd ..
	ECHO.  Number of Stored Procedures for the main schema is  --^> %maxcnt%
	ECHO.
	set maxcnt=0
	
	cd "%PROJECT_BASE%swganh\functions"
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