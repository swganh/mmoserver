@echo off

%~d0
cd %~dp0

start ConnectionServer.exe
TIMEOUT /T 2
start LoginServer.exe
::TIMEOUT /T 10
::start ChatServer.exe
TIMEOUT /T 2
start PingServer.exe
TIMEOUT /T 2
start ZoneServer.exe --ZoneName tutorial
TIMEOUT /T 2
start ZoneServer.exe --ZoneName tatooine
rem TIMEOUT /T 2
rem start ZoneServer.exe --ZoneName naboo
rem TIMEOUT /T 2
rem start ZoneServer.exe --ZoneName corellia
rem TIMEOUT /T 2
rem start ZoneServer.exe --ZoneName dantooine
rem TIMEOUT /T 2
rem start ZoneServer.exe --ZoneName dathomir
rem TIMEOUT /T 2
rem start ZoneServer.exe --ZoneName endor
TIMEOUT /T 2
start ZoneServer.exe --ZoneName lok
rem TIMEOUT /T 2
rem start ZoneServer.exe --ZoneName rori
rem TIMEOUT /T 2
rem start ZoneServer.exe --ZoneName talus
rem TIMEOUT /T 2
rem start ZoneServer.exe --ZoneName yavin4
rem TIMEOUT /T 2
rem start ZoneServer.exe --ZoneName taanab
TIMEOUT /T 5
start ChatServer.exe