# Voice Manager

<div align="center">
  A sourcemod plugin and extension that allows players to individually modify the voice volume of other players. 
  </br>
  </br>
  <h3><a href=https://youtu.be/5lFNonAkXDQ>View Demo on YouTube</a>
  </br>
  </br>
  	
  [![Demo](https://i3.ytimg.com/vi/5lFNonAkXDQ/maxresdefault.jpg)](https://youtu.be/5lFNonAkXDQ "Voice Manager Sourcemod Extension and Plugin Demo")
  </h3>
</div>

## How To Use
A player can type the command `/vm` into chat to display a menu that allows them to set volume overrides for players in the server. Overrides can be set for invidual players or globally for all players (individual overrides will take precedence).

![277191737-b882ee1c-3e8d-4ca4-94db-0448c03f876a](https://github.com/SouthernCrossGaming/voicemanager/assets/20617130/be666240-5ae6-42d3-9073-d8ebdb23d9ac)

When a volume adjustment is made, all voice communications from that player will be adjusted accordingly.

There are currently 5 volume levels that can be selected:

![277191747-171bb8bf-4a6c-4e0b-a7eb-fb970ec07137](https://github.com/SouthernCrossGaming/voicemanager/assets/20617130/66ae51c2-fb66-4315-8a07-16052b76a2fa)

## Requirements

### Supported Games*
- Team Fortress 2
- Open Fortress

<sub>* Voice Manager would likely work with any game that supports the steam voice codec. I would be happy to add support for other games by request so long as testing assistance for said game is provided.</sub>

### Supported Platforms
- Linux
- Windows

### Sourcemod
- Version 1.10+

### Supported Database Drivers
- mysql
- sqlite
  
### Supported Voice Codecs
- steam (`sv_voicecodec steam`)

## Installation
Download the [latest release](https://github.com/SouthernCrossGaming/voicemanager/releases/latest/download/voicemanager.zip), unzip and copy to your `addons` directory.

Add a configuration for the voice manager database to your `addons/sourcemod/configs/databases.cfg` file. Note that voice manager will use the "default" configuration by default, but this can be configured via cvar, see below.

## Configuration
`vm_enabled` - Enables or disables voice manager (0/1, default 1)  
`vm_database` - Database configuration to use from databases.cfg (default is "default")  
`vm_allow_self` - Allow players to override their own volume. This is recommended only for testing (0/1, default 0) 

## Commands
`/vm` | `/voicemanager` - Opens the Voice Manager menu  
`/vmclear` - Clears the player's overrides from the database

## Building

### Build Extension for Linux

*Requires Docker (with docker compose) using <b>Linux</b> containers
```
> .\build_ext.bat
```
```
$ ./build_ext.sh
```

### Build Extension for Windows

*Requires Windows environment with Docker (with docker compose) using <b>Windows</b> containers
```
> .\build_ext_windows.bat
```

### Build Plugin
*Requires spcomp (1.10 or higher) added to your path or to the base directory
```
> .\build_plugin.bat
```
```
$ ./build_plugin.sh
```

### Build All (Windows environment only)
```
> .\build.bat
```

## VS Code Setup
To setup C++ includes for VS Code, clone sourcemod, metamod, and the tf2 sdk. Include the following paths:
- `./`
- `./include`
- `<sm_path>`
- `<sm_path>\public`
- `<sm_path>\public\extensions`
- `<sm_path>\sourcepawn\include`
- `<sm_path>\sourcepawn\third_party\amtl`
- `<sm_path>\sourcepawn\third_party\amtl\amtl`
- `<mm_path>\core`
- `<mm_path>\core\sourcehook`
- `<hl2sdk-tf2_path>\public`
- `<hl2sdk-tf2_path>\public\tier0`
- `<hl2sdk-tf2_path>\public\tier1`

### Troubleshooting
- <b>Problem</b>: The VoiceManager plugin/extension did not load.
- <b>Potential Solution</b>: Check for any errors on startup and ensure that an empty `voicemanager.autoload` file exists under the `addons/sourcemod/extension` directory.

</br>

* <b>Problem</b>: When an adjustment is applied, there are no errors but the player does not have any voice output.  
* <b>Potential Solution</b>: Make sure the server is using the `steam` voice codec (`sv_voicecodec steam`)

# Credits
- [Fraeven](https://fraeven.dev) (Extension Code, Plugin Code, Testing)
- [Rowedahelicon](https://rowdythecrux.dev) (Plugin Code, Debugging Assistance, Testing)
- Many members of the SCG community (Testing)

# AlliedModders Thread
https://forums.alliedmods.net/showthread.php?t=344276
