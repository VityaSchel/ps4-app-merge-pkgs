# PlayStation 4 app to merge pkgs on the console itself

For those of us who only have 32 gb flash drive and too slow poor router for remote game transferring...

## How to use it

1. Download and install the merger itself on your PS4 (IV0000-PKGM40924_00-MERGEFILES000000.pkg) in [releases](https://github.com/VityaSchel/ps4-app-merge-pkgs/releases)
2. Download the `splitter` binary for your OS in [releases](https://github.com/VityaSchel/ps4-app-merge-pkgs/releases), drag it to your terminal and as a paramter drag the pkg you want to split. This program will output splitted pkg files by 15 gb chunks in the current terminal directory.
3. Transfer these splitted parts to your flash drive
4. After you plug-in your flash drive to your ps4, you have to move these splitted parts to very specific directory: /data/pkg_merger. **It won't exist by default, create it by yourself in /data directory**. MAKE SURE TO WRITE `pkg_merger` CORRECT! Otherwise this app won't be able to see your splitted pkgs. To do that, you'll have to use FTP, so go ahead and turn on that in goldhen as usual and connect from your PC. 
   1. Go into /mnt/usb0 (usually usb0 but can also be usb1, usb2 and so on). 
   2. Drag these splitted files from /mnt/usb0 to /data/pkg_merger. Don't worry, this will take <0.1s
5. Repeat 3 and 4 step for every splitted part of your game. Make sure to not rename files as the app expects them to have _001, _002 and similar suffixes.
6. Open the app on your ps4 and it should see all files in pkg_merger, merge them into single one and place it into internal harddrive. You must have at least double size of your pkg remaining free space on ps4.
7. Finally, the resulted file should appear in goldhen debug installer menu (if you selected Debug Settings -> Package source: all or hdd), install it as usual.