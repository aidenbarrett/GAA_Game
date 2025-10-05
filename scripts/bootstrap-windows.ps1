param(
    [switch]$Gui,
    [switch]$Package,
    [switch]$Run
)

$ErrorActionPreference = "Stop"

function Write-Section {
    param([string]$Message)
    Write-Host "`n=== $Message ===" -ForegroundColor Cyan
}

function Resolve-RepoRoot {
    $scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
    return (Resolve-Path (Join-Path $scriptPath ".."))
}

function Get-PortableCMake {
    param([string]$Version = "3.29.2")

    $root = Resolve-RepoRoot
    $toolsDir = Join-Path $root "tools"
    $cmakeRoot = Join-Path $toolsDir "cmake-$Version-windows-x86_64"
    $cmakeExe = Join-Path $cmakeRoot "bin/cmake.exe"

    if (Test-Path $cmakeExe) {
        return $cmakeExe
    }

    Write-Section "Downloading portable CMake $Version"
    if (-not (Test-Path $toolsDir)) {
        New-Item -ItemType Directory -Path $toolsDir | Out-Null
    }

    $zipPath = Join-Path $toolsDir "cmake-$Version.zip"
    $uri = "https://github.com/Kitware/CMake/releases/download/v$Version/cmake-$Version-windows-x86_64.zip"

    Write-Host "Fetching $uri" -ForegroundColor Yellow
    Invoke-WebRequest -Uri $uri -OutFile $zipPath

    Write-Host "Extracting archive..." -ForegroundColor Yellow
    Expand-Archive -Path $zipPath -DestinationPath $toolsDir -Force
    Remove-Item $zipPath

    if (-not (Test-Path $cmakeExe)) {
        throw "Unable to locate cmake.exe after extraction"
    }

    return $cmakeExe
}

function Resolve-CMake {
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmake) {
        return $cmake.Source
    }

    Write-Host "CMake not found in PATH. Falling back to portable bundle." -ForegroundColor Yellow
    return Get-PortableCMake
}

function Invoke-CMake {
    param(
        [string[]]$Arguments
    )

    $cmakeExe = Resolve-CMake
    Write-Host "Running: $cmakeExe $($Arguments -join ' ')" -ForegroundColor DarkGray
    & $cmakeExe @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "CMake command failed ($LASTEXITCODE). Ensure the Visual Studio Build Tools with the C++ workload are installed."
    }
}

$repoRoot = Resolve-RepoRoot
Set-Location $repoRoot

$configurePreset = if ($Gui) { "windows-gui-release" } else { "windows-release" }
$buildPreset = $configurePreset

Write-Section "Configuring ($configurePreset)"
Invoke-CMake -Arguments @("--preset", $configurePreset)

Write-Section "Building"
Invoke-CMake -Arguments @("--build", "--preset", $buildPreset)

if ($Package) {
    Write-Section "Packaging"
    Invoke-CMake -Arguments @("--build", "--preset", $buildPreset, "--target", "package")
}

if ($Run) {
    Write-Section "Launching"
    $binaryDir = Join-Path $repoRoot "build/windows-release"
    $exeName = if ($Gui) { "GaelicClashGUI.exe" } else { "GaelicClash.exe" }
    $candidatePaths = @(
        Join-Path $binaryDir "Release/$exeName",
        Join-Path $binaryDir $exeName
    )
    $exe = $candidatePaths | Where-Object { Test-Path $_ } | Select-Object -First 1

    if (-not $exe) {
        throw "Could not locate built executable in $binaryDir"
    }

    Write-Host "Starting $exe" -ForegroundColor Green
    & $exe
}
