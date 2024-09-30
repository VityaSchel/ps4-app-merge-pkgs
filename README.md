# PlayStation 4 app to merge pkgs on the console itself

For those of us who only have 32 gb flash drive and too slow poor router for remote game transferring...

![Screenshot](https://github.com/user-attachments/assets/b2c619ee-6c5e-4d0e-bd11-3263f3ccb30a)

## How to use it

1. Download and install the merger itself on your PS4 (IV0000-PKGM40924_00-MERGEFILES000000.pkg) in [releases](https://github.com/VityaSchel/ps4-app-merge-pkgs/releases)
2. Download the `splitter` binary for your OS in [releases](https://github.com/VityaSchel/ps4-app-merge-pkgs/releases), drag it to your terminal and as a paramter drag the pkg you want to split. 
   - This program will output splitted pkg files in 15 gb chunks in the current terminal directory. 
   - For example, this is the resulting command to split detroit become human game: `/Users/me/Downloads/splitter /Users/me/Downloads/Detroit.Become.Human.pkg` (assuming you've downloaded splitter to your Downloads directory along with Detroit.Become.Human.pkg)
   - You can adjust this size by providing `-c` option which is chunk size in MEGAbytes. For example, `./splitter -c 30000 ./Detroit.Become.Human.pkg` splits pkg in 30 gb chunks (30 * 1000)
   - After starting splitter, wait patiently and look if .pkgpart files started to appear in the directory where you run this command.
3. Transfer these splitted parts to your flash drive
4. After you plug-in your flash drive to your ps4, you have to move these splitted parts to very specific directory: /data/pkg_merger. **It won't exist by default, you must create it in /data directory**. MAKE SURE TO WRITE `pkg_merger` CORRECT! Otherwise this app won't be able to see your splitted pkgs. To copy from usb to /pkg/merger, you should use ps4 xplorer 2.0 from homebrew store, nothing else worked for me, for example FTP simply does not allow you to move files from usb to hdd.
   1. Go into /mnt/usb0 (usually usb0 but can also be usb1, usb2 and so on). In ps4 xplorer you can just press "left" on your D-PAD to instantly open mounted usb directory.
   2. Send these splitted files from /mnt/usb0 to /data/pkg_merger. This should take about the same time that took you transferring these files from your pc to flash drive, i.e. 60 mb/s.
5. Repeat 3 and 4 step for every splitted part of your game. Make sure to not rename files as the app expects them to have _001, _002 and similar suffixes.
6. Open the app on your ps4 and it should see all files in /data/pkg_merger. Verify that all parts are there on screen in the list, press any button on controller as prompted, confirm merging and the app will start to merge them into single one and place it into internal harddrive (/data/pkg). You must have at least double size of your pkg remaining free space on ps4. The app does not keep parts in RAM, it only has 1 mb buffer to concat parts as stream of bytes.
7. Finally, the resulted file should appear in goldhen debug installer menu (if you selected Debug Settings -> Package source: all or hdd), install it as usual.

## Compiling source code

### PS4 app

1. Download orbis toolkit
2. Modify Makefile so that it fits your system's environment (especially CC, CCX, LD, CDIR) section
3. Run `make` in root directory of repository

### Splitter

1. Download cmake
2. Go to splitter/build and run `cmake ..`
3. Run `make` in the same directory

## Troubleshootig

### Errors when running splitter such as "illegal instruction"

Most likely it's because splitter wasn't built for your OS, you must build it from source or use any other splitting program out there, just make sure that it splits exactly in sorted parts without modifying parts contents and that parts are named properly (all have basename + _001.pkgpart suffix and so on)

### "Errored" when transferring files from usb to hdd in ps4 xplorer

That happens sometimes and I have no idea why. The better approach would be to eliminate need for ps4 xplorer at all and read parts directly from usb and write to resulting file, but I tried and it didn't work with sandboxed app access, and I have no idea how to use root access with orbis toolchain.

What can you do? Try to rebuild splitter/main.cpp and change chunk size. Sometimes it helps, for example decrease from 5 gb to 2 gb and transfer new parts to your ps4.

### My ps4 won't respond to any interactions

This just happens.

Now beg for your hdd to not be corrupted and hold power button for 10-30 seconds, until you hear beep. Restart your ps4 shortly after and either get shameful message about how correct ps4 shutdown should be made from its menu or terrifying message that your hdd is corrupted.

### Trying to install resulted pkg errors with CE-38603-0

Go to notifictions -> find the game you're trying to install in list and remove stuck installation in the list.

Source: https://www.reddit.com/r/ps4homebrew/comments/hybr5z/solved_error_ce386030/

## Support me

[hloth.dev/donate](https://hloth.dev/donate)
