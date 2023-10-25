DEL addons\sourcemod\extensions\voicemanager.ext.2.sdk2013.so
DEL addons\sourcemod\extensions\voicemanager.ext.2.tf2.so

cd extension
rmdir build /s /q
docker compose build
docker compose run extension-build --remove-orphans

echo f | XCOPY build\voicemanager.ext.2.sdk2013\voicemanager.ext.2.sdk2013.so ..\addons\sourcemod\extensions\voicemanager.ext.2.sdk2013.so /Y
echo f | XCOPY build\voicemanager.ext.2.tf2\voicemanager.ext.2.tf2.so ..\addons\sourcemod\extensions\voicemanager.ext.2.tf2.so /Y
cd ..