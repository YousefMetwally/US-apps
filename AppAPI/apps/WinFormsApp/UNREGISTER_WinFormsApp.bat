@echo off
echo NOTICE: Script MUST be run as Administrator.
:: Errors from "reg" tool are muted to avoid flooding the build log with errors from already deleted registry entries.

:: Fix issue with "Run as Administrator" current dir
setlocal enableextensions
cd /d "%~dp0"


:: Remove all traces of WinFormsApp from registry
for %%R in (HKEY_LOCAL_MACHINE HKEY_CURRENT_USER) do (
  for %%P in (32 64) do (
    :: COM class
    reg delete "%%R\SOFTWARE\Classes\CLSID\{B8AE7209-83FE-4035-966D-21AC0DA1DCE4}"     /f /reg:%%P 2> NUL
  )
)

::pause
