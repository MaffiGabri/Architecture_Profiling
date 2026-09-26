<#
.SYNOPSIS
    Builds and tests the Architecture Profiling Windows Companion Application.
    Satisfies Acceptance Criterion AC2 ("Il progetto Windows si compila correttamente senza errori").

.DESCRIPTION
    Automated configuration, compilation, and testing wrapper for Windows:
    - Auto-detects CMake and CTest in PATH or Python Scripts / Program Files / local tools.
    - Auto-detects C++20 toolchains (MinGW-w64 / w64devkit, MSVC, Clang) in PATH or standard directories.
    - Auto-detects Ninja or chooses appropriate CMake generator.
    - Configures project via: cmake -B <build_dir> -S <source_dir> -DCMAKE_BUILD_TYPE=<config>
    - Compiles project via: cmake --build <build_dir> --config <config>
    - Executes test suite via: ctest --test-dir <build_dir> --output-on-failure -C <config>

.PARAMETER Config
    Build configuration: "Release" (default), "Debug", "RelWithDebInfo", "MinSizeRel".

.PARAMETER Clean
    Wipes the build directory before configuring.

.PARAMETER SkipTest
    Skips the ctest test execution phase.

.PARAMETER Generator
    Explicitly overrides the CMake generator (e.g. "Ninja", "MinGW Makefiles", "Visual Studio 17 2022").

.PARAMETER CompilerDir
    Explicit path to compiler bin directory (e.g. "<repo_root>\tools\w64devkit\bin").

.PARAMETER CMakeDir
    Explicit path to directory containing cmake.exe.

.PARAMETER BuildDir
    Explicit build directory path (default: <repo_root>\build).

.PARAMETER SourceDir
    Explicit source directory path (default: <repo_root>\windows).

.PARAMETER DryRun
    Inspects environment, resolves tools, prints commands without executing.
#>

[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [ValidateSet("Release", "Debug", "RelWithDebInfo", "MinSizeRel")]
    [string]$Config = "Release",

    [Parameter()]
    [switch]$Clean,

    [Parameter()]
    [switch]$SkipTest,

    [Parameter()]
    [string]$Generator = "",

    [Parameter()]
    [string]$CompilerDir = "",

    [Parameter()]
    [string]$CMakeDir = "",

    [Parameter()]
    [string]$BuildDir = "",

    [Parameter()]
    [string]$SourceDir = "",

    [Parameter()]
    [switch]$DryRun,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraCMakeArgs
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host "`n>>> $Message" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Message)
    Write-Host "[OK] $Message" -ForegroundColor Green
}

function Write-Warn {
    param([string]$Message)
    Write-Host "[WARN] $Message" -ForegroundColor Yellow
}

function Write-Fail {
    param([string]$Message)
    Write-Host "[FAIL] $Message" -ForegroundColor Red
}

Write-Host "================================================================================" -ForegroundColor Cyan
Write-Host " Architecture Profiling Companion - Windows Build & Test Runner (AC2)" -ForegroundColor Cyan
Write-Host "================================================================================" -ForegroundColor Cyan

# -----------------------------------------------------------------------------
# 1. Resolve Project Paths
# -----------------------------------------------------------------------------
$ScriptPath = $MyInvocation.MyCommand.Path
if ($PSScriptRoot) {
    $ScriptDir = $PSScriptRoot
} elseif ($ScriptPath) {
    $ScriptDir = Split-Path -Parent $ScriptPath
} else {
    $ScriptDir = (Get-Location).Path
}

# Locate repo root by searching for ORIGINAL_REQUEST.md
$CurrentProbe = [System.IO.Path]::GetFullPath($ScriptDir)
$ProjectRoot = $null
while ($CurrentProbe -and (Test-Path $CurrentProbe)) {
    $LeafName = Split-Path -Leaf $CurrentProbe
    if ($LeafName -ne ".agents") {
        if ((Test-Path (Join-Path $CurrentProbe ".git")) -or ((Test-Path (Join-Path $CurrentProbe "ORIGINAL_REQUEST.md")) -and (Test-Path (Join-Path $CurrentProbe "shared")))) {
            $ProjectRoot = $CurrentProbe
            break
        }
    }
    $ParentProbe = Split-Path -Parent $CurrentProbe
    if ($ParentProbe -eq $CurrentProbe) { break }
    $CurrentProbe = $ParentProbe
}

if (-not $ProjectRoot) {
    $ProjectRoot = [System.IO.Path]::GetFullPath((Join-Path $ScriptDir ".."))
}

if (-not $SourceDir) {
    $SourceDir = Join-Path $ProjectRoot "windows"
} else {
    $SourceDir = [System.IO.Path]::GetFullPath($SourceDir)
}

if (-not $BuildDir) {
    $BuildDir = Join-Path $ProjectRoot "build"
} else {
    $BuildDir = [System.IO.Path]::GetFullPath($BuildDir)
}

Write-Host "Repository Root: $ProjectRoot"
Write-Host "Source Directory: $SourceDir"
Write-Host "Build Directory:  $BuildDir"
Write-Host "Configuration:    $Config"

# -----------------------------------------------------------------------------
# 2. Detect / Provision CMake and CTest
# -----------------------------------------------------------------------------
Write-Step "Detecting CMake and CTest..."

$CMakeExe = $null
$CTestExe = $null

if ($CMakeDir -and (Test-Path $CMakeDir)) {
    $Candidate = Join-Path $CMakeDir "cmake.exe"
    if (-not (Test-Path $Candidate)) { $Candidate = Join-Path $CMakeDir "bin\cmake.exe" }
    if (Test-Path $Candidate) { $CMakeExe = $Candidate }
} elseif ($env:CMAKE_DIR -and (Test-Path $env:CMAKE_DIR)) {
    $Candidate = Join-Path $env:CMAKE_DIR "cmake.exe"
    if (-not (Test-Path $Candidate)) { $Candidate = Join-Path $env:CMAKE_DIR "bin\cmake.exe" }
    if (Test-Path $Candidate) { $CMakeExe = $Candidate }
}

if (-not $CMakeExe) {
    $Cmd = Get-Command "cmake.exe" -ErrorAction SilentlyContinue
    if ($Cmd) { $CMakeExe = $Cmd.Source }
}

if (-not $CMakeExe) {
    $PythonScripts = Join-Path $env:LOCALAPPDATA "Programs\Python\Python312\Scripts\cmake.exe"
    $CandidatePaths = @(
        $PythonScripts,
        "C:\Program Files\CMake\bin\cmake.exe",
        "D:\Program Files\CMake\bin\cmake.exe",
        "C:\Program Files (x86)\CMake\bin\cmake.exe",
        (Join-Path $ProjectRoot "tools\cmake\bin\cmake.exe"),
        "C:\tools\cmake\bin\cmake.exe",
        "D:\tools\cmake\bin\cmake.exe"
    )

    $PythonCmd = Get-Command "python.exe" -ErrorAction SilentlyContinue
    if ($PythonCmd) {
        $PyDir = Split-Path -Parent $PythonCmd.Source
        $CandidatePaths += (Join-Path $PyDir "Scripts\cmake.exe")
    }

    foreach ($Candidate in $CandidatePaths) {
        if ($Candidate -and (Test-Path $Candidate)) {
            $CMakeExe = $Candidate
            break
        }
    }
}

if (-not $CMakeExe) {
    Write-Fail "CMake executable not found!"
    Write-Host "Please ensure CMake is installed. Run: python -m pip install cmake ninja"
    exit 1
}

$CMakeBinDir = Split-Path -Parent $CMakeExe
$CandidateCTest = Join-Path $CMakeBinDir "ctest.exe"
if (Test-Path $CandidateCTest) {
    $CTestExe = $CandidateCTest
} else {
    $CTestCmd = Get-Command "ctest.exe" -ErrorAction SilentlyContinue
    if ($CTestCmd) { $CTestExe = $CTestCmd.Source }
}

if ($env:PATH -notlike "*$CMakeBinDir*") {
    $env:PATH = "$CMakeBinDir;$env:PATH"
}

$CMakeVer = & "$CMakeExe" --version | Select-Object -First 1
Write-Success "Found CMake: $CMakeExe ($CMakeVer)"
if ($CTestExe) {
    Write-Success "Found CTest: $CTestExe"
} else {
    Write-Warn "CTest not found in $CMakeBinDir; test step will be skipped."
    $SkipTest = $true
}

# -----------------------------------------------------------------------------
# 3. Detect / Provision Compiler & Build Toolchain
# -----------------------------------------------------------------------------
Write-Step "Detecting C++20 Compiler and Build Toolchain..."

$CompilerType = $null
$CompilerExe = $null
$CompilerBinDir = $null

if ($CompilerDir -and (Test-Path $CompilerDir)) {
    $BinCandidate = if (Test-Path (Join-Path $CompilerDir "g++.exe")) { $CompilerDir } else { Join-Path $CompilerDir "bin" }
    if (Test-Path (Join-Path $BinCandidate "g++.exe")) {
        $CompilerType = "MinGW"
        $CompilerExe = Join-Path $BinCandidate "g++.exe"
        $CompilerBinDir = $BinCandidate
    } elseif (Test-Path (Join-Path $BinCandidate "cl.exe")) {
        $CompilerType = "MSVC"
        $CompilerExe = Join-Path $BinCandidate "cl.exe"
        $CompilerBinDir = $BinCandidate
    }
}

# Search candidate portable MinGW / w64devkit / LLVM paths first
if (-not $CompilerType) {
    $CandidateBinDirs = @(
        (Join-Path $ProjectRoot "tools\w64devkit\bin"),
        (Join-Path $ProjectRoot "tools\compiler\bin"),
        "C:\tools\w64devkit\bin",
        "D:\tools\w64devkit\bin",
        "C:\w64devkit\bin",
        "D:\w64devkit\bin",
        (Join-Path $env:USERPROFILE "tools\w64devkit\bin"),
        "C:\msys64\ucrt64\bin",
        "C:\msys64\mingw64\bin",
        "C:\winlibs\bin",
        "D:\winlibs\bin",
        (Join-Path $env:LOCALAPPDATA "Programs\w64devkit\bin"),
        "C:\Program Files\LLVM\bin",
        "D:\Program Files\LLVM\bin"
    )

    if ($env:MINGW_HOME) { $CandidateBinDirs = @((Join-Path $env:MINGW_HOME "bin"), $env:MINGW_HOME) + $CandidateBinDirs }
    if ($env:W64DEVKIT_HOME) { $CandidateBinDirs = @((Join-Path $env:W64DEVKIT_HOME "bin"), $env:W64DEVKIT_HOME) + $CandidateBinDirs }
    if ($env:COMPILER_DIR) { $CandidateBinDirs = @((Join-Path $env:COMPILER_DIR "bin"), $env:COMPILER_DIR) + $CandidateBinDirs }

    foreach ($Dir in $CandidateBinDirs) {
        if ($Dir -and (Test-Path $Dir)) {
            $GppPath = Join-Path $Dir "g++.exe"
            $ClangPath = Join-Path $Dir "clang++.exe"
            if (Test-Path $GppPath) {
                $CompilerType = "MinGW"
                $CompilerExe = $GppPath
                $CompilerBinDir = $Dir
                break
            } elseif (Test-Path $ClangPath) {
                $CompilerType = "Clang"
                $CompilerExe = $ClangPath
                $CompilerBinDir = $Dir
                break
            }
        }
    }
}

# Check PATH if not found in tools
if (-not $CompilerType) {
    $GppCmd = Get-Command "g++.exe" -ErrorAction SilentlyContinue
    $ClCmd  = Get-Command "cl.exe"  -ErrorAction SilentlyContinue
    $ClangCmd = Get-Command "clang++.exe" -ErrorAction SilentlyContinue

    if ($GppCmd) {
        $CompilerType = "MinGW"
        $CompilerExe = $GppCmd.Source
        $CompilerBinDir = Split-Path -Parent $CompilerExe
    } elseif ($ClCmd) {
        $CompilerType = "MSVC"
        $CompilerExe = $ClCmd.Source
        $CompilerBinDir = Split-Path -Parent $CompilerExe
    } elseif ($ClangCmd) {
        $CompilerType = "Clang"
        $CompilerExe = $ClangCmd.Source
        $CompilerBinDir = Split-Path -Parent $CompilerExe
    }
}

# Check Visual Studio installation via vswhere if still not found
$VSInstallDir = $null
if (-not $CompilerType) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $VSPath = & "$vswhere" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($VSPath -and (Test-Path $VSPath)) {
            $CompilerType = "MSVC_IDE"
            $VSInstallDir = $VSPath
        }
    }
}

if (-not $CompilerType) {
    Write-Fail "No suitable C++ compiler found (g++, cl, clang++)!"
    Write-Host "Please provision a modern C++ compiler to tools\w64devkit\."
    exit 1
}

if ($CompilerBinDir -and ($env:PATH -notlike "*$CompilerBinDir*")) {
    $env:PATH = "$CompilerBinDir;$env:PATH"
    Write-Host "Added to PATH: $CompilerBinDir"
}

if ($CompilerExe) {
    $CompVer = & "$CompilerExe" --version 2>$null | Select-Object -First 1
    Write-Success "Found $CompilerType compiler: $CompilerExe ($CompVer)"
} elseif ($CompilerType -eq "MSVC_IDE") {
    Write-Success "Found Visual Studio installation: $VSInstallDir"
}

# -----------------------------------------------------------------------------
# 4. Detect Ninja & Determine CMake Generator
# -----------------------------------------------------------------------------
Write-Step "Selecting CMake Generator..."

$NinjaExe = $null
$NinjaCmd = Get-Command "ninja.exe" -ErrorAction SilentlyContinue
if ($NinjaCmd) {
    $NinjaExe = $NinjaCmd.Source
} else {
    $CandidateNinjas = @(
        (Join-Path $CMakeBinDir "ninja.exe"),
        (Join-Path $env:LOCALAPPDATA "Programs\Python\Python312\Scripts\ninja.exe"),
        (Join-Path $ProjectRoot "tools\ninja\ninja.exe"),
        (Join-Path $CompilerBinDir "ninja.exe"),
        "C:\tools\ninja\ninja.exe",
        "D:\tools\ninja\ninja.exe"
    )
    foreach ($Candidate in $CandidateNinjas) {
        if ($Candidate -and (Test-Path $Candidate)) {
            $NinjaExe = $Candidate
            $NinjaDir = Split-Path -Parent $NinjaExe
            if ($env:PATH -notlike "*$NinjaDir*") { $env:PATH = "$NinjaDir;$env:PATH" }
            break
        }
    }
}

if ($NinjaExe) {
    $NinjaVer = & "$NinjaExe" --version 2>$null
    Write-Success "Found Ninja: $NinjaExe ($NinjaVer)"
}

$ChosenGenerator = $Generator
$GeneratorArgs = @()

if (-not $ChosenGenerator) {
    if ($CompilerType -eq "MinGW" -or $CompilerType -eq "Clang") {
        if ($NinjaExe) {
            $ChosenGenerator = "Ninja"
        } else {
            $ChosenGenerator = "MinGW Makefiles"
        }
    } elseif ($CompilerType -eq "MSVC") {
        if ($NinjaExe) {
            $ChosenGenerator = "Ninja"
        } else {
            $ChosenGenerator = "Visual Studio 17 2022"
            $GeneratorArgs += @("-A", "x64")
        }
    } elseif ($CompilerType -eq "MSVC_IDE") {
        $ChosenGenerator = "Visual Studio 17 2022"
        $GeneratorArgs += @("-A", "x64")
    }
}

if ($ChosenGenerator) {
    Write-Success "Selected CMake Generator: '$ChosenGenerator'"
} else {
    Write-Host "Using default CMake Generator."
}

# -----------------------------------------------------------------------------
# 5. Build Directory Management (Clean Phase)
# -----------------------------------------------------------------------------
if ($Clean) {
    Write-Step "Cleaning build directory: $BuildDir"
    if (Test-Path $BuildDir) {
        Remove-Item -Path $BuildDir -Recurse -Force
        Write-Success "Build directory removed."
    }
}

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
}

# -----------------------------------------------------------------------------
# 6. Configure Phase
# -----------------------------------------------------------------------------
Write-Step "Configuring CMake (cmake -B build -S windows)..."

$ConfigureCmd = @(
    "$CMakeExe",
    "-B", "$BuildDir",
    "-S", "$SourceDir",
    "-DCMAKE_BUILD_TYPE=$Config"
)

if ($CompilerType -eq "MinGW" -and $CompilerBinDir) {
    $GppPath = (Join-Path $CompilerBinDir "g++.exe") -replace "\\", "/"
    $GccPath = (Join-Path $CompilerBinDir "gcc.exe") -replace "\\", "/"
    $ConfigureCmd += @("-DCMAKE_CXX_COMPILER=$GppPath", "-DCMAKE_C_COMPILER=$GccPath")
}

if ($ChosenGenerator) {
    $ConfigureCmd += @("-G", "$ChosenGenerator")
    if ($GeneratorArgs) {
        $ConfigureCmd += $GeneratorArgs
    }
}

if ($ExtraCMakeArgs) {
    $ConfigureCmd += $ExtraCMakeArgs
}

Write-Host "Executing: $($ConfigureCmd -join ' ')" -ForegroundColor DarkGray
if ($DryRun) {
    Write-Host "[DryRun] Would execute configuration." -ForegroundColor Magenta
} else {
    & $ConfigureCmd[0] $ConfigureCmd[1..($ConfigureCmd.Length - 1)]
    if ($LASTEXITCODE -ne 0) {
        Write-Fail "CMake configuration failed with exit code $LASTEXITCODE."
        exit $LASTEXITCODE
    }
    Write-Success "CMake configuration completed successfully."
}

# -----------------------------------------------------------------------------
# 7. Build Phase
# -----------------------------------------------------------------------------
Write-Step "Building Target (cmake --build build --config $Config)..."

$BuildCmd = @(
    "$CMakeExe",
    "--build", "$BuildDir",
    "--config", "$Config",
    "--parallel"
)

Write-Host "Executing: $($BuildCmd -join ' ')" -ForegroundColor DarkGray
if ($DryRun) {
    Write-Host "[DryRun] Would execute build." -ForegroundColor Magenta
} else {
    & $BuildCmd[0] $BuildCmd[1..($BuildCmd.Length - 1)]
    if ($LASTEXITCODE -ne 0) {
        Write-Fail "Compilation failed with exit code $LASTEXITCODE."
        exit $LASTEXITCODE
    }
    Write-Success "Compilation completed successfully."
}

# -----------------------------------------------------------------------------
# 8. Test Phase
# -----------------------------------------------------------------------------
if (-not $SkipTest) {
    Write-Step "Running Unit Tests (ctest --test-dir build --output-on-failure)..."

    $TestCmd = @(
        "$CTestExe",
        "--test-dir", "$BuildDir",
        "--output-on-failure",
        "-C", "$Config"
    )

    Write-Host "Executing: $($TestCmd -join ' ')" -ForegroundColor DarkGray
    if ($DryRun) {
        Write-Host "[DryRun] Would execute tests." -ForegroundColor Magenta
    } else {
        & $TestCmd[0] $TestCmd[1..($TestCmd.Length - 1)]
        if ($LASTEXITCODE -ne 0) {
            Write-Fail "Unit tests failed with exit code $LASTEXITCODE."
            exit $LASTEXITCODE
        }
        Write-Success "All unit tests passed successfully."
    }
} else {
    Write-Warn "Skipping test execution as requested (-SkipTest)."
}

# -----------------------------------------------------------------------------
# 9. Final Verification Summary (AC2)
# -----------------------------------------------------------------------------
Write-Host "`n================================================================================" -ForegroundColor Green
Write-Host " [AC2 VERIFIED] Windows C++ Project Built & Tested Successfully!" -ForegroundColor Green
Write-Host "================================================================================" -ForegroundColor Green
Write-Host "Build Directory: $BuildDir"
Write-Host "Configuration:   $Config"
Write-Host "Generator:       $ChosenGenerator"
Write-Host "Compiler:        $CompilerType ($CompilerExe)"
Write-Host "Status:          CLEAN EXIT (0)"
Write-Host "================================================================================`n" -ForegroundColor Green

exit 0
