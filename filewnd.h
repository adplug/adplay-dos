/*
 * filewnd.h - Small File Manager for DOS w/ archive support
 * Copyright (c) 2002 Simon Peter <dn.tlp@gmx.net>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <dos.h>
#include <window/window.h>

#include "arcfile.h"

class FileWnd: public CListWnd
{
public:
        // Errors
        enum Error { None, Drive_NotReady };

        // Attributes
        enum Attribute { File, Directory, Drive, Archive };

        FileWnd();
        ~FileWnd();

        void refresh();
        bool select();

        Error geterror();
        char *getfilename(char *fn);
        bool inarchive();
        archive *getarchive();

private:
        Error err;
        int arcmode;
        zipfile arc;
        char attrs[MAXITEMS];
        unsigned long items;
        char dirprefix[PATH_MAX];

        void listfiles();
        void listdrives();
        void listarc(archive &a);
        char *extract(char *newfn, archive &a, char *oldfn);
        unsigned int drivenum(char *fname);
};
