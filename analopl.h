/*
 * analopl.h - Spectrum analyzing hardware OPL, by Simon Peter (dn.tlp@gmx.net)
 */

#ifndef H_ANALOPL_DEFINED
#define H_ANALOPL_DEFINED

#include "opl.h"

#define DFL_ADLIBPORT	0x388		// default adlib baseport

class CAnalopl: public Copl
{
public:
	CAnalopl(unsigned short initport = DFL_ADLIBPORT);	// initport = OPL2 hardware baseport

	bool detect();						// returns true if adlib compatible board is found, else false
	void setvolume(int volume);			// set adlib master volume (0 - 63) 0 = loudest, 63 = softest
	void setquiet(bool quiet = true);	// sets the OPL2 quiet, while still writing to the registers
	void setport(unsigned short port)	// set new OPL2 hardware baseport
	{ adlport = port; };
	void setnowrite(bool nw = true)		// set hardware write status
	{ nowrite = nw; };

	int getvolume()						// get adlib master volume
	{ return hardvol; };
	int getcarriervol(unsigned int v);			// get carrier volume of adlib voice v
	int getmodulatorvol(unsigned int v);		// get modulator volume of adlib voice v
	bool getkeyon(unsigned int v);

	// template methods
	void write(int reg, int val);
	void init();

private:
	void hardwrite(int reg, int val);	// write to OPL2 hardware registers

	unsigned short	adlport;			// adlib hardware baseport
	int				hardvol,oldvol;		// hardware master volume
	bool			bequiet;			// quiet status cache
	char			hardvols[22][2];	// volume cache
	unsigned char	keyregs[9][2];			// shadow key register
	bool			nowrite;			// don't write to hardware, if true
};

#endif
