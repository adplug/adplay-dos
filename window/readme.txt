Portable Textmode Window Class Library
Copyright (c) 2000 - 2002, 2006 Simon Peter <dn.tlp@gmx.net>
Version 1.1

Description
-----------
This package contains portable window classes, especially designed for
textmode usage, and the necessary console output back-ends for various
architectures.

Window Classes
--------------
For the moment, there are the following window classes:
CWindow : Basic framed window w/o any special behaviours
CTxtWnd : Window that contains text
CListWnd: Listbox that can have one item selected
CBarWnd	: Window containing vertical bar(s)

The classes can be inherited by you, to extend their functionality.

There is also a window manager class called CWndMan. It has the ability to
redraw all managed windows at once and in the right order.

Console Output Back-Ends
------------------------
The output back-ends provide hardware console output and some special
features, like cursor on/off, clear screen and video mode selection, if that
is supported on a particular architecture.

The following back-ends are provided:

Architecture   Operating System     Compiler
------------   ----------------     --------
x86            MS-DOS               DJGPP

Building
--------
Please read the INSTALL file for build instructions!

Debug Logging
-------------
The library can be compiled to produce some debug logging output. To enable
this, you have to define the DEBUG preprocessor variable. Please refer
to the manual of your respective compiler architecture for information
on how to do this.

The output file for the debug messages is specified with the
DEBUG_FILE preprocessor define in the 'window.cpp' file.

Legal
-----
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

History
-------
Changes from version 1.0 to version 1.1:
- Added support for the DJGPP compiler.
- Miscellaneous fixes (mainly cosmetic).
