wsl -e cp /mnt/c/ws/voicemanager/build/voicemanager.ext.2.sdk2013/voicemanager.ext.2.sdk2013.so /home/of_server_linux/sdk/open_fortress/addons/sourcemod/extensions/voicemanager.ext.2.sdk2013.so
wsl -e cp /mnt/c/ws/voicemanager/addons/sourcemod/gamedata/voicemanager.txt /home/of_server_linux/sdk/open_fortress/addons/sourcemod/gamedata/voicemanager.txt
wsl -e cp /mnt/c/ws/voicemanager/dist/plugins/voicemanager.smx /home/of_server_linux/sdk/open_fortress/addons/sourcemod/plugins/voicemanager.smx
wsl -e pkill srcds_linux
wsl -e /home/of_server_linux/start.sh