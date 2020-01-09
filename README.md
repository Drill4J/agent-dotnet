# Drill dotNet agent module

This repository contains the native agent used to profile .NET applications and report the results to Drill4J admin services.

Profiler is a COM DLL loaded to dotnet.exe process on startup. It provides component class and implements specific interfaces.

Agent DLL binaries can be found under ./Drill4dotNet/bin/$Platform/$Configuration/Drill4dotNet.dll

To make `Drill4dotNet.dll` to be loaded into .NET machine on startup, adjust `CORECLR_PROFILER_PATH` in `./setruntimeenv.cmd` to the desired path and run `./setruntimeenv.cmd` in an elevated console to register COM DLL and define environment settings before starting a .NET application.

Note: ATM, only Windows 32/64-bit is currently supported.

Note: Current project is built with ATL COM frame and .NET Frameworks 4.x SDK.

