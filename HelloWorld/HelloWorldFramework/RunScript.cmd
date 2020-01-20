@echo Also setting .NET Framework run-time configuration options...
set COR_ENABLE_PROFILING=1
set COR_PROFILER={BE4D1E40-1A9F-45E4-BCE4-68DE191CC8D6}
set COR_PROFILER_PATH=%~dp0..\Drill4dotNet\Drill4dotNet.dll
@echo Setting compatibility for .NET Framework 4.x due to use outdated profiler...
set COMPlus_ProfAPI_ProfilerCompatibilitySetting=EnableV2Profiler

@echo Running the Program
%~dp0\HelloWorldFramework.exe

pause
