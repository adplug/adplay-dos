               €ﬂﬂﬂﬂﬂ€ €€ﬂﬂﬂﬂﬂ‹ €€ﬂﬂﬂﬂﬂ€ €€      €ﬂﬂﬂﬂﬂ€ €€    €€
               €‹‹‹‹‹€ €€    €€ €€‹‹‹‹‹€ €€      €‹‹‹‹‹€ €€‹‹‹‹‹€
               €    ‹€ €€    €€ €€       €€      €    ‹€    ‹€
               €     € €€‹‹‹‹‹ﬂ €€       €€‹‹‹‹‹ €     €    €€

AdPlay/DOS 1.4, Copyright (c) 2000 - 2003 Simon Peter <dn.tlp@gmx.net>
Website: http://adplug.sourceforge.net

Description:
------------
AdPlay is AdPlug's MS-DOS based frontend. Sporting an exclusively programmed
textmode interface, it provides a convenient playback experience. AdPlay
requires an installed OPL2 audio board for song replay. No emulated playback
is supported.

Supported formats:
------------------
AdPlug implements unique file replayers for each supported format in order to
achieve the best possible replay quality. Below is a list of all currently
supported file formats along with information about possible replay issues.
Players marked as "preliminary" aren't considered final by the author(s) and
may contain many replay issues, but are included for testing purposes anyway.

- A2M: AdLib Tracker 2 by subz3ro
  - Unimplemented commands (version 1-4): FF1 - FF9, FAx - FEx
  - Unimplemented commands (version 5-8): Gxy, Hxy, Kxy - &xy
  - In version 5-8 files, instrument panning & the flags byte is ignored
  - Only SixPack compressed and uncompressed files are supported
  - All OPL3 functionality is absent (not possible with AdLib)
- AMD: AMUSIC Adlib Tracker by Elyssis
- BAM: Bob's Adlib Music Format by Bob
- CFF: BoomTracker 4.0 by CUD
- CMF: Creative Music File Format by Creative Technology
  - Unimplemented: AdLib rhythm mode
- D00: EdLib by Vibrants
  - Bugs: Hard restart SR sometimes sound wrong
- DFM: Digital-FM by R.Verhaag
- DMO: Twin TrackPlayer by TwinTeam
- DTM: DeFy Adlib Tracker by DeFy
- HSC: HSC Adlib Composer by Hannes Seifert, HSC-Tracker by Electronic Rats
- HSP: HSC Packed by Number Six / Aegis Corp.
- IMF: Apogee IMF File Format by Apogee
- KSM: Ken Silverman's Adlib Music Format by Ken Silverman
  - Needs file 'insts.dat' in the same directory as loaded file
- LAA: LucasArts AdLib Audio File Format by LucasArts
  - Bugs: Some volumes are a bit off
- LDS: LOUDNESS Music Format (preliminary)
  - Bugs: Undocumented format, more to come.
- M: Ultima 6 Music Format by Origin
- MAD: Mlat Adlib Tracker
- MID: MIDI Audio File Format
- MKJ: MKJamz by M \ K Productions (preliminary)
  - Bugs: Still many replay flaws
- MTK: MPU-401 Trakker by SuBZeR0
- RAD: Reality ADlib Tracker by Reality
- RAW: RdosPlay RAW file format by RDOS
  - Unimplemented: OPL3 register writes (not possible with AdLib)
- ROL: AdLib Visual Composer by AdLib Inc.
  - Needs file 'standard.bnk' in the same directory as loaded file
    (be careful under UNIX - it's case sensitive!)
- S3M: Scream Tracker 3 by Future Crew
  - Bugs: Extra Fine Slides (EEx, FEx) & Fine Vibrato (Uxy) are inaccurate
- SA2: Surprise! Adlib Tracker 2 by Surprise! Productions
- SAT: Surprise! Adlib Tracker by Surprise! Productions
- SCI: Sierra's AdLib Audio File Format by Sierra On-Line Inc.
  - Needs file '???patch.003' in the same directory as loaded file, where
    '???' are the first 3 characters of the loaded file
  - Bugs: Some instruments are messed up
- SNG: SNGPlay by BUGSY of OBSESSION
- SNG: Faust Music Creator by FAUST
- SNG: Adlib Tracker 1.0 by TJ
- XAD: eXotic ADlib Format by Riven the Mage
- XMS: XMS-Tracker by MaDoKaN/E.S.G

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

Credits:
--------
AdPlay/DOS is completely programmed by Simon Peter. It depends on the AdPlug
audio library, the WATCOM textmode window class library and the OpenCP WATCOM
timer library, which are all also copyrighted by Simon Peter. The OpenCP
WATCOM timer library is also copyrighted by Niklas Beisert, et al.

All documentation is written by Simon Peter.

The screen layouts Default, HighRes, LowRes, Monochrome and Analyzer,
provided inside the INI file, are created by Simon Peter. The layouts Arctic
and Cherry are created by Death Adder <death-adder@juno.com>.

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
- 1.4 (2003-03-16)
  - Linked with AdPlug 1.4
  - File selector can now group files by extension, as an option.
  - Files supported by AdPlug are now displayed in another color.
  - The length of the playing song is now displayed in the song info window.

- 1.3 (2002-11-21)
  - Linked with AdPlug 1.3 + patches
  - Colorized & sorted file selector
  - Enhanced on-line help
  - Lots of navigational changes (more MS Windows like)
  - More screen layouts

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
