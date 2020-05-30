# Drill dotNet agent module

This repository contains a native agent to profile .NET applications and to report results to Drill4J admin services.

The profiler is a COM DLL loaded to Common Language Runtime process on startup. It provides a component class and implements necessary interfaces.

# Sources

Source repository is located at GitHub: https://github.com/Drill4J/agent-dotnet

To get sources locally, run:
```
git clone --recurse-submodules https://github.com/Drill4J/agent-dotnet
```

Note: `agent-dotnet` module depends on `dotnet/runtime` repository, located at GitHib: https://github.com/dotnet/runtime

# Visual Studio 2019 installation

The project is built with Visual Studio 2019 (or corresponding MS Build tools).

Visual Studio 2019 installation configuration (required):

- Workloads:
  - Desktop development with C++;
  - .NET desktop development;
  - .NET Core cross-platform development.
- Individual components (make sure the following ones are selected):
  - MSVC v142 - VS 2019 C++ x64/x86 build tools (latest available);
  - C++ ATL for latest v142 build tools (x86 & x64);
  - Windows 10 SDK (latest available);
  - Test Adapter for Google Test;
  - .NET Core 3.1 SDK
  - .NET Framework 4.7.2 SDK or .NET Framework 4.8 SDK

# Solution and projects

Visual Studio 2019 solution `Drill4dotNet.sln` in the top repository folder consists of:

- `Drill4dotNet.sln`:
  - `Drill4dotNet` folder with a couple of C++ projects:
    - `Connector.vcxproj` to build `Connector.exe`, a test application for communication with admin;
    - `Drill4dotNet.vcxproj` to build the COM DLL `Drill4dotNet.dll`;
    - `Drill4dotNetPS.vcxproj` proxy/stub DLL (not used);
    - `Drill4dotNet-Tests.vcxproj` unit tests for `Drill4dotNet.vcxproj`;
    - `Injection.csproj` to build .NET `Injection.dll` to be injected to the target application by the profiler.
  - `HelloWorld` folder with a couple of sample C# projects:
    - `HelloWorld.csproj` to build an application to run in .NET Core 3.1 runtime;
    - `HelloWorldFramework.csproj` to build an application to run in .NET Framework 4.x CLR;
  - `Solution items` folder
    - `README.md` - this file;
    - `setruntimeenv.cmd` (see above).


`Drill4dotNet.vcxproj` project is created in ATL COM frame; it uses latest Platform SDK (10.0) and dependency profiling files from [dotnet/runtime](https://github.com/dotnet/runtime) repository as a submodule in `./dependencies` folder.
It is written in C++20 language standard.

Note: Do not build (deselect in Batch Build and Configuration Manager) `Drill4dotNetPS.vcxproj`; it is reserved for the future.

Note: Do not build (deselect in Batch Build and Configuration Manager) Win32 configurations.

`HelloWorld.csproj` project requires .NET Core 3.1 SDK to be installed. If you have different version installed, change Target framework in the project properties. 

`HelloWorldFramework.csproj` project requires .NET Framework 4.7.2 SDK to be installed. If you have different version installed, change Target framework in the project properties. 

Note: You can run HelloWorld.csproj (set As StartUp Project) from the IDE and under debugger. The project defines necessary Environment variables under Debug page. 

# Building

Before the first build, run the following commands in the `dependencies\vcpkg` directory:
```
bootstrap-vcpkg.bat
vcpkg install nlohmann-json:x64-windows
```

# Running Drill dotNet agent

Agent DLL binaries can be found under `./bin/$Platform/$Configuration/Drill4dotNet/` (for example: `./bin/x64/Release/Drill4dotNet/`):
- `Drill4dotNet.dll`
- `Injection.dll`

Sample projects binaries can be found here:
- `./bin/$Platform/$Configuration/HelloWorld/netcoreapp3.1/HelloWorld.exe`
- `./bin/$Platform/$Configuration/HelloWorldFramework/HelloWorldFramework.exe`

The current version just outputs information to the console window. 

- Start cmd. 
- Configure cmd window to keep a large amount of lines: System menu / Properties / Layout / Screen Buffer Size / Height = 9999 (maximum).
- Run `./setruntimeenv.cmd` in the top repository folder to configure the environment for profiling. It can be run without elevation. The environment settings affect both .NET Core runtime and .NET Framework CLR, whichever is installed and will be running in this console.
  Optional parameter can be used to choose between Debug and Release variants of the builds.
- Run a .NET application in the same console session, for example:
    ```
    dotnet.exe .\bin\x64\Release\HelloWorld\netcoreapp3.1\HelloWorld.dll
    ```
- The profiler prints to stdout the information about events from the target application executed in CLR.
- You can redirect the output to a file, for example:
    ```
    .\bin\x64\Release\HelloWorldFramework\HelloWorldFramework.exe >profiling.log
    ```


In addition to `setruntimeenv.cmd`, you can register the COM DLL in the system using command `regsvr32.exe Drill4dotNet.dll`. To register/unregister, you need administrative privileges.


# Limitations

* The profiler only outputs to stdout; it is not controlled or configured.
* Only Windows 64-bit (x64) platform is currently supported.
* ATM, the agent injects artificial calls to Console.WriteLine into the target sample function. Injection is not supported to functions with exceptions, generic functions, unmanaged (native) functions.
* ATM, the profiler is not thread-safe.
