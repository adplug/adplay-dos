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

#define MAXFCOLORS      6

static unsigned char FileWnd::fc[MAXFCOLORS] = {0x70,7,0x70,7,0x70,7};

FileWnd::FileWnd()
        : CListWnd(), arcmode(0), err(None)
{
        *dirprefix = '\0';
}

FileWnd::~FileWnd()
{
}

void FileWnd::setfilecolor(FColor c, unsigned char v)
{
        fc[c] = v;
}

void FileWnd::refresh()
{
        FileItem *item = new FileItem;

        err = None; removeall();        // clear list
        listdrives();
        if(arcmode) listarc(arc); else listfiles();
        item->settext(".."); item->attr = FileItem::Directory;
        item->setcolor(Item::Selected,fc[DirSel]);
        item->setcolor(Item::Unselected,fc[DirUnsel]);
        additem(item);
}

FileWnd::Error FileWnd::geterror()
{
        return err;
}

char *FileWnd::getfilename(char *fn)
{
        err = None;
        if(!arcmode || !(*dirprefix))
                strcpy(fn,getselection()->gettext());
        else {
                strcpy(fn,dirprefix);
                strcat(fn,getselection()->gettext());
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
        FileItem *f = (FileItem *)getselection();
        char *fname = f->gettext();

        err = None;

        // Disk drive selected?
        if(f->attr == FileItem::Drive && (drive = drivenum(fname)))
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
                if(f->attr == FileItem::Directory) {
                        strcat(dirprefix,fname);
                        strcat(dirprefix,"/");
                        return false;
                }

                // File in archive selected
                return true;
        }

        // If directory selected, switch to it
        if(f->attr == FileItem::Directory) {
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
 * working directory.
 */
{
	DIR *dirp,*direntp;
        char *wd;
        FileItem *f;

        // add files and directories
	wd = getcwd(NULL,0);
	dirp = opendir(wd);
	while(direntp = readdir(dirp))	// add files
                if(!(direntp->d_attr & _A_SUBDIR)) {
                        f = new FileItem;
                        f->settext(direntp->d_name);
                        f->attr = FileItem::File;
                        f->setcolor(Item::Selected,fc[FileSel]);
                        f->setcolor(Item::Unselected,fc[FileUnsel]);
                        sortinsert(f);
                }
	rewinddir(dirp);
	while(direntp = readdir(dirp))	// add directories
                if(direntp->d_attr & _A_SUBDIR && strcmp(direntp->d_name,".")
                        && strcmp(direntp->d_name,"..")) {
                        f = new FileItem;
                        f->settext(direntp->d_name);
                        f->attr = FileItem::Directory;
                        f->setcolor(Item::Selected,fc[DirSel]);
                        f->setcolor(Item::Unselected,fc[DirUnsel]);
                        sortinsert(f);
                }
	closedir(dirp);
	free(wd);
}

void FileWnd::sortinsert(FileItem *newitem)
/* Inserts the given FileItem sorted into the listbox. Assumes that the
 * drive letters are already added and sorts files before drives and
 * directories before files.
 */
{
        unsigned int i = 0;
        FileItem *f;

        while(f = (FileItem *)getitem(i)) {
                // at "end of list" ?
                if(f->attr == FileItem::Drive ||
                       (newitem->attr == FileItem::Directory &&
                       f->attr == FileItem::File)) {
                        if(i) insertitem(newitem,i-1); else additem(newitem);
                        return;
                }

                // sort into list
                if(strcmp(newitem->gettext(),f->gettext()) < 0) {
                        if(i) insertitem(newitem,i-1); else additem(newitem);
                        return;
                }

                i++;
        }
}

void FileWnd::listdrives()
{
        char drvstr[3] = "A:";
        unsigned int drives,drive,i;
        FileItem *f;

        // add drives
        drive = _getdrive();    // save current drive
        _dos_setdrive(drive,&drives);     // get number of drives
        for(i=drives;i>=1;i--) {
                _dos_setdrive(i,&drives);
                if(_getdrive() == i) {  // drive exists?
                        *drvstr = 'A'+i-1;      // build drive string
                        f = new FileItem;
                        f->settext(drvstr);
                        f->attr = FileItem::Drive;
                        f->setcolor(Item::Selected,fc[DriveSel]);
                        f->setcolor(Item::Unselected,fc[DriveUnsel]);
                        additem(f);     // add drive name
                }
        }
        _dos_setdrive(drive,&drives);   // restore current drive
}

void FileWnd::listarc(archive &a)
{
	unsigned int i;
        ArcFile *f;
        char *basename;
        FileItem *item;

        // Add files of current directory
        for(i=0;i<a.getfiles();i++) {
                f = a.getfile(i);

                // In our current archive's directory?
                if(strncmp(f->name,dirprefix,strlen(dirprefix))) continue;
                basename = f->name + strlen(dirprefix);
                if(strchr(basename,'/')) continue;

                if(!(f->attr & ArcFile::Directory)) {
                        item = new FileItem;
                        item->settext(basename);
                        item->attr = FileItem::File;
                        item->setcolor(Item::Selected,fc[FileSel]);
                        item->setcolor(Item::Unselected,fc[FileUnsel]);
                        sortinsert(item);
                }
        }

        // Add Subdirectories
        for(i=0;i<a.getfiles();i++) {
                f = a.getfile(i);

                // In our current archive's directory?
                if(strncmp(f->name,dirprefix,strlen(dirprefix))) continue;
                basename = f->name + strlen(dirprefix);

                if(f->attr & ArcFile::Directory && !strchr(basename,'/')) {
                        item = new FileItem;
                        item->settext(basename);
                        item->attr = FileItem::Directory;
                        item->setcolor(Item::Selected,fc[DirSel]);
                        item->setcolor(Item::Unselected,fc[DirUnsel]);
                        sortinsert(item);
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
