<#
.SYNOPSIS
Builds the MMOServer on Windows using the CMake presets in CMakePresets.json.

.DESCRIPTION
The Windows counterpart of build_server.sh. Finds Visual Studio 2022 or later
(with the "Desktop development with C++" workload), sets up its x64
developer environment, builds the dependencies with vcpkg and the servers
with the windows-debug or windows-release preset. Each build stages config\,
script\, logs\ and heightmaps\ next to the binaries in build\<preset>\bin.

vcpkg: uses $env:VCPKG_ROOT, else the copy that ships with Visual Studio,
else clones it into build\vcpkg.

.PARAMETER Release
Build the windows-release preset (RelWithDebInfo) instead of windows-debug.

.PARAMETER Clean
Delete the preset's build directory first.

.PARAMETER Test
Run the unit tests after building.

.PARAMETER KeepGoing
Keep compiling after errors, to see all of them at once.

.PARAMETER Heightmaps
Download heightmaps first: planet names, or 'all'. Example: -Heightmaps tatooine,lok

.PARAMETER Solution
Generate a Visual Studio solution (build\windows-vs\mmoserver.sln, or .slnx
for Visual Studio 2026) instead of
building with Ninja, then build its Debug (or -Release) configuration. Open
the solution in Visual Studio to build, run and debug from there.

.EXAMPLE
.\build_server.ps1 -Test -Heightmaps tatooine
.\build_server.ps1 -Solution
#>
[CmdletBinding()]
param(
    [switch]$Release,
    [switch]$Clean,
    [switch]$Test,
    [switch]$KeepGoing,
    [string[]]$Heightmaps,
    [switch]$Solution
)

$ErrorActionPreference = 'Stop'
$basedir = $PSScriptRoot
$preset = if ($Release) { 'windows-release' } else { 'windows-debug' }
$config = if ($Release) { 'RelWithDebInfo' } else { 'Debug' }
$buildDir = if ($Solution) { Join-Path $basedir 'build\windows-vs' } else { Join-Path $basedir "build\$preset" }

function Invoke-Checked {
    param([string]$Exe, [string[]]$Arguments)
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe $($Arguments -join ' ') failed (exit code $LASTEXITCODE)" }
}

# --- Visual Studio developer environment ----------------------------------------
# vcpkg builds the libraries with the newest MSVC in the newest Visual Studio.
# The servers must be built with the same one: a static library from a newer
# MSVC fails to link with an older one ("unresolved external symbol
# __std_search_1"). Developer prompts can be set to an older MSVC installed
# side by side, so pick the toolset explicitly.
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) { throw 'Visual Studio 2022 or later is required (vswhere.exe not found)' }
$vsPath = & $vswhere -latest -products * -version '[17.0,)' `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    throw 'No Visual Studio 2022 or later with the C++ x64 tools found. Install the "Desktop development with C++" workload.'
}
$vsMajor = [int]((& $vswhere -path $vsPath -property installationVersion) -split '\.')[0]
$msvcVersion = Get-ChildItem (Join-Path $vsPath 'VC\Tools\MSVC') -Directory |
    Where-Object { Test-Path (Join-Path $_.FullName 'bin\Hostx64\x64\cl.exe') } |
    Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1 -ExpandProperty Name
if (-not $msvcVersion) { throw "No MSVC x64 compiler found in $vsPath" }

if ($env:VCToolsVersion -ne $msvcVersion -or $env:VSINSTALLDIR -ne "$vsPath\") {
    # The developer environment points VCPKG_ROOT at Visual Studio's own
    # vcpkg; keep one that was chosen before.
    $chosenVcpkg = $env:VCPKG_ROOT
    Import-Module (Join-Path $vsPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
    Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation `
        -DevCmdArguments "-arch=x64 -host_arch=x64 -vcvars_ver=$msvcVersion" | Out-Null
    if ($chosenVcpkg) { $env:VCPKG_ROOT = $chosenVcpkg }
}
$env:VCPKG_VISUAL_STUDIO_PATH = $vsPath
Write-Host "Using Visual Studio at $vsPath, MSVC $msvcVersion"
foreach ($tool in 'cl.exe', 'cmake.exe', 'ninja.exe') {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool not found in the Visual Studio developer environment. Install the `"C++ CMake tools for Windows`" component."
    }
}

# --- vcpkg ------------------------------------------------------------------------
if (-not $env:VCPKG_ROOT -or -not (Test-Path (Join-Path $env:VCPKG_ROOT 'scripts\buildsystems\vcpkg.cmake'))) {
    $env:VCPKG_ROOT = Join-Path $basedir 'build\vcpkg'
    if (-not (Test-Path $env:VCPKG_ROOT)) {
        Write-Host "Cloning vcpkg into $env:VCPKG_ROOT"
        Invoke-Checked git @('clone', 'https://github.com/microsoft/vcpkg.git', $env:VCPKG_ROOT)
    }
}
if (-not (Test-Path (Join-Path $env:VCPKG_ROOT 'vcpkg.exe'))) {
    Invoke-Checked (Join-Path $env:VCPKG_ROOT 'bootstrap-vcpkg.bat') @('-disableMetrics')
}
Write-Host "Using vcpkg at $env:VCPKG_ROOT"

# --- Heightmaps -------------------------------------------------------------------
if ($Heightmaps) {
    $planets = @($Heightmaps | ForEach-Object { $_ -split ',' } | Where-Object { $_ })
    if ($planets -contains 'all') { $planets = @() }
    & (Join-Path $basedir 'tools\heightmaps\download.ps1') @planets
}

# --- Build ------------------------------------------------------------------------
if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "Removing $buildDir"
    Remove-Item -Recurse -Force $buildDir
}
# A cache made with another compiler keeps using it: start that one again.
$cache = Join-Path $buildDir 'CMakeCache.txt'
if ((Test-Path $cache) -and -not (Select-String -Path $cache -SimpleMatch @("MSVC/$msvcVersion/", "version=$msvcVersion") -Quiet)) {
    Write-Host "Reconfiguring $buildDir for MSVC $msvcVersion"
    Remove-Item -Force $cache
    Remove-Item -Recurse -Force (Join-Path $buildDir 'CMakeFiles') -ErrorAction SilentlyContinue
}

Push-Location $basedir
try {
    if ($Solution) {
        $generator = switch ($vsMajor) {
            17 { 'Visual Studio 17 2022' }
            18 { 'Visual Studio 18 2026' }
            default { throw "No CMake generator known for Visual Studio $vsMajor" }
        }
        Invoke-Checked cmake @('-S', '.', '-B', $buildDir, '-G', $generator, '-A', 'x64',
            '-T', "version=$msvcVersion",
            "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake",
            '-DVCPKG_TARGET_TRIPLET=x64-windows-static-md',
            "-DVCPKG_INSTALLED_DIR=$basedir\build\vcpkg_installed")
        Invoke-Checked cmake @('--build', $buildDir, '--config', $config, '--', '/m')
        if ($Test) { Invoke-Checked ctest @('--test-dir', $buildDir, '-C', $config, '--output-on-failure') }
    }
    else {
        Invoke-Checked cmake @('--preset', $preset)
        $buildArguments = @('--build', '--preset', $preset)
        if ($KeepGoing) { $buildArguments += @('--', '-k', '0') }
        Invoke-Checked cmake $buildArguments
        if ($Test) { Invoke-Checked ctest @('--preset', $preset) }
    }
}
finally {
    Pop-Location
}

$binDir = if ($Solution) { "$buildDir\bin\$config" } else { "$buildDir\bin" }
Write-Host ''
Write-Host "Build complete: $binDir"
if ($Solution) {
    # Visual Studio 2026 solutions are .slnx, 2022 ones .sln.
    $sln = Get-ChildItem $buildDir -Filter 'mmoserver.sln*' | Select-Object -First 1
    Write-Host "Solution: $($sln.FullName)"
}
Write-Host "Database settings are in $binDir\config\*.cfg (see docs\WINDOWS.md)."
