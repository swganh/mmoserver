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

.EXAMPLE
.\build_server.ps1 -Test -Heightmaps tatooine
#>
[CmdletBinding()]
param(
    [switch]$Release,
    [switch]$Clean,
    [switch]$Test,
    [switch]$KeepGoing,
    [string[]]$Heightmaps
)

$ErrorActionPreference = 'Stop'
$basedir = $PSScriptRoot
$preset = if ($Release) { 'windows-release' } else { 'windows-debug' }
$buildDir = Join-Path $basedir "build\$preset"

function Invoke-Checked {
    param([string]$Exe, [string[]]$Arguments)
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe $($Arguments -join ' ') failed (exit code $LASTEXITCODE)" }
}

# --- Visual Studio developer environment ----------------------------------------
# Visual Studio's own CMake support and its developer prompts already provide
# it (they set VSINSTALLDIR); otherwise find the newest install with the C++
# tools via vswhere. A cl.exe that is merely on PATH is not enough: it may be
# an older MSVC than the one vcpkg builds the libraries with.
if (-not $env:VSINSTALLDIR -or -not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) { throw 'Visual Studio 2022 or later is required (vswhere.exe not found)' }
    $vsPath = & $vswhere -latest -products * -version '[17.0,)' `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsPath) {
        throw 'No Visual Studio 2022 or later with the C++ x64 tools found. Install the "Desktop development with C++" workload.'
    }
    Write-Host "Using Visual Studio at $vsPath"
    # The developer environment points VCPKG_ROOT at Visual Studio's own
    # vcpkg; keep one that was chosen before.
    $chosenVcpkg = $env:VCPKG_ROOT
    Import-Module (Join-Path $vsPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
    Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null
    if ($chosenVcpkg) { $env:VCPKG_ROOT = $chosenVcpkg }
}
# vcpkg picks a Visual Studio by itself, which can be a different (newer)
# one than this environment's when several are installed; libraries built
# by a newer compiler then fail to link with "unresolved external symbol
# __std_...". Make vcpkg use this environment's Visual Studio.
if ($env:VSINSTALLDIR) {
    $env:VCPKG_VISUAL_STUDIO_PATH = $env:VSINSTALLDIR.TrimEnd('\')
}
if ($env:VCToolsVersion) { Write-Host "Using MSVC $env:VCToolsVersion" }
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

Push-Location $basedir
try {
    Invoke-Checked cmake @('--preset', $preset)
    $buildArguments = @('--build', '--preset', $preset)
    if ($KeepGoing) { $buildArguments += @('--', '-k', '0') }
    Invoke-Checked cmake $buildArguments
    if ($Test) { Invoke-Checked ctest @('--preset', $preset) }
}
finally {
    Pop-Location
}

Write-Host ''
Write-Host "Build complete: $buildDir\bin"
Write-Host "Database settings are in $buildDir\bin\config\*.cfg (see docs\WINDOWS.md)."
