/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999, 2000, 2001 Simon Peter, <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * arcfile.cpp - Archive file handling, by Simon Peter (dn.tlp@gmx.net)
 */

#include <fstream.h>
#include <string.h>

#include "arcfile.h"

zipfile::zipfile(char *filename)
{
	if(filename)
		open(filename);
}

zipfile::~zipfile()
{
	for(unsigned int i=0;i<names;i++)
		delete [] fname[i];
}

void zipfile::open(char *filename)
{
	ifstream f(filename);
	open(f);
}

void zipfile::open(ifstream &f)
{
#pragma pack(1)
	struct {
		char id[4];
		char dummy[14];
		unsigned long cmpsize,fsize;
		unsigned short flen,xlnlen;
	} fhead;
#pragma pack()

	char test[101];

	for(names=0;!f.eof();names++) {
		f.read((char *)&fhead,sizeof(fhead));
		if(strncmp(fhead.id,"PK\x03\x04",4))
			break;
		fname[names] = new char[fhead.flen + 1];
		f.read(fname[names],fhead.flen);
		fname[names][fhead.flen] = '\0';
		f.seekg(fhead.xlnlen+fhead.cmpsize,ios::cur);
	}
}
