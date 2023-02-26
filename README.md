# voicemanager

Prerequisites:
- Docker

To build in Docker, run:
`./scripts/build.bat`

To setup C++ includes for VS Code, clone sourcemod, metamod, and the tf2 sdk. Include the following paths:
- ./
- ./include
- <sm_path>
- <sm_path>\public
- <sm_path>\public\extensions
- <sm_path>\sourcepawn\include
- <sm_path>\sourcepawn\third_party\amtl
- <sm_path>\sourcepawn\third_party\amtl\amtl
- <mm_path>\core
- <mm_path>\core\sourcehook
- <hl2sdk-tf2_path>\public
- <hl2sdk-tf2_path>\public\tier0
- <hl2sdk-tf2_path>\public\tier1