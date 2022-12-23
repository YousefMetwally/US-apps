@echo off
echo NOTICE: Script MUST be run as Administrator.
:: Errors from "reg" tool are muted to avoid flooding the build log with errors from already deleted registry entries.

:: Fix issue with "Run as Administrator" current dir
setlocal enableextensions
cd /d "%~dp0"


:: Remove all traces of QtQuickApp from registry
for %%R in (HKEY_LOCAL_MACHINE HKEY_CURRENT_USER) do (
  :: TypeLib & AppID
  reg delete "%%R\SOFTWARE\Classes\TypeLib\{ADFFA069-3C72-4296-9F3C-2DDE1192C098}" /f 2> NUL
  reg delete "%%R\SOFTWARE\Classes\AppID\{5C343A81-7F27-433B-B8AB-975B1A1FE046}"   /f 2> NUL
  
  for %%P in (32 64) do (
    :: QtQuickApp class
    reg delete "%%R\SOFTWARE\Classes\CLSID\{F2439C70-97A2-48B7-A9F7-6A95F5563084}"     /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\Interface\{E70F7F0D-1562-4563-95EA-3154217951C0}" /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\Interface\{F117F234-676B-4437-84A7-BDC7C2C92116}" /f /reg:%%P 2> NUL
  )
)

::pause
