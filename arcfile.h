/*
 * arcfile.h - Archive file handling
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
 *
 */

#ifndef H_ARCFILE
#define H_ARCFILE

#include <stdio.h>

/* ArcFile represents a file inside an archive. */
class ArcFile
{
public:
        enum Attributes { None = 0, Directory = 1 };

        char *name;
        unsigned long attr;

        ArcFile();
        ~ArcFile();

        void set_name(char *fname);
};

/* archive represents an archive file (i.e. zipfiles and similar). */
class archive
{
protected:
        FILE *f;
        char *arcname;
        unsigned int files;
        ArcFile *file[256];

public:
        // This function tries to detect the archive type of a given file
        // and returns an initialized archive object of the right type for
        // that file, if successful. 0 is returned otherwise.
        static archive *detect(char *filename);

        // (De-)Constructors
        archive();
        virtual ~archive();

        // Template methods
        virtual bool read() = 0;

        bool open(const char *filename);        // open and read in an archive file
        void close();   // Close an opened archive file and deinit data

        // Returns a file inside the archive
        ArcFile *getfile(unsigned int n)
        { return file[n]; };
        unsigned int getfiles() // Returns number of files inside the archive
        { return files; };
        const char *getarcname()        // Returns the archive's filename
	{ return arcname; };
};

/* zipfile represents a .ZIP file archive. */
class zipfile: public archive
{
public:
        static zipfile *factory(char *filename);
        bool read();
};

#endif
