# Voice Manager

<div align="center">
  A sourcemod plugin and extension that allows players to individually modify the voice volume of other players. 
  </br>
  </br>
  <h3>View Demo on YouTube
  </br>
  </br>
  	
  [![Demo](https://i3.ytimg.com/vi/5lFNonAkXDQ/maxresdefault.jpg)](https://youtu.be/5lFNonAkXDQ "Voice Manager Sourcemod Extension and Plugin Demo")
  </h3>
</div>

## How to use
A player can type the command `/vm` into chat to display a menu that allows them to set volume overrides for players in the server. Overrides can be set for invidual players or globally for all players (individual overrides will take precedence).

![image](https://github.com/SouthernCrossGaming/voicemanager/assets/20617130/b882ee1c-3e8d-4ca4-94db-0448c03f876a)

When a volume adjustment is made, all voice communications from that player will be adjusted accordingly.

There are currently 5 volume levels that can be selected:

![image](https://github.com/SouthernCrossGaming/voicemanager/assets/20617130/171bb8bf-4a6c-4e0b-a7eb-fb970ec07137)

## Compatible Games
- Team Fortress 2
- Open Fortress

## Supported Platforms
- Linux

## Sourcemod
- Version 1.10+

## Installation
Download the [latest release](https://github.com/SouthernCrossGaming/voicemanager/releases/latest/download/voicemanager.zip), unzip and copy to your `addons` directory.

Add a configuration for the voice manager database to your `addons/sourcemod/configs/databases.cfg` file.

For example:
```
	"voicemanager"
	{
		"driver" "sqlite"
		"database" "voicemanager"
	}
```

## Configuration
`vm_enabled` - Enables or disables voice manager (0/1, default 1)  
`vm_database` - Database configuration to use from databases.cfg (default "voicemanager")  
`vm_allow_self` - Allow players to override their own volume. This is recommended only for testing (0/1, default 0) 

## Building

### Build Extension

<b>Requirements:</b>
- Docker

<b>Windows</b>
```
$ .\build_ext.bat
```

<b>Linux</b>
```
$ .\build_ext.sh
```

### Build Plugin
<b>Requirements:</b>
- spcomp (1.11 or higher) added to your path or to the base directory

<b>Windows</b>
```
$ .\build_plugin.bat
```
<b>Linux</b>
```
$ .\build_plugin.sh
```

### Build Both (same requirements as above)

<b>Windows</b>
```
$ .\build.bat
```

<b>Linux</b>
```
$ .\build.sh
```

# Credits
- [Fraeven](https://fraeven.dev) (Extension Code, Plugin Code, Testing)
- [Rowedahelicon](https://rowdythecrux.dev) (Plugin Code, Debugging Assistance, Testing)
- Many members of the SCG community (Testing)
