               €ﬂﬂﬂﬂﬂ€ €€ﬂﬂﬂﬂﬂ‹ €€ﬂﬂﬂﬂﬂ€ €€      €ﬂﬂﬂﬂﬂ€ €€    €€
               €‹‹‹‹‹€ €€    €€ €€‹‹‹‹‹€ €€      €‹‹‹‹‹€ €€‹‹‹‹‹€
               €    ‹€ €€    €€ €€       €€      €    ‹€    ‹€
               €     € €€‹‹‹‹‹ﬂ €€       €€‹‹‹‹‹ €     €    €€

AdPlay v1.0 by Simon Peter (dn.tlp@gmx.net) et al.
Website: http://adplug.sourceforge.net/dos.shtml

Description:
------------
AdPlay is AdPlug's MS-DOS based frontend. Sporting an exclusively
programmed textmode interface, it provides a convenient playback
experience. As opposed to AdPlug's capability to replay through
an OPL2 emulator, AdPlay requires an installed OPL2 audio board
for song replay.

Supported formats:
------------------
AdPlay implements unique file replayers for each supported format in order to
achieve the best possible playback quality. Below is a list of all currently
supported file formats along with information about possible replay issues.

- A2M: AdLib Tracker 2 by subz3ro
  - Unimplemented commands (version 1-4): FF1 - FF9, FAx - FEx
  - Unimplemented commands (version 5-8): Gxy, Hxy, Kxy - &xy
  - In version 5-8 files, instrument panning & the flags byte is ignored
  - Only SixPack or uncompressed files are supported
- AMD: AMUSIC Adlib Tracker by Elyssis
- CMF: Creative Music File Format by Creative Technology
  - Unimplemented: AdLib rhythm mode
- D00: EdLib by Vibrants
  - Bugs: Hard restart SR sometimes sound wrong
- HSC: HSC Adlib Composer by Hannes Seifert, HSC-Tracker by Electronic Rats
- HSP: HSC Packed by Number Six / Aegis Corp.
- IMF: Apogee IMF File Format
- KSM: Ken Silverman's Music Format
- LAA: LucasArts AdLib Audio File Format by LucasArts
  - Bugs: Some volumes are a bit off
- MID: MIDI Audio File Format
- MTK: MPU-401 Trakker by SuBZeR0
- S3M: Screamtracker 3 by Future Crew
  - Bugs: Extra Fine Slides (EEx, FEx) & Fine Vibrato (Uxy) are inaccurate
- SCI: Sierra's AdLib Audio File Format
  - Bugs: Some instruments are messed up
- SNG: SNGPlay by BUGSY of OBSESSION
- RAD: Reality ADlib Tracker by Reality
- RAW: RdosPlay RAW file format by RDOS
  - Unimplemented: OPL3 register writes (not possible with AdLib)
- SA2: Surprise! Adlib Tracker 2 by Surprise! Productions
  - Unimplemented: Version 7 files (i don't have any)

Usage:
------
AdPlay is normally started without any commandline parameters, bringing it
into interactive mode where you select the files to play using a file
selector within a graphical user interface. Select files using the Up/Down
cursor keys and press enter to start playback. F1 will bring up a help
window, explaining all other available key functions.

You can also invoke AdPlay in the so called batch mode, which will turn it
into a console background playback utility. Use the '-b <file>' commandline
parameter to start AdPlay in this way, replacing <file> with the desired
file to be played back.

Other interesting parameters are the '-f' and '-c' options, which will load
another configuration file and/or section. You can stack up as many of
these parameters, as you like. For example, use '-c LowRes -c Monochrome'
to turn the interface to low resolution mode, in monochrome. With any new
configuration file loaded, the 'default' section will always be processed
first.

To get a list with short descriptions of all available commandline
parameters in AdPlay, use the '-?' or '-h' commandline options. All options
can be set with '-' or '/' respectively.

Configuration:
--------------
AdPlay can be configured through the configuration file adplay.ini, which
should be placed inside AdPlay's program directory. You can refer to the
default adplay.ini file for help on how to create your own configuration
sheme. Add more configuation shemes by adding a new section. The file
works like any other standard Microsoft INI file.

Known Problems:
---------------
Returning from a DOS shell in a Windows 95 DOS Box will destroy the sound.

Shelling to DOS while playing back with high timer rates can crash the
player while within real DOS.

Legal:
------
AdPlay is released under the terms and conditions of the LGPL. A recent copy
of the LGPL can be found at http://www.gnu.org/copyleft/lesser.html.

Release History:
----------------
- v1.0 (2001-06-30)
  - AdPlay is open-source now!
  - Using PMODE/W stub
  - Linked with AdPlug 1.0 core release, excluding U6M player
  - New timer routine
  - New, customizable screen layout
- v1.0 BETA (2001-02-17)
  - First beta release.
