# Drill dotNet agent module

This repository contains the native agent used to profile .NET applications and report the results to Drill4J admin services.

Agent DLL binaries can be found under ./Drill4dotNet/bin/$Platform/$Configuration/Drill4dotNet.dll

To make `Drill4dotNet.dll` to be loaded into .NET machine on startup, correct paths in ./setruntimeenv.cmd and execute it to register COM DLL before starting a .NET application.

Note: ATM, only Windows 32/64-bit is currently supported.
