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
 * cfgparse.cpp - Config file parser, by Simon Peter (dn.tlp@gmx.net)
 */

#include <fstream.h>
#include <string.h>
#include <stdio.h>

#include "cfgparse.h"

CfgParse::CfgParse(char *cfgfile)
	: cf(cfgfile, ios::in | ios::nocreate), delim('\n'), varlist(0), err(ERR_NONE)
{
	if(!cf.is_open())
		err = ERR_NOTFOUND;
}

CfgParse::~CfgParse()
{
	unsigned int i;

	if(varlist) {
		for(i=0;i<items;i++)
			delete [] varlist[i];
		delete [] varlist;
	}
}

bool CfgParse::section(char *name)
{
	char secline[MAXINILINE],var[MAXINILINE],*dummy;

	strcpy(cursec,name);
	cf.seekg(0);
	do {				// search section
		cf.getline(secline,MAXINILINE,delim);
		sscanf(secline," [%s",var);
		dummy = strrchr(var,']');
		*dummy = '\0';
	} while(strcmp(name,var) && !cf.eof());
	if(strcmp(name,var))
		return false;	// section not found
	else {
		err = ERR_NONE;
		return true;	// section found
	}
}

bool CfgParse::subsection(char *name)
{
	char secline[MAXINILINE],var[MAXINILINE],*dummy;

	section(cursec);
	do {				// search section
		cf.getline(secline,MAXINILINE,delim);
		sscanf(secline," (%s",var);
		dummy = strrchr(var,')');
		*dummy = '\0';
	} while(strcmp(name,var) && !cf.eof() && !strchr(secline,'['));
	if(strchr(var,'[')) {
		err = ERR_NEXTSECTION;
		return false;
	}
	if(strcmp(name,var)) {
		err = ERR_NOTFOUND;
		return false;	// section not found
	} else {
		err = ERR_NONE;
		return true;	// section found
	}
}

void CfgParse::config(char ndelim)
{
	delim = ndelim;
}

void CfgParse::enum_vars(char *vars)
{
	char			*pos;
	unsigned int	i=0;

	items=0;
	for(pos=vars;*pos;pos+=strlen(pos)+1)
		items++;

	varlist = new char*[items];

	for(pos=vars;*pos;pos+=strlen(pos)+1) {
		varlist[i] = new char[strlen(pos)+1];
		strcpy(varlist[i],pos);
		i++;
	}
}

unsigned int CfgParse::peekvar()
{
	char			var[MAXINILINE];
	unsigned int	i;

	cf >> var; cf.ignore(MAXINILINE,'=');

	if(strchr(var,'[')) {
		err = ERR_NEXTSECTION;
		return items+1;
	}

	for(i=0;i<items;i++)
		if(!strcmp(var,varlist[i]))
			return i;

	err = ERR_NOTFOUND;
	return items+1;
}

int CfgParse::readint()
{
	int val;

	cf >> val;
	return val;
}

unsigned int CfgParse::readuint()
{
	unsigned int val;

	cf >> val;
	return val;
}

char *CfgParse::readstr(char *val)
{
	cf >> val;
	return val;
}

char CfgParse::readchar()
{
	char val;

	cf >> val;
	return val;
}

bool CfgParse::readbool()
{
	char val[MAXINILINE];

	cf >> val;
	if(!strcmp(val,"Yes") || !strcmp(val,"True"))
		return true;
	else
		return false;
}

unsigned int CfgParse::geterror()
{
	return err;
}
