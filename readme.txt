               €ﬂﬂﬂﬂﬂ€ €€ﬂﬂﬂﬂﬂ‹ €€ﬂﬂﬂﬂﬂ€ €€      €ﬂﬂﬂﬂﬂ€ €€    €€
               €‹‹‹‹‹€ €€    €€ €€‹‹‹‹‹€ €€      €‹‹‹‹‹€ €€‹‹‹‹‹€
               €    ‹€ €€    €€ €€       €€      €    ‹€    ‹€
               €     € €€‹‹‹‹‹ﬂ €€       €€‹‹‹‹‹ €     €    €€

AdPlay/DOS 1.2, (c) 2001, 2002 Simon Peter <dn.tlp@gmx.net>
Website: http://adplug.sourceforge.net

Description:
------------
AdPlay is AdPlug's MS-DOS based frontend. Sporting an exclusively programmed
textmode interface, it provides a convenient playback experience. AdPlay
requires an installed OPL2 audio board for song replay. No emulated playback
is supported.

Usage:
------
AdPlay is normally started without any commandline parameters, bringing it
into interactive mode where you select the files to play using a file selector
within a graphical user interface. Select files using the Up/Down cursor keys
and press enter to start playback. F1 will bring up a help window, explaining
all other available key functions.

You can also invoke AdPlay in the so called batch mode, which will turn it
into a console background playback utility. Use the '-b <file>' commandline
parameter to start AdPlay in this way, replacing <file> with the desired file
to be played back.

Other interesting parameters are the '-f' and '-c' options, which will load
another configuration file and/or section. You can stack up as many of these
parameters, as you like. For example, use '-c lowres -c monochrome' to turn
the interface to low resolution mode, in monochrome. With any new
configuration file loaded, the 'default' section of that file will always be
processed first.

To get a list with short descriptions of all available commandline parameters
in AdPlay, use the '-?' or '-h' commandline options. All options can be set
with '-' or '/' respectively.

Configuration:
--------------
AdPlay can be configured through the configuration file "adplay.ini", which
should be placed inside AdPlay's program directory. You can refer to the
default adplay.ini file for help on how to create your own configuration
scheme. Add more configuation schemes by adding a new section. The file works
much in a way like a standard Microsoft INI file.

Known Problems:
---------------
Returning from a DOS shell in a Windows 95 DOS Box will destroy the sound.

Shelling to DOS while playing back with high timer rates can crash the
player while within real DOS.

Legal:
------
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Release History:
----------------
- 1.2 (2002-07-30)
  - Linked with AdPlug 1.2, excluding U6M & ROL players
  - Disk drive selection support
  - Fixed ZIP file support
  - Lots of other fixes
- v1.1 (2001-10-25)
  - Linked with AdPlug 1.1
  - .ZIP file reading support
- v1.0 (2001-06-30)
  - AdPlay is open-source now!
  - Using PMODE/W stub
  - Linked with AdPlug 1.0 core release, excluding U6M player
  - New timer routine
  - New, customizable screen layout
- v1.0 BETA (2001-02-17)
  - First beta release
