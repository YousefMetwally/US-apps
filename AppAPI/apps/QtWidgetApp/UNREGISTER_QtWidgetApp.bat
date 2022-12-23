@echo off
echo NOTICE: Script MUST be run as Administrator.
:: Errors from "reg" tool are muted to avoid flooding the build log with errors from already deleted registry entries.

:: Fix issue with "Run as Administrator" current dir
setlocal enableextensions
cd /d "%~dp0"


:: Remove all traces of QtWidgetApp from registry
for %%R in (HKEY_LOCAL_MACHINE HKEY_CURRENT_USER) do (
  :: TypeLib & AppID
  reg delete "%%R\SOFTWARE\Classes\TypeLib\{B58EAFA5-C53A-4D67-9698-55D8C2EA47D6}" /f 2> NUL
  reg delete "%%R\SOFTWARE\Classes\AppID\{3ECBF94A-21FE-40EF-B60A-B0BCC514ABBC}"   /f 2> NUL
  
  for %%P in (32 64) do (
    :: QtWidgetApp class
    reg delete "%%R\SOFTWARE\Classes\CLSID\{27F51D13-CD40-4390-98D2-9AE79BA9216B}"     /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\Interface\{08E0BF35-A529-44CE-A2BF-E404C4B93E53}" /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\Interface\{8BE39C99-67C5-437D-A55C-B3FDF7534240}" /f /reg:%%P 2> NUL
  )
)

::pause
