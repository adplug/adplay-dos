/*
 * cfgparse.cpp - Config file parser
 * Copyright (c) 2001, 2002 Simon Peter <dn.tlp@gmx.net>
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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Define this for debug mode
//#define DEBUG

#ifdef DEBUG
#define DEBUG_FILE      "debug2.log"     // File to log to

static FILE *f_log;

static void dbg_printf(char *fmt, ...)
{
        static char logbuffer[256];

        // build debug log string
        va_list argptr;
        va_start(argptr, fmt);
        vsprintf(logbuffer, fmt, argptr);
        va_end(argptr);

        // print out log line
        fprintf(f_log,logbuffer);
}
#else
static void dbg_printf(char *fmt, ...) { }
#endif

#include "cfgparse.h"

CfgParse::CfgParse(const char *cfgfile)
        : varlist(0), err(None), linenum(0)
{
#ifdef DEBUG
        f_log = fopen(DEBUG_FILE,"at");
        dbg_printf("CfgParse: created, using \"%s\"!\n",cfgfile);
#endif

        if(!(f = fopen(cfgfile,"rt"))) {
                dbg_printf("CfgParse: File not found\n");
                err = NotFound;
        }
}

CfgParse::~CfgParse()
{
	unsigned int i;

	if(varlist) {
		for(i=0;i<items;i++)
			delete [] varlist[i];
		delete [] varlist;
	}

        fclose(f);

#ifdef DEBUG
        dbg_printf("~CfgParse: destroyed!\n\n");
        fclose(f_log);
#endif
}

bool CfgParse::section(const char *name)
{
        dbg_printf("section {\n");

        reparse();

        do {
                do {
                        if(!parse_line()) return false;
                } while(err != NextSection);
        } while(stricmp(name,cursection));

        dbg_printf("}: Section found! (Error::None)\n");
        err = None;
        return true;
}

bool CfgParse::subsection(const char *name, const char *nsec)
{
        char tmpsection[MAXINIITEM];

        dbg_printf("subsection {\n");

        // Rewind current section first
        if(!nsec)
                strcpy(tmpsection,cursection);
        else
                strcpy(tmpsection,nsec);
        section(tmpsection);

        do {
                do {
                        if(!parse_line()) return false;
                        if(err == NextSection) return false;
                } while(err != NextSubsection);
        } while(stricmp(name,cursubsection));

        dbg_printf("}: Subsection found! (Error::None)\n");
        err = None;
        return true;
}

void CfgParse::enum_vars(const char *vars)
{
        const char      *pos;
	unsigned int	i=0;

        // count number of submitted variables for easy malloc()'ing, later
	items=0;
	for(pos=vars;*pos;pos+=strlen(pos)+1)
		items++;

        // Initialize our own variable list with the submitted variables
        varlist = new char*[items];     // get memory
	for(pos=vars;*pos;pos+=strlen(pos)+1) {
		varlist[i] = new char[strlen(pos)+1];
		strcpy(varlist[i],pos);
		i++;
	}

        dbg_printf("enum_vars: %d Variables enumerated\n",items);
}

unsigned int CfgParse::peekvar()
{
        unsigned int i;

        dbg_printf("peekvar {\n");
        err = None;

        parse_line();
        if(err) return items+1;         // exit on error

        dbg_printf("}: ");

        // Check if variable is in our list
        for(i=0;i<items;i++)
                if(!stricmp(var,varlist[i])) {
                        dbg_printf("Variable number: %d\n",i);
                        return i;
                }

        // Variable not in our list!
        dbg_printf("Invalid variable! (Error::Invalid)\n");
        err = Invalid;
        return items+1;
}

long CfgParse::readlong()
{
        long l;

        sscanf(val,"%li",&l);
        return l;
}

unsigned long CfgParse::readulong()
{
        unsigned long ul;

        sscanf(val,"%lu",&ul);
        return ul;
}

char *CfgParse::readstr()
{
        return val;
}

char CfgParse::readchar()
{
        return (*val);
}

bool CfgParse::readbool()
{
        if(!stricmp(val,"yes") || !stricmp(val,"true"))
		return true;
        else
		return false;
}

CfgParse::Error CfgParse::geterror()
{
	return err;
}

bool CfgParse::empty(const char *str)
// Returns true, if 'str' contains only whitespace
{
        unsigned int i;

        for(i=0;i<strlen(str);i++)
                if(str[i] != ' ' && str[i] != '\n' && str[i] != '\r' &&
                        str[i] != '\t')
                        return false;

        return true;
}

bool CfgParse::parse_line()
// Parse the next valid config file line and assign variables
{
        char iniline[MAXINILINE], *dummy;

        err = None;
        dbg_printf("parse_line: ");

        // read in next non-whitespace line
        do {
                if(feof(f)) break;
                fgets(iniline,MAXINILINE,f); linenum++;    // read line
        } while(empty(iniline));

        // Return error on EOF
        if(feof(f)) {
                dbg_printf("End of file (Error::eof)\n");
                err = eof;
                return false;
        }

        // parse line
        dbg_printf("(%d) ",linenum);
        if(sscanf(iniline,"[%s]",cursection)) {
                dummy = strrchr(cursection,']'); *dummy = '\0';
                dbg_printf("[%s] (Error::NextSection)\n",cursection);
                err = NextSection;
                return true;
        }
        if(sscanf(iniline,"(%s)",cursubsection)) {
                dummy = strrchr(cursubsection,')'); *dummy = '\0';
                dbg_printf("(%s) (Error::NextSubsection)\n",cursubsection);
                err = NextSubsection;
                return true;
        }
        if(sscanf(iniline,"%s = %s",var,val)) {
                dbg_printf("<%s> = <%s>\n",var,val);
                return true;
        }

        dbg_printf("Junk detected! (Error::Invalid)\n");
        err = Invalid;
        return false;
}

void CfgParse::reparse()
{
        rewind(f);
        linenum = 0;
}
