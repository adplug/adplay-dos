/*
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

private:
	ifstream		cf;
	char			delim;
	char			**varlist;
	unsigned int	items,err;
	char			cursec[MAXINILINE];
};
