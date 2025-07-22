set PLUGINNAME=SteamInputSimplePlugin
set ENGINEDIR=D:\UnrealEngine
set RUNUAT=%ENGINEDIR%\Engine\Build\BatchFiles\RunUAT.bat
set package_dir=%~dp0Build\Plugins\%PLUGINNAME%
set plugin_dir=%~dp0Plugins\%PLUGINNAME%

rmdir %package_dir% /s /q

call "%RUNUAT%" BuildPlugin -Plugin="%plugin_dir%\%PLUGINNAME%.uplugin" -Package=%package_dir% -Rocket

rem rmdir %plugin_dir%\Binaries /s /q
rem rmdir %plugin_dir%\Intermediate /s /q

rem Robocopy "%package_dir%\Binaries" "%plugin_dir%\Binaries" /D

pause