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
 * cfgparse.h - Config file parser, by Simon Peter (dn.tlp@gmx.net)
 */

#include <fstream.h>

#define MAXINILINE	256			// max. length of a line in the INI-File, incl. 0-char

#define ERR_NONE		0
#define ERR_NOTFOUND	1
#define ERR_NEXTSECTION	2

class CfgParse
{
public:
	CfgParse(char *cfgfile);
	~CfgParse();

	void config(char ndelim = '\n');
	bool section(char *name);
	bool subsection(char *name);
	unsigned int nitems()
	{ return items; };

	void enum_vars(char *vars);
	unsigned int peekvar();
	unsigned int geterror();

	int readint();
	unsigned int readuint();
	char *readstr(char *val);
	char readchar();
	bool readbool();

private:
	ifstream		cf;
	char			delim;
	char			**varlist;
	unsigned int	items,err;
	char			cursec[MAXINILINE];
};
