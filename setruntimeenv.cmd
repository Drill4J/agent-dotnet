@echo Setting .NET Core run-time configuration options for profiling...
set COMPlus_EnableDiagnostics=1
set CORECLR_ENABLE_PROFILING=1
rem Define Profiler either by CLSID:
set CORECLR_PROFILER={BE4D1E40-1A9F-45E4-BCE4-68DE191CC8D6}
rem ...or by ProgId name:
set CORECLR_PROFILER=Drill4dotNet.DrillProfiler
set CORECLR_PROFILER_PATH=.\Drill4dotNet\bin\x64\Release\Drill4dotNet.dll
@echo Also setting .NET Framework run-time configuration options...
set COR_ENABLE_PROFILING=%CORECLR_ENABLE_PROFILING%
set COR_PROFILER=%CORECLR_PROFILER%
set COR_PROFILER_PATH=%CORECLR_PROFILER_PATH%
@echo Registering COM component...
regsvr32 -s "%CORECLR_PROFILER_PATH%" || echo Registration failed.
@echo Setting compatibility for .NET Framework 4.x due to use outdated profiler...
set COMPlus_ProfAPI_ProfilerCompatibilitySetting=EnableV2Profiler
