/*
 * arcfile.cpp - Archive file handling
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

#include <stdio.h>
#include <string.h>

#include "arcfile.h"

/***** ArcFile *****/

ArcFile::ArcFile(): name(0), attr(None)
{
}

ArcFile::~ArcFile()
{
        if(name) delete [] name;
}

void ArcFile::set_name(char *fname)
{
        if(name) delete [] name;
        name = new char[strlen(fname)+1];
        strcpy(name,fname);
}

/***** archive *****/

archive::archive(): arcname(0), f(0), files(0)
{
}

archive::~archive()
{
        close();
}

bool archive::open(const char *filename)
{
        close();
        f = fopen(filename,"rb");
	arcname = new char [strlen(filename)+1];
	strcpy(arcname,filename);
        if(read())
                return true;
        else {
                close();
                return false;
        }
}

void archive::close()
{
        for(unsigned int i=0;i<files;i++) delete file[i];
        if(arcname) delete [] arcname;
        if(f) fclose(f);
}

archive *archive::detect(char *filename)
{
        return zipfile::factory(filename);
}

/***** zipfile *****/

static zipfile *zipfile::factory(char *filename)
{
        zipfile *a = new zipfile;

        if(a->open(filename))
                return a;
        else {
                delete a;
                return 0;
        }
}

bool zipfile::read()
{
#pragma pack(1)
	struct {
                unsigned long id;
		char dummy[14];
		unsigned long cmpsize,fsize;
                unsigned short fnlen,xlen;
	} fhead;
#pragma pack()

        // Read local file headers
        for(files=0;!feof(f);files++) {
                fread(&fhead,sizeof(fhead),1,f);  // read local file header

                // End of local file headers (central directory begins)
                if(fhead.id == 0x02014b50) return true;
                // Not a local file header
                if(fhead.id != 0x04034b50) return false;

                // Allocate new file object
                file[files] = new ArcFile;

                // Read file name
                file[files]->name = new char[fhead.fnlen + 1];
                fread(file[files]->name,fhead.fnlen,1,f);
                file[files]->name[fhead.fnlen] = '\0';

                // Set file attributes
                // Directory entry
                if(file[files]->name[fhead.fnlen-1] == '/') {
                        file[files]->attr |= ArcFile::Directory;
                        file[files]->name[fhead.fnlen-1] = '\0';
                }

                // Seek to next local file header
                fseek(f,fhead.xlen+fhead.cmpsize,SEEK_CUR);
	}

        return false;   // Prematurely reached end of file
}
