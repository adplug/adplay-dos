/*
 * filewnd.h - Small File Manager for DOS w/ archive support
 * Copyright (c) 2002, 2003, 2006 Simon Peter <dn.tlp@gmx.net>
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

        // Colors
        enum FColor { FileSel, FileUnsel, DirSel, DirUnsel, DriveSel,
                DriveUnsel, SupportedSel, SupportedUnsel };

	// Sort Methods
	enum SortMethods { SortByName, SortByExtension };

	SortMethods sortby;

        static void setfilecolor(FColor c, unsigned char v);
        static unsigned char getfilecolor(FColor c) { return fc[c]; }

        FileWnd();
        ~FileWnd();

        void refresh();
        bool select();

        Error geterror();
        char *getfilename(char *fn);
        bool inarchive();
        archive *getarchive();

private:
        class FileItem: public Item
        {
        public:
                // Attributes
                enum Attribute { File, Directory, Drive };

                unsigned int attr;
        };

        Error err;
        int arcmode;
        zipfile arc;
        static const int PATH_MAX=247;
        char dirprefix[PATH_MAX];
        static unsigned char fc[];

        void listfiles();
        void listdrives();
        void listarc(archive &a);
        char *extract(char *newfn, archive &a, char *oldfn);
        unsigned int drivenum(char *fname);
        void sortinsert(FileItem *newitem);
	bool supported(const char *filename);
	bool is_subdir(struct dirent *direntp);
};
