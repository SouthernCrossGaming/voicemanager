# Voice Manager

A sourcemod plugin and extension that allows players to individually modify the voice volume of other players. 

## How to use
TODO

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

## How it works
Player adjustments are made via a sourcemod menu by 

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
$ .\build.bat
```

# Credits
- [Fraeven](https://fraeven.dev) (Extension Code, Plugin Code, Testing)
- [Rowedahelicon](https://rowdythecrux.dev) (Plugin Code, Debugging Assistance, Testing)
- Many members of the SCG community (Testing)