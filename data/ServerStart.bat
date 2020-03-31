@echo off

%~d0
cd %~dp0

start ConnectionServer.exe

start LoginServer.exe

start ChatServer.exe

start PingServer.exe

start ZoneServer.exe --ZoneName tutorial
start ZoneServer.exe --ZoneName tatooine
rem start ZoneServer.exe --ZoneName naboo
rem start ZoneServer.exe --ZoneName corellia
rem start ZoneServer.exe --ZoneName dantooine
rem start ZoneServer.exe --ZoneName dathomir
rem start ZoneServer.exe --ZoneName endor
rem start ZoneServer.exe --ZoneName lok
rem start ZoneServer.exe --ZoneName rori
rem start ZoneServer.exe --ZoneName talus
rem start ZoneServer.exe --ZoneName yavin4
rem start ZoneServer.exe --ZoneName taanab