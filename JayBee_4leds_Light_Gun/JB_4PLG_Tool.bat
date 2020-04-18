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
    set params = %*:"=""
    echo UAC.ShellExecute "cmd.exe", "/c %~s0 %params%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------

:MAIN
cls
echo.
echo ==================================================================
echo.
echo              --- JB LIGHT GUN 4 POINTS SYSTEM  ---
echo.
echo                        *Config Tool*
echo.
echo.
echo  1. Flash the firmware
echo    (recommended if not up to date)
echo.
echo  2. Set the screen aspect ratio
echo    (recommended if never done before)
echo.
echo  3. Enable/Disable or setup the temperature sensor
echo    (ONLY recommended if you connected a sensor)
echo.
echo  4. Clear the arduino EEPROM
echo    (ONLY recommended if coming a firmware lower than 1.85)
echo.
echo  5. Exit
echo.
echo ==================================================================
echo.
SET /P MENUCHOICE=Input a choice:

IF /I "%MENUCHOICE%"=="1" GOTO FLASH
IF /I "%MENUCHOICE%"=="2" GOTO SETSCR
IF /I "%MENUCHOICE%"=="3" GOTO TMPSEN
IF /I "%MENUCHOICE%"=="4" GOTO CLREEP
IF /I "%MENUCHOICE%"=="5" GOTO END
goto MAIN


:FLASH
cls
echo.
echo ==================================================================
echo.
echo      		Flash the normal or test firmware ? 
echo.
echo         /!\    Don't flash the wrong firmware,    /!\
echo         /!\    you might damage your hardware!    /!\
echo.
echo          Read the forum topic for more informations.
echo.
echo ==================================================================
echo.
SET /P FIRMTYPE=(N: NORMAL, T: TEST, C: Cancel)?

IF /I "%FIRMTYPE%"=="N" GOTO NFIRM
IF /I "%FIRMTYPE%"=="T" GOTO TFIRM
goto MAIN

:NFIRM
for /f %%a in ('dir /B /a-d *leonardo.hex') do set hexfile=%%a
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%Arduino%%'" Get DeviceID ^| FIND "COM"`) do set comport1=%%B
mode %comport1% BAUD=1200 PARITY=n DATA=8
TIMEOUT 3 /NOBREAK
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%bootloader%%'" Get DeviceID ^| FIND "COM"`) do set comport2=%%B
if "%comport2%"=="" (
TIMEOUT 3 /NOBREAK
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%bootloader%%'" Get DeviceID ^| FIND "COM"`) do set comport2=%%B
)
avrdude -C avrdude.conf -v -patmega32u4 -cavr109 -P%comport2% -b57600 -D -Uflash:w:%hexfile%:i
goto CLREEP

:TFIRM
for /f %%a in ('dir /B /a-d *leonardo.test.hex') do set hexfile=%%a
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%Arduino%%'" Get DeviceID ^| FIND "COM"`) do set comport1=%%B
mode %comport1% BAUD=1200 PARITY=n DATA=8
TIMEOUT 3 /NOBREAK
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%bootloader%%'" Get DeviceID ^| FIND "COM"`) do set comport2=%%B
if "%comport2%"=="" (
TIMEOUT 3 /NOBREAK
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%bootloader%%'" Get DeviceID ^| FIND "COM"`) do set comport2=%%B
)
avrdude -C avrdude.conf -v -patmega32u4 -cavr109 -P%comport2% -b57600 -D -Uflash:w:%hexfile%:i
goto CLREEP

:SETSCR
cls
echo.
echo ==================================================================
echo.
echo        Input your screen Width, then your screen Height.
echo.
echo  Example:
echo   For a 4:3 screen, input 4, then 3.
echo.
echo  Note: the default aspect ratio is 16:9,
echo        so if you have a 16:9 screen, 
echo        no need to change anything.
echo.
echo ==================================================================
echo.
SET /P SCRW=Screen Width?
SET /P SCRH=Screen Height?

cls
echo.
echo ==================================================================
echo.
echo        Your screen aspect ratio is %SCRW%:%SCRH%, is that correct?
echo.
echo ==================================================================
echo.
SET /P SCRQ=Y: Yes, N: No and retype, C: Cancel?

IF /I "%SCRQ%"=="Y" GOTO DOSCR
IF /I "%SCRQ%"=="N" GOTO SETSCR
IF /I "%SCRQ%"=="C" GOTO MAIN
goto MAIN

:DOSCR
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%Arduino%%'" Get DeviceID ^| FIND "COM"`) do set comport1=%%B
mode %comport1% BAUD=9600 PARITY=n DATA=8
echo ZY6,%SCRW% > %comport1%
echo ZY7,%SCRH% > %comport1%
goto RESTART

:TMPSEN
cls
echo.
echo ==================================================================
echo.
echo       Do you want to enable/disable the temperature sensor,
echo                  or setup the threshold values? 
echo           This function will change the solenoid speed
echo              depending on the sensed temperature.
echo.
echo           /!\     Enable only if you connected     /!\
echo           /!\     a TMP36 sensor to the pin A0     /!\
echo.
echo            Read the forum topic for more informations.
echo.
echo ==================================================================
echo.
SET /P TMPQ=E: Enable, D: Disable, S: Setup, C: Cancel?

IF /I "%TMPQ%"=="E" GOTO ENASEN
IF /I "%TMPQ%"=="D" GOTO DISSEN
IF /I "%TMPQ%"=="S" GOTO SETSEN
goto MAIN

:ENASEN
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%Arduino%%'" Get DeviceID ^| FIND "COM"`) do set comport1=%%B
mode %comport1% BAUD=9600 PARITY=n DATA=8
echo ZY16,1 > %comport1%
goto RESTART

:DISSEN
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%Arduino%%'" Get DeviceID ^| FIND "COM"`) do set comport1=%%B
mode %comport1% BAUD=9600 PARITY=n DATA=8
echo ZY16,0 > %comport1%
goto RESTART

:SETSEN
cls
echo.
echo ==================================================================
echo.
echo           Enter the lower threshold in celsius degree
echo        (the temp where the solenoid starts slowing down)
echo.
echo ==================================================================
echo.
SET /P LTS=Threshold?

cls
echo.
echo ==================================================================
echo.
echo           Enter the higher threshold in celsius degree
echo             (the temp where the solenoid is stopped)
echo.
echo ==================================================================
echo.
SET /P HTS=Threshold?

cls
echo.
echo ==================================================================
echo.
echo     Your solenoid will gradually slow down from %LTS% degrees C,
echo          to stop at %HTS% degrees C, is that correct?
echo.
echo ==================================================================
echo.
SET /P SCRQ=Y: Yes, N: No and retype, C: Cancel?

IF /I "%SCRQ%"=="Y" GOTO DOSTS
IF /I "%SCRQ%"=="N" GOTO SETSEN
goto MAIN

:DOSTS
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%Arduino%%'" Get DeviceID ^| FIND "COM"`) do set comport1=%%B
mode %comport1% BAUD=9600 PARITY=n DATA=8
echo ZY30,%LTS% > %comport1%
echo ZY31,%HTS% > %comport1%
goto RESTART

:CLREEP
cls
echo.
echo ==================================================================
echo.
echo        Do you want to clear the arduino EEPROM memory? 
echo.
echo              This will clear any custom setting,
echo                 but is necessary if updating
echo                from a firmare lower than 1.85.
echo.
echo        /!\    Recommended ONLY if you are coming    /!\
echo        /!\     from a firmware lower than 1.85,     /!\
echo        /!\    or if you want to reset everything.   /!\
echo        /!\   Doing it too often is not recommended. /!\
echo.
echo          Read the forum topic for more informations.
echo.
echo ==================================================================
echo.
SET /P CLRQ=Y: Yes, N: No?

IF /I "%CLRQ%"=="Y" GOTO DOCLR
goto MAIN

:DOCLR
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%Arduino%%'" Get DeviceID ^| FIND "COM"`) do set comport1=%%B
if "%comport1%"=="" (
echo The arduino is not responding yet, please wait...
TIMEOUT 9 /NOBREAK
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%Arduino%%'" Get DeviceID ^| FIND "COM"`) do set comport1=%%B
)
mode %comport1% BAUD=9600 PARITY=n DATA=8
echo ZC > %comport1%
cls
echo.
echo ==================================================================
echo.
echo                       Memory cleared !
echo.
echo          Don't forget to set your screen aspect ratio
echo                  if your screen is not 16:9
echo.
echo ==================================================================
echo.
goto RESTART

:RESTART
TIMEOUT 2 /NOBREAK
mode %comport1% BAUD=1200 PARITY=n DATA=8
echo The arduino is restarting, please wait...
TIMEOUT 10 /NOBREAK
goto MAIN

:END