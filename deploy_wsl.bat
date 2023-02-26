wsl -e cp /mnt/c/ws/voicemanager/build/voicemanager.ext.2.tf2/voicemanager.ext.2.tf2.so /home/tf2_server/hlserver/tf/addons/sourcemod/extensions/voicemanager.ext.2.tf2.so
wsl -e cp /mnt/c/ws/voicemanager/addons/sourcemod/gamedata/voicemanager.txt /home/tf2_server/hlserver/tf/addons/sourcemod/gamedata/voicemanager.txt
wsl -e pkill srcds_linux
wsl -e ./home/tf2_server/tf2.sh