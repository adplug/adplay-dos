/*
 * arcfile.cpp - Archive file handling, by Simon Peter (dn.tlp@gmx.net)
 */

#include <fstream.h>
#include <string.h>

#include "arcfile.h"

zipfile::zipfile(char *filename): arcname(0)
{
	if(filename)
		open(filename);
}

zipfile::~zipfile()
{
	for(unsigned int i=0;i<names;i++)
		delete [] fname[i];
	if(arcname)
		delete [] arcname;
}

bool zipfile::open(char *filename)
{
	ifstream f(filename);
	arcname = new char [strlen(filename)+1];
	strcpy(arcname,filename);
	return open(f);
}

bool zipfile::open(ifstream &f)
{
#pragma pack(1)
	struct {
		char id[4];
		char dummy[14];
		unsigned long cmpsize,fsize;
		unsigned short flen,xlnlen;
	} fhead;
#pragma pack()
	unsigned long fpos=0;

	for(names=0;!f.eof();names++) {
		f.read((char *)&fhead,sizeof(fhead));
		if(strncmp(fhead.id,"PK\x03\x04",4) || !fhead.flen)
			break;
		fname[names] = new char[fhead.flen + 1];
		f.read(fname[names],fhead.flen);
		fname[names][fhead.flen] = '\0';
		fpos += sizeof(fhead)+fhead.flen+fhead.xlnlen+fhead.cmpsize;
		f.seekg(fpos);
	}

	if(names)
		return true;
	else
		return false;
}
