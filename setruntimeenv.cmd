@echo Setting .NET Core run-time configuration options for profiling...
set COMPlus_EnableDiagnostics=1
set CORECLR_ENABLE_PROFILING=1
set CORECLR_PROFILER=BB54E37E-3E2E-4B45-AD99-223CB351098E
set CORECLR_PROFILER_PATH=.\Drill4dotNet\bin\x64\Release\Drill4dotNet.dll
@echo Also setting .NET Framework run-time configuration options...
set COR_ENABLE_PROFILING=%CORECLR_ENABLE_PROFILING%
set COR_PROFILER=%CORECLR_PROFILER%
set COR_PROFILER_PATH=%CORECLR_PROFILER_PATH%
@echo Registering COM component...
regsvr32 -s "%CORECLR_PROFILER_PATH%" || echo Registration failed.
