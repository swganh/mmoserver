@echo off
SETLOCAL EnableDelayedExpansion

if not exist %1 (
	goto :eof
)

cd %1

xcopy %1deps\glog\%2\libglog.dll %1bin /I /Y /s
xcopy %1deps\mysql\bin %1bin /I /Y /s
xcopy %1deps\mysql-connector-cpp\driver\%2 %1bin /I /Y /s

if %2 == "Debug" (
    if not exist %1bin\tbb_debug.dll (
        xcopy %1deps\tbb\bin\ia32\vc10\tbb_debug.dll %1bin /I /Y /s
        xcopy %1deps\tbb\bin\ia32\vc10\tbb_debug.pdb %1bin /I /Y /s
    )
)

if %2 == "Release" (    
    if not exist %1bin\tbb.dll (
        xcopy %1deps\tbb\bin\ia32\vc10\tbb.dll %1bin /I /Y /s
        xcopy %1deps\tbb\bin\ia32\vc10\tbb.pdb %1bin /I /Y /s
    )
)

if not exist %1bin\logs (
    mkdir %1bin\logs
)

%1tools\gitversion.exe > %1bin\VERSION

if not exist %1bin\heightmaps (
	if exist %1data\heightmaps (
		xcopy %1data\heightmaps\*.hmpw %1bin\heightmaps /I /Y /s
	)
)

if not exist %1bin\ServerStart.bat (
    xcopy %1data\ServerStart.bat %1bin /I /Y /s
)

ENDLOCAL