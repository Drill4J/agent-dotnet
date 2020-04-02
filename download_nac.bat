setlocal
set target_dir=%~dp0.\dependencies\native-agent-connector
if not exist "%target_dir%" mkdir "%target_dir%"
del /s/f/q "%target_dir%\*.*"

set artifactory_url=https://oss.jfrog.org/artifactory/list/oss-release-local/com/epam/drill/dotnet

call :download debug Debug
call :download release Release
goto :eof

:download
set config1=%1
set config2=%2
set nac_dir=agent_connector-mingwX64-%config1%
set nac_version=0.1.0
set nac_zip=agent_connector-mingwX64-%config1%-%nac_version%.zip
set url=%artifactory_url%/%nac_dir%/%nac_version%/%nac_zip%
set zipfile=%target_dir%\%nac_zip%
curl --output "%zipfile%" %url%

set header=mingwX64%config2%-%nac_version%/agent_connector_api.h
set dll=mingwX64%config2%-%nac_version%/agent_connector.dll
set def=mingwX64%config2%-%nac_version%/agent_connector.def

unzip -n "%zipfile%" %header% %dll% %def% -d "%target_dir%"
exit /B

