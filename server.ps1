<#
.SYNOPSIS
Starts, stops and checks the MMOServer processes of a build made with
build_server.ps1 (Windows).

.DESCRIPTION
The Windows counterpart of server.sh.

  start [zone ...]   Start the Connection, Chat, Login and Ping servers, then
                     the given zones (default: tutorial tatooine). Output goes
                     to build\<preset>\bin\logs\<server>.out
  stop               Stop every server started by this script
  status             Show which servers are running and each log's last line
  restart [zone ...] stop, then start

With -Console each server runs in its own console window instead, showing its
output live (nothing is written to logs\). To debug a server, start the others
with this script and run that one from Visual Studio (see docs\WINDOWS.md).

Zones other than tutorial need their heightmap: .\build_server.ps1 -Heightmaps <zone>

.EXAMPLE
.\server.ps1 start tutorial tatooine lok
.\server.ps1 status
.\server.ps1 stop
#>
[CmdletBinding()]
param(
    [Parameter(Position = 0)][ValidateSet('start', 'stop', 'status', 'restart')][string]$Command = 'status',
    [Parameter(Position = 1, ValueFromRemainingArguments = $true)][string[]]$Zones,
    [switch]$Release,
    [switch]$Console
)

$ErrorActionPreference = 'Stop'
$preset = if ($Release) { 'windows-release' } else { 'windows-debug' }
$binDir = Join-Path $PSScriptRoot "build\$preset\bin"
$runDir = Join-Path $binDir 'run'
$logDir = Join-Path $binDir 'logs'

# Order matters: the other servers register with the ConnectionServer.
$coreServers = 'connection', 'chat', 'login', 'ping'
$executables = @{ connection = 'ConnectionServer'; chat = 'ChatServer'; login = 'LoginServer'; ping = 'PingServer' }

function Get-Executable([string]$Name) {
    if ($executables.ContainsKey($Name)) { $executables[$Name] } else { 'ZoneServer' }
}

# The process of a server started by this script, or $null.
function Get-ServerProcess([string]$Name) {
    $pidFile = Join-Path $runDir "$Name.pid"
    if (-not (Test-Path $pidFile)) { return $null }
    $process = Get-Process -Id ([int](Get-Content $pidFile)) -ErrorAction SilentlyContinue
    # Only trust the pid if it still belongs to one of our servers.
    if ($process -and $process.ProcessName -eq (Get-Executable $Name)) { return $process }
    return $null
}

function Get-KnownServers {
    if (-not (Test-Path $runDir)) { return @() }
    $zones = Get-ChildItem $runDir -Filter *.pid | ForEach-Object { $_.BaseName } | Where-Object { $coreServers -notcontains $_ }
    @($coreServers) + @($zones | Sort-Object)
}

function Start-Server([string]$Name) {
    $exe = Get-Executable $Name
    $process = Get-ServerProcess $Name
    if ($process) {
        Write-Host "${Name}: already running (pid $($process.Id))"
        return
    }
    if ($exe -eq 'ZoneServer' -and $Name -ne 'tutorial' -and -not (Test-Path (Join-Path $binDir "heightmaps\$Name.hmpw"))) {
        Write-Warning "${Name}: skipped, no heightmap (.\build_server.ps1 -Heightmaps $Name)"
        return
    }

    $arguments = @{
        FilePath         = Join-Path $binDir "$exe.exe"
        WorkingDirectory = $binDir
        PassThru         = $true
    }
    if ($exe -eq 'ZoneServer') { $arguments.ArgumentList = @('--ZoneName', $Name) }

    if ($Console) {
        $process = Start-Process @arguments
        Write-Host "${Name}: started in its own window (pid $($process.Id))"
    }
    else {
        $log = Join-Path $logDir "$Name.out"
        # Keep the previous run's log: it may hold the reason that run ended.
        foreach ($file in $log, "$log.err") {
            if ((Test-Path $file) -and (Get-Item $file).Length -gt 0) { Move-Item -Force $file "$file.1" }
        }
        $process = Start-Process @arguments -WindowStyle Hidden `
            -RedirectStandardOutput $log -RedirectStandardError "$log.err"
        Write-Host "${Name}: started (pid $($process.Id), log logs\$Name.out)"
    }
    Set-Content -Path (Join-Path $runDir "$Name.pid") -Value $process.Id
}

function Stop-Servers {
    $servers = @(Get-KnownServers)
    # Zones first, the ConnectionServer last.
    [array]::Reverse($servers)
    foreach ($name in $servers) {
        $process = Get-ServerProcess $name
        if ($process) {
            Stop-Process -Id $process.Id -Force
            Write-Host "${name}: stopped"
        }
        Remove-Item -Force (Join-Path $runDir "$name.pid") -ErrorAction SilentlyContinue
    }
}

function Show-Status {
    $servers = @(Get-KnownServers)
    if ($servers.Count -eq 0) { Write-Host 'No servers started by this script.'; return }
    foreach ($name in $servers) {
        $process = Get-ServerProcess $name
        $state = if ($process) { "running (pid $($process.Id))" } else { 'not running' }
        $last = ''
        $log = Join-Path $logDir "$name.out"
        if (Test-Path $log) { $last = Get-Content $log -Tail 1 -ErrorAction SilentlyContinue }
        Write-Host ('{0,-12} {1,-22} {2}' -f $name, $state, $last)
    }
}

function Start-Servers {
    if (-not (Test-Path (Join-Path $binDir 'ConnectionServer.exe'))) { throw "No build in $binDir (run .\build_server.ps1)" }
    if (-not (Test-Path (Join-Path $binDir 'config\general.cfg'))) { throw "$binDir\config missing (run .\build_server.ps1)" }
    New-Item -ItemType Directory -Force $runDir, $logDir | Out-Null

    $zones = if ($Zones) { $Zones } else { @('tutorial', 'tatooine') }
    foreach ($zone in $zones) {
        if (-not (Test-Path (Join-Path $binDir "config\$zone.cfg"))) { throw "unknown zone '$zone' (no config\$zone.cfg)" }
    }
    foreach ($name in @($coreServers) + @($zones)) {
        Start-Server $name
        Start-Sleep -Seconds 2
    }
    Write-Host ''
    Write-Host 'Zones take a minute or two to load. Check with: .\server.ps1 status'
}

switch ($Command) {
    'start' { Start-Servers }
    'stop' { Stop-Servers }
    'status' { Show-Status }
    'restart' { Stop-Servers; Start-Servers }
}
