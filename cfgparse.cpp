/*
 * cfgparse.cpp - Config file parser, by Simon Peter (dn.tlp@gmx.net)
 */

#include <fstream.h>
#include <string.h>
#include <stdio.h>

#include "cfgparse.h"

CfgParse::CfgParse(char *cfgfile)
	: cf(cfgfile, ios::in | ios::nocreate), delim('\n'), varlist(0), err(ERR_NONE)
{
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

unsigned int CfgParse::geterror()
{
	return err;
}
