@echo off
echo NOTICE: Script MUST be run as Administrator.
:: Errors from "reg" tool are muted to avoid flooding the build log with errors from already deleted registry entries.

:: Fix issue with "Run as Administrator" current dir
setlocal enableextensions
cd /d "%~dp0"


:: Remove all traces of QtWebBrowserApp from registry
for %%R in (HKEY_LOCAL_MACHINE HKEY_CURRENT_USER) do (
  :: TypeLib & AppID
  reg delete "%%R\SOFTWARE\Classes\TypeLib\{401B111E-19B5-495D-AD8D-EE66E9F0AE5A}" /f 2> NUL
  reg delete "%%R\SOFTWARE\Classes\AppID\{A1D606A6-13FF-482B-8DA8-650BB6DE9EB6}"   /f 2> NUL
  
  for %%P in (32 64) do (
    :: QtWebBrowserApp class
    reg delete "%%R\SOFTWARE\Classes\CLSID\{6C063252-8BA4-42B1-B0C9-D72C89130EA7}"     /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\Interface\{3E46EB5F-9E51-4D42-A018-9C538D4D9060}" /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\Interface\{227BD063-9720-48A3-8D70-4EEE45026167}" /f /reg:%%P 2> NUL
  )
)

::pause
