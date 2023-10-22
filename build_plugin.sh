rm -rf ./addons/sourcemod/plugins
mkdir addons/sourcemod/plugins

./spcomp addons/sourcemod/scripting/voicemanager.sp -i/usr/include -iaddons/sourcemod/scripting/include -o addons/sourcemod/plugins/voicemanager.smx