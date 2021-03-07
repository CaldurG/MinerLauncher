# MinerLauncher
Automatically stop mining when game is launched and start mining after it stopped. Also supports switching GPU profile via MSI Afterburner.

## Startup
I recommend launching this from scheduled task on user logon. Don't forget to run it as Admin ("Run with highest privileges") since it won't be able to switch MSI Afterburner profiles otherwise. Example task MinerLauncher.xml and screenshots with task creation can be found in "Task" folder.

## Config
See MinerLauncher.ini file to configure your games, miners and Afterburner profiles. MinerLauncher.ini must be placed in the same directory as MinerLauncher.exe.
