



                    =----------------------------------=
                     gcube - Nintendo Gamecube emulator
                    =----------------------------------=
												 


gcube is a gamecube emulator for linux, windows and mac os x.
Compatibility is still very low so don't expect much.



Software requirements:
----------------------
* SDL library 1.2.7 or newer (won't work with older versions):
  www.libsdl.org
* zlib:
  www.zlib.org
* GNU C compiler


Hardware requirements:
----------------------
* Something good. I'm developing it on Athlon XP 2.0 with 0.5 GB ram and
  GeForce 2 GTS, and that is not enough for a full-speed game of pong;)
* Video card must support the follwing extensions:
  - GL_EXT_texture_rectangle or GL_NV_texture_rectangle
  - GL_IBM_texture_mirrored_repeat


Building gcube:
---------------
* Before compiling check Makefile for options.
* To compile just type 'make' or 'make release' (optimized)
  (on windows use cygwin and type make -f Makefile.win, and on mac use
  make -f Makefile.mac).
* To install, copy gcube to /usr/local/bin.


Usage tips:
-----------
* If You have binfmt_misc support compiled into kernel,
  copy gcube to /usr/local/bin and put this in some
  startup script (eg rc.local):
    echo ":DOL:E::dol::/usr/local/bin/gcube:" >/proc/sys/fs/binfmt_misc/register
    echo ":GCM:E::gcm::/usr/local/bin/gcube:" >/proc/sys/fs/binfmt_misc/register
    echo ":IMP:M::\x7fIMP::/usr/local/bin/gcube:" >/proc/sys/fs/binfmt_misc/register
  You will then be able to run gc-binary files / mini-dvd images
  just like any other executables.
* The easiest way to use it on windows is to make a shortcut to gcube
  on the desktop and then just drop Your gc-binary files on it. You can
  also associate gcube with dol / gcm files.


A few notes:
----------------------------------------
* Some games will need specific settings to run. Run gcube --help to
  see the list of parameters. If the game doesn't work, try starting it
  with hle enabled (-ls) or bigger refresh delay (-r value, eg. 900000).
  HLE might also help if the input isn't working. If the games stop trying
  to play a movie, try running it with -i1 or -i2.
* GCM's can be compressed with the supplied utility isopack. Compression
  uses a very simple method but it is quite effective with some dvd images.
  For example, size of 'The Legend Of Zelda - Four Swords' decreases from
  1.4G to 255M. Other files might not compress that well (if at all).
* In order to run most of the SDK demos, a directory containing files needed
  by the demo will have to be mounted as the root directory of dvd.
  If a directory named 'dvdroot' is present in the same path as the 'elf' file
  it will be mounted automagicly. Otherwise, use the --mount option to
  specify the needed dvdroot.
* Only interpreter is implemented at the moment. That means it is slow
  and You might have to wait a long time before something happens
  (eg action replay and xrick).
* Debugger is integrated with emulator, so it will always pop up
  whenever a fatal error occurs. Pressing F11 (or using 'x' command)
  will run the program ignoring the error (if further execution is
  at all possible). F12 will quit emulator.
* Configuration file is kept in users home directory
  ("/home/username/.gcube-0.3" on Linux and
	 "/documents and settings/username/.gcube-0.3" on windows).
* Default keys are:
    arrows                      - digital pad
    keypad 8/5/4/6              - first analog
    home/end/delete/page down   - second analog
    q/w/a/s/z/x/c               - A/B/L/R/X/Y/Z

  F1 will disable/enable texture caching (slow workaround until correct texture
     caching will be implemented).
  F2 will switch wireframe mode on / off.
	F4 forces linear filtering on all textures.
	F5 makes all textures transparent. use if one texture occludes the view.
	F9 will create a screenshot.


Credits...
----------
Big thanks goes to:
* Dolwin authors, org and hotquick. If they wouldn't release
  sources of their emulator, gcube wouldn't exist.
* gropeaz / hitmen for Yet Another Gamecube Documentation,
  also to everyone who contributed to it in any way.
* Frank Willie (phx) for PowerPC disassembler.
* Every GC homebrew developer releasing sources.
* Metalmurphy for gcube homepage and EXEmu.net staff for hosting it.
* Adam Green for helping me out with the Mac OS X port.

* Great icons by rodimus:
  www.rodimusconvoy.com


  Keep in mind this is a beta-quality product. It hasn't been throughoutly
tested and may contain bugs. Patches and suggestions are always welcome.

  If You want to report a bug, ask a question or share a solution to a
common problem, check out gcube section at www.emuboards.net (thanks to
the courtesy of EXEmu.net staff).

Be sure to checkout the gcube homepage:
  http://gcube.exemu.net

monk@mad.scientist.com

monk
