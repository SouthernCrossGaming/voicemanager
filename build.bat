"C:\Program Files\Docker\Docker\DockerCli.exe" -SwitchLinuxEngine
CALL .\build_ext.bat
"C:\Program Files\Docker\Docker\DockerCli.exe" -SwitchWindowsEngine
CALL .\build_ext_windows.bat

CALL  .\build_plugin.bat