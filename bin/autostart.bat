@echo off
:: BatchGotAdmin  
:-------------------------------------  
REM  --> Check for permissions  
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"  
  
REM --> If error flag set, we do not have admin.  
if '%errorlevel%' NEQ '0' (  
    echo Requesting administrative privileges...  
    goto UACPrompt  
) else ( goto gotAdmin )  
  
:UACPrompt  
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"  
    echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%temp%\getadmin.vbs"  
  
    "%temp%\getadmin.vbs"  
    exit /B  
  
:gotAdmin  
    if exist "%temp%\getadmin.vbs" ( del "%temp%\getadmin.vbs" )  
    pushd "%CD%"  
    CD /D "%~dp0"  
:-------------------------------------- 

echo "runas adminstrator"
set SVR=hidapi
set ARG=
setlocal enabledelayedexpansion
chcp 936
(sc query %SVR%|findstr /i win32 >nul 2>nul) && ((sc stop %SVR% 2>nul) && (sc delete %SVR%) ||(sc delete %SVR%))
sc create %SVR% binPath= "%cd%\srvany.exe" start= auto type= own
REG ADD HKLM\SYSTEM\CURRENTCONTROLSET\services\%SVR% /V DESCRIPTION /T REG_SZ /D "%SVR% SERVICE, PLEASE DON'T DELETE IT!" /f
REG ADD HKLM\SYSTEM\CURRENTCONTROLSET\services\%SVR%\Parameters /V APPLICATION /T REG_SZ /D %~dp0hidapi.exe /f

sc start %SVR% %ARG%

pause
