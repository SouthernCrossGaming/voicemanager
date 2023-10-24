DEL addons\sourcemod\extensions\voicemanager.ext.2.sdk2013.dll
DEL addons\sourcemod\extensions\voicemanager.ext.2.tf2.dll

cd extension
rmdir .\build /s /q
docker compose -f docker-compose.windows.yml build
docker compose -f docker-compose.windows.yml run extension-build-windows --remove-orphans

echo f | XCOPY build\voicemanager.ext.2.sdk2013\voicemanager.ext.2.sdk2013.dll ..\addons\sourcemod\extensions\voicemanager.ext.2.sdk2013.dll /Y
echo f | XCOPY build\voicemanager.ext.2.tf2\voicemanager.ext.2.tf2.dll ..\addons\sourcemod\extensions\voicemanager.ext.2.tf2.dll /Y
cd ..