/*
 * cfgparse.h - Config file parser
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
 *
 * NOTES:
 * All functions that can return an error, will only return true if they
 * succeeded and false if there was an error. A subsequent call to geterror()
 * will return the errorcode, defined through the Error enum.
 */

#include <stdio.h>

#define MAXINILINE      256     // max. length of a line in the INI-File
#define MAXINIITEM      16      // max. length of an item in the INI-File

class CfgParse
{
public:
        enum Error { None, NotFound, NextSection, NextSubsection, eof, Invalid };

        CfgParse(const char *cfgfile);  // Open up a config file
        ~CfgParse();

        bool section(const char *name); // Jump to section

        // Jump to subsection, under section 'nsec' or current section if NULL
        bool subsection(const char *name, const char *nsec);

        void reparse();                 // Reparse from scratch

        // Number of with enum_vars() defined variables
        unsigned int nitems() { return items; };

        // Give the parser an idea of what variables you defined in your
        // config file. 'vars' consists of null-terminated substrings for
        // each defined config variable. The whole string is terminated by a
        // double null sequence (i.e. "\0\0").
        void enum_vars(const char *vars);

        // Returns an index into the 'vars' string, pointing to the
        // corresponding variable that comes next in the config file.
        // Use this to get an idea of what of your variables comes next,
        // before reading its value and advance to the next variable.
        //
        // If an invalid variable is parsed, nitems()+1 is returned.
        unsigned int peekvar();

        // Returns the error code of the last called method
        Error geterror();

        // Returns currently parsed line number
        unsigned int getlinenum() { return linenum; };

        // The following methods read variables' values from the config file:
        // They never fail and return no errors at all.
        long readlong();                // read a signed long int
        unsigned long readulong();      // read an unsigned long int
        char *readstr();                // read a string
        char readchar();                // read a char
        bool readbool();                // read a bool ("yes"/"true"=true)

private:
        bool parse_line();
        bool empty(const char *str);

        Error err;
        FILE *f;
        char **varlist;
        unsigned int items,linenum;
        char var[MAXINIITEM],val[MAXINIITEM],cursection[MAXINIITEM],
                cursubsection[MAXINIITEM];
};
