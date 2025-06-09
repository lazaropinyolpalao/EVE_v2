cls
@echo off
IF EXIST ..\build rmdir /s /q ..\build
IF EXIST ..\compiledlibs rmdir /s /q ..\compiledlibs
IF EXIST ..\bin rmdir /s /q ..\bin
IF EXIST ..\*.sln del /s /q ..\*.sln
IF EXIST ..\deps\DebugPC rmdir /s /q ..\DebugPC
IF EXIST ..\deps\ReleasePC rmdir /s /q ..\ReleasePC
IF EXIST ..\deps\RelWithDebInfoPC rmdir /s /q ..\RelWithDebInfoPC

pause