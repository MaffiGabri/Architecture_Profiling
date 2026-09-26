@echo off
rem ============================================================================
rem Architecture Profiling Companion - Windows Build Wrapper (Batch Entry Point)
rem Satisfies Acceptance Criterion AC2 ("Il progetto Windows si compila senza errori")
rem
rem Usage:
rem   .\windows\build.bat                      (Default: Release build + ctest)
rem   .\windows\build.bat -Clean               (Wipe build directory first)
rem   .\windows\build.bat -Config Debug        (Build Debug configuration)
rem   .\windows\build.bat -SkipTest            (Skip ctest execution)
rem   .\windows\build.bat -CompilerDir <path>  (Explicit compiler path)
rem ============================================================================

setlocal enabledelayedexpansion

set "WRAPPER_DIR=%~dp0"
set "PS_SCRIPT=%WRAPPER_DIR%..\scripts\build_windows.ps1"

if not exist "%PS_SCRIPT%" (
    if exist "%WRAPPER_DIR%scripts\build_windows.ps1" (
        set "PS_SCRIPT=%WRAPPER_DIR%scripts\build_windows.ps1"
    ) else if exist "%CD%\scripts\build_windows.ps1" (
        set "PS_SCRIPT=%CD%\scripts\build_windows.ps1"
    ) else (
        echo [ERROR] Cannot locate scripts\build_windows.ps1!
        exit /b 1
    )
)

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%PS_SCRIPT%" %*
set "EXIT_CODE=%ERRORLEVEL%"

exit /b %EXIT_CODE%
