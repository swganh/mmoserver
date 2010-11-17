@echo off

%~d0
cd %~dp0

start ConnectionServer.exe

start LoginServer.exe

start ChatServer.exe

start PingServer.exe

start ZoneServer.exe tutorial
start ZoneServer.exe tatooine
rem start ZoneServer.exe naboo
rem start ZoneServer.exe corellia
rem start ZoneServer.exe dantooine
rem start ZoneServer.exe dathomir
rem start ZoneServer.exe endor
rem start ZoneServer.exe lok
rem start ZoneServer.exe rori
rem start ZoneServer.exe talus
rem start ZoneServer.exe yavin4
rem start ZoneServer.exe taanab
