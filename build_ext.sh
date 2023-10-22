rm -rf ./addons/sourcemod/extensions

cd extension
rm -rf ./build
docker compose build
docker compose run extension-build --remove-orphans

mkdir -p ../addons/sourcemod/extensions
cp build/voicemanager.ext.2.sdk2013/voicemanager.ext.2.sdk2013.so ../addons/sourcemod/extensions/voicemanager.ext.2.sdk2013.so
cp build/voicemanager.ext.2.tf2/voicemanager.ext.2.tf2.so ../addons/sourcemod/extensions/voicemanager.ext.2.tf2.so