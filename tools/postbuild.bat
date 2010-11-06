@echo off
SETLOCAL EnableDelayedExpansion

if not exist %1 (
    goto :eof
)

cd %1

if not exist %2\bin\%3\libglog.dll (
    xcopy %1\deps\glog\%3\libglog.dll %2\bin\%3 /I /Y /s    
)

if not exist %2\bin\%3\libmysql.dll (
    xcopy %1\deps\mysql\bin\libmysql.dll %2\bin\%3 /I /Y /s    
)

if not exist %2\bin\%3\mysqlcppconn.dll (
    xcopy %1\deps\mysql-connector-cpp\driver\%3\mysqlcppconn.dll %2\bin\%3 /I /Y /s    
)

if %3 == "Debug" (
    if not exist %2\bin\%3\spatialindex_d.dll (
        xcopy %1\deps\spatialindex\spatialindex-vc\Debug\spatialindex_d.dll %2\bin\%3 /I /Y /s  
    )
    
    if not exist %2\bin\%3\tbb_debug.dll (
        xcopy %1\deps\tbb\bin\ia32\vc10\tbb_debug.dll %2\bin\%3 /I /Y /s  
        xcopy %1\deps\tbb\bin\ia32\vc10\tbbmalloc_debug.dll %2\bin\%3 /I /Y /s      
    )    
)

if %3 == "Release" (
    if not exist %2\bin\%3\spatialindex.dll (
        xcopy %1\deps\spatialindex\spatialindex-vc\Release\spatialindex.dll %2\bin\%3 /I /Y /s  
    )
    
    if not exist %2\bin\%3\tbb.dll (
        xcopy %1\deps\tbb\bin\ia32\vc10\tbb.dll %2\bin\%3 /I /Y /s  
        xcopy %1\deps\tbb\bin\ia32\vc10\tbbmalloc.dll %2\bin\%3 /I /Y /s      
    )    
)

if not exist %2\bin\%3\config (
    xcopy %1\data\config %2\bin\%3\config /I /Y /s    
)

if not exist %2\bin\%3\heightmaps (
    if exist %1\data\heightmaps (
        xcopy %1\data\heightmaps\*.hmpw %2\bin\%3\heightmaps /I /Y /s
    )    
)

if not exist %2\bin\%3\logs (
    mkdir %2\bin\%3\logs
)

if not exist %2\bin\%3\script (
    xcopy %1\data\script %2\bin\%3\script /I /Y /s    
)

if not exist %2\bin\%3\ServerStart.bat (
    xcopy %1\data\ServerStart.bat %2\bin\%3 /I /Y /s
)

ENDLOCAL