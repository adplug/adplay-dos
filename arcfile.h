/*
 * arcfile.h - Archive file handling, by Simon Peter (dn.tlp@gmx.net)
 */

#include <fstream.h>

class zipfile
{
public:
	zipfile(char *filename = 0);
	~zipfile();

	bool open(char *filename);
	bool open(ifstream &f);

	char *getfname(unsigned int n)
	{ return fname[n]; };
	unsigned int getnames()
	{ return names; };
	char *getarcname()
	{ return arcname; };

private:
	char *arcname;
	char *fname[256];
	unsigned int names;
};
