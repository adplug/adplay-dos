/*
 * filewnd.cpp - Small File Manager for DOS w/ archive support
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

#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <sys/types.h>
#include <direct.h>

#include "filewnd.h"
#include "arcfile.h"

FileWnd::FileWnd()
        : CListWnd(), arcmode(0), err(None), items(0)
{
        *dirprefix = '\0';
}

FileWnd::~FileWnd()
{
}

void FileWnd::refresh()
{
        err = None;
        selectitem(0); removeall(); // clear list
        additem(".."); items = 1; attrs[0] = Directory;
        if(arcmode) listarc(arc); else listfiles();
        listdrives();
}

FileWnd::Error FileWnd::geterror()
{
        return err;
}

char *FileWnd::getfilename(char *fn)
{
        err = None;
        if(!arcmode || !(*dirprefix))
                strcpy(fn,getitem(getselection()));
        else {
                strcpy(fn,dirprefix);
                strcat(fn,getitem(getselection()));
        }

        return fn;
}

bool FileWnd::select()
/* Returns false, if this method overtakes all actions.
 * Returns true, if user selected a file to be used.
 */
{
        unsigned int dummy,drive;
        struct diskfree_t dummy2;
        char *fname = getitem(getselection());

        err = None;

        // Disk drive selected?
        if(attrs[getselection()] == Drive && (drive = drivenum(fname)))
                // Try to switch to drive
                if(!_dos_getdiskfree(drive,&dummy2)) {
                        _dos_setdrive(drive,&dummy);
                        arcmode = 0; *dirprefix = '\0'; // Bail out of archive mode
                        return false;
                } else {  // Drive not ready error
                        err = Drive_NotReady;
                        return false;
                }

        // Currently displaying an archive?
        if(arcmode) {
                // ".." selected
                if(!strcmp(fname,"..")) {
                        if(*dirprefix) { // Maybe just up a directory
                                char *tmpstr = strrchr(dirprefix,'/'); *tmpstr = '\0';
                                if(tmpstr = strrchr(dirprefix,'/'))
                                        *(tmpstr+1) = '\0';
                                else
                                        *dirprefix = '\0';
                        } else  // Exit the archive
                                arcmode--;
                        return false;
                }

                // Directory within archive selected?
                if(attrs[getselection()] == Directory) {
                        strcat(dirprefix,fname);
                        strcat(dirprefix,"/");
                        return false;
                }

                // File in archive selected
                return true;
        }

        // If directory selected, switch to it
        if(attrs[getselection()] == Directory) {
                chdir(fname);
                return false;
        }

        // If archive selected, open it
        if(arc.open(fname)) {
                arcmode++;
                return false;
        }

        return true;   // Ordinary file selected
}

bool FileWnd::inarchive()
{
        err = None;
        return arcmode;
}

archive *FileWnd::getarchive()
{
        err = None;
        return &arc;
}

void FileWnd::listfiles()
/* Takes a CListWnd and rebuilds it with a filelist of the current
 * working directory, plus it puts the drive names at last.
 */
{
	DIR *dirp,*direntp;
        char *wd;

        // add files and directories
	wd = getcwd(NULL,0);
	dirp = opendir(wd);
	while(direntp = readdir(dirp))	// add directories
                if(direntp->d_attr & _A_SUBDIR && strcmp(direntp->d_name,".")
                        && strcmp(direntp->d_name,"..")) {
                        additem(direntp->d_name);
                        attrs[items] = Directory; items++;
                }
	rewinddir(dirp);
	while(direntp = readdir(dirp))	// add files
                if(!(direntp->d_attr & _A_SUBDIR)) {
                        additem(direntp->d_name);
                        attrs[items] = File; items++;
                }
	closedir(dirp);
	free(wd);
}

void FileWnd::listdrives()
{
        char drvstr[3] = "A:";
        unsigned int drives,drive,i;

        // add drives
        drive = _getdrive();    // save current drive
        _dos_setdrive(drive,&drives);     // get number of drives
        for(i=1;i<=drives;i++) {
                _dos_setdrive(i,&drives);
                if(_getdrive() == i) {  // drive exists?
                        *drvstr = 'A'+i-1;      // build drive string
                        additem(drvstr);     // add drive name
                        attrs[items] = Drive; items++;
                }
        }
        _dos_setdrive(drive,&drives);   // restore current drive
}

void FileWnd::listarc(archive &a)
{
	unsigned int i;
        ArcFile *f;
        char *basename;

        // Add Subdirectories
        for(i=0;i<a.getfiles();i++) {
                f = a.getfile(i);

                // In our current archive's directory?
                if(strncmp(f->name,dirprefix,strlen(dirprefix))) continue;
                basename = f->name + strlen(dirprefix);

                if(f->attr & ArcFile::Directory && !strchr(basename,'/')) {
                        additem(basename);
                        attrs[items] = Directory; items++;
                }
        }

        // Add files of current directory
        for(i=0;i<a.getfiles();i++) {
                f = a.getfile(i);

                // In our current archive's directory?
                if(strncmp(f->name,dirprefix,strlen(dirprefix))) continue;
                basename = f->name + strlen(dirprefix);
                if(strchr(basename,'/')) continue;

                if(!(f->attr & ArcFile::Directory)) {
                        additem(basename);
                        attrs[items] = File; items++;
                }
        }
}

unsigned int FileWnd::drivenum(char *fname)
{
        char *tmpstr = "A:";

        for(*tmpstr='A';*tmpstr<='Z';(*tmpstr)++)
                if(!strcmp(fname,tmpstr))
                        return ((*tmpstr)-'A'+1);

        return 0;
}
