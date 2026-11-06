@echo off
rem Builds the MMOServer on Windows. All the work is in build_server.ps1;
rem this wrapper only lets it run from cmd.exe or a double click.
rem Usage: build_server.bat [-Release] [-Clean] [-Test] [-Heightmaps tatooine,lok]
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_server.ps1" %*
exit /b %ERRORLEVEL%
