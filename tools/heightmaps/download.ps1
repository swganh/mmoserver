<#
.SYNOPSIS
Downloads planet heightmaps into data\heightmaps\<planet>.hmpw (Windows).

.DESCRIPTION
The Windows counterpart of download.sh. The archives are assets of the
swganh/mmoserver v0.6.0 release; each one is checked against SHA256SUMS
before it is unpacked. Every planet is ~150-230 MB to download and ~470 MB
unpacked. Uses curl.exe and tar.exe, which ship with Windows 10 and later.

.EXAMPLE
tools\heightmaps\download.ps1 tatooine lok
tools\heightmaps\download.ps1            # all planets
#>
[CmdletBinding()]
param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Planets)

$ErrorActionPreference = 'Stop'

$baseUrl = 'https://github.com/swganh/mmoserver/releases/download/v0.6.0'
$projectDir = (Resolve-Path "$PSScriptRoot\..\..").Path
$dest = Join-Path $projectDir 'data\heightmaps'
$allPlanets = 'corellia', 'dantooine', 'dathomir', 'endor', 'lok', 'naboo', 'rori', 'talus', 'tatooine', 'yavin4'

$checksums = @{}
foreach ($line in Get-Content (Join-Path $PSScriptRoot 'SHA256SUMS')) {
    $parts = $line -split '\s+', 2
    if ($parts.Count -eq 2) { $checksums[$parts[1].TrimStart('*')] = $parts[0].ToLowerInvariant() }
}

if (-not $Planets) { $Planets = $allPlanets }

foreach ($tool in 'curl.exe', 'tar.exe') {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool is required (included with Windows 10 and later)"
    }
}

New-Item -ItemType Directory -Force $dest | Out-Null
# Work inside the destination so the final move is a rename: an interrupted
# run never leaves a partial .hmpw behind that a later run would skip.
$tmp = Join-Path $dest ".download.$PID"
New-Item -ItemType Directory -Force $tmp | Out-Null

try {
    foreach ($planet in $Planets) {
        $planet = $planet.ToLowerInvariant()
        # Tatooine's archive is the only one named <planet>.hmpw.zip.
        $archive = if ($planet -eq 'tatooine') { 'tatooine.hmpw.zip' } else { "$planet.zip" }
        $expected = $checksums[$archive]
        if (-not $expected) { throw "unknown planet '$planet' (known: $($allPlanets -join ' '))" }

        if (Test-Path (Join-Path $dest "$planet.hmpw")) {
            Write-Host "${planet}: already present, skipping"
            continue
        }

        Write-Host "${planet}: downloading $archive"
        $archivePath = Join-Path $tmp $archive
        & curl.exe --fail --location --proto '=https' --tlsv1.2 --retry 3 --output $archivePath "$baseUrl/$archive"
        if ($LASTEXITCODE -ne 0) { throw "download of $archive failed" }

        $actual = (Get-FileHash -Algorithm SHA256 $archivePath).Hash.ToLowerInvariant()
        if ($actual -ne $expected) { throw "$archive checksum mismatch (expected $expected, got $actual)" }

        & tar.exe -xf $archivePath -C $tmp "$planet.hmpw"
        if ($LASTEXITCODE -ne 0) { throw "unpacking $archive failed" }
        Move-Item (Join-Path $tmp "$planet.hmpw") (Join-Path $dest "$planet.hmpw")
        Remove-Item $archivePath
        Write-Host "${planet}: ok"
    }
}
finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}
