               €ﬂﬂﬂﬂﬂ€ €€ﬂﬂﬂﬂﬂ‹ €€ﬂﬂﬂﬂﬂ€ €€      €ﬂﬂﬂﬂﬂ€ €€    €€
               €‹‹‹‹‹€ €€    €€ €€‹‹‹‹‹€ €€      €‹‹‹‹‹€ €€‹‹‹‹‹€
               €    ‹€ €€    €€ €€       €€      €    ‹€    ‹€
               €     € €€‹‹‹‹‹ﬂ €€       €€‹‹‹‹‹ €     €    €€

AdPlay v1.0 by Simon Peter (dn.tlp@gmx.net)
Website: http://dfg.theunderdogs.org/adlib/adplay/

Description:
------------
AdPlay is the DOS port of the AdPlug Winamp input plugin. As opposed to
AdPlug's capability to replay through an OPL2 emulator, AdPlay requires
an installed OPL2 audio board for song replay.

Supported formats:
------------------
AdPlay implements unique file replayers for each supported format in order to
achieve the best possible replay quality. Below is a list of all currently
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
window, explaining all available key functions.

You can also invoke AdPlay by giving it a file to play as commandline
parameter, which will put AdPlay into background play mode, meaning it
will immediately return to DOS, playing the file in the background.

Configuration:
--------------
The only available configuration is the color configuration, which is done
through the file colors.ini, which should be placed inside AdPlay's
program directory. You can refer to the default colors.ini file for help
on how to create your own color configuration. Add color shemes by adding
a new section into colors.ini. The file works like any other standard
Microsoft INI File.

Known Problems:
---------------
There is no support for MID, CMF, LAA & SCI files, known from AdPlug. I'm
working on fixing the corresponding replayers to make them work under DOS.
Please be patient.

In a Windows 95 DOS Box, there are multiple problems with AdPlay and it
is discouraged to use AdPlay from within Windows - Windows users should
retain to AdPlug instead. Particulary, it is known that returning from a
DOS Shell will destroy the sound and the SA2 replayer will hang the system
while within a Windows 95 DOS Box.

Legal:
------
AdPlay is released under the terms and conditions of the LGPL. A recent copy
of the LGPL can be found at http://www.gnu.org/copyleft/lesser.html.

Release History:
----------------
- v1.0 (2001-06-01)
  - AdPlay is open-source now!
  - Using PMODE/W stub
  - Brought up-to-date with AdPlug 0.10, except U6M player
  - New timer routine
- v1.0 BETA (2001-02-17)
  - First beta release.
