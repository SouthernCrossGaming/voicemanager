rmdir .\addons\extensions /s /q

cd extension
rmdir .\build /s /q
docker compose build
docker compose run extension-build --remove-orphans

echo f | XCOPY build\voicemanager.ext.2.sdk2013\voicemanager.ext.2.sdk2013.so ..\addons\extensions\voicemanager.ext.2.sdk2013.so
echo f | XCOPY build\voicemanager.ext.2.tf2\voicemanager.ext.2.tf2.so ..\addons\extensions\voicemanager.ext.2.tf2.so