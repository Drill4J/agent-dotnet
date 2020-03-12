@if %1s==s (
set Configuration=Release
) else (
set Configuration=%1
)
@echo Setting .NET Core runtime configuration options for profiling...
set COMPlus_EnableDiagnostics=1
set CORECLR_ENABLE_PROFILING=1
set CORECLR_PROFILER={BE4D1E40-1A9F-45E4-BCE4-68DE191CC8D6}
set CORECLR_PROFILER_PATH=%~dp0.\bin\x64\%Configuration%\Drill4dotNet\Drill4dotNet.dll
@echo Also setting .NET Framework runtime configuration options...
set COR_ENABLE_PROFILING=%CORECLR_ENABLE_PROFILING%
set COR_PROFILER=%CORECLR_PROFILER%
set COR_PROFILER_PATH=%CORECLR_PROFILER_PATH%
@echo Setting compatibility since .NET Framework 4.x rejects to launch a profiler with a deprecated interface...
set COMPlus_ProfAPI_ProfilerCompatibilitySetting=EnableV2Profiler
