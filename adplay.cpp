/*
 * AdPlay/DOS - AdPlug DOS Frontend
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

#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <malloc.h>
#include <direct.h>
#include <env.h>
#include <adplug/adplug.h>
#include <adplug/analopl.h>
#include <timer.h>
#include <window/txtgfx.h>
#include <window/window.h>
#include <window/wndman.h>

#include "cfgparse.h"
#include "arcfile.h"
#include "filewnd.h"

// global defines
#define ADPLAYVERS      "AdPlay 1.2"    // AdPlay version string
#define DEFSTACK        (32*1024)       // stack size for timer-replay code
#define CONFIGFILE      "adplay.ini"    // filename of default configuration file
#define DEFCONFIG       "default"       // name of default configuration section
#define FF_MSEC         5000            // milliseconds to fast forward
#define NORMALIZE       70.0f           // Timer frequency to normalize to

// Valid configuration variables
#define CONFIG_VARS     "Background\0AdlibPort\0PosX\0PosY\0SizeX\0SizeY\0" \
        "ColBorder\0ColCaption\0ColIn\0ColSelect\0ColUnselect\0ColBar\0" \
        "ColClip\0HighRes\0Force\0"

// global variables
CAdPlug ap;                               // Main AdPlug object
CAnalopl opl;                             // The only output device
CPlayer *p=0;                             // Main player (0 = none loaded)
CWndMan wnds;                             // Window manager
CTxtWnd titlebar,infownd,songwnd,instwnd; // Windows
FileWnd filesel;                          // File selector window
CBarWnd volbars(9,63),mastervol(1,63);    // More windows
unsigned char backcol=7,colIn=7,colBorder=7,colCaption=7,colSelect=0x70,
        colUnselect=7;                    // Main colors (set to defaults)
unsigned int subsong,optind=1;
bool hivideo=true,oplforce=false;         // Configuration
volatile float time_ms=0.0f;              // Current playing time in ms
const char *optstr = "h?pofcb";           // Commandline options
char configfile[PATH_MAX];                // Path to configfile
VideoInfo dosvideo;                       // Stores previous (DOS's) video settings
int volume=0;                             // Main volume
float last_ms=0.0f;                       // helper variable for delay_ms()

void poll_player(void)
/* This is the main replay function and hooked at first into the timer
 * interrupt. It calls the appropriate player code and does some sanity
 * checks before. It provides a pseudo timer environment for the player for
 * timer rates that cannot be set in DOS. If the player requests a new timer
 * rate, and it can be set, this function sets it appropriately.
 *
 * The pseudo timer works as follows: If a timer rate slower than NORMALIZE
 * is requested, we determine the next smallest multiple of the requested
 * rate, that is bigger than NORMALIZE and set the timer using this value
 * instead. The next calls to our function will then wait for as many times
 * as it took the original value to get above NORMALIZE and poll the player
 * only at the last waiting cycle.
 */
{
        static float            oldfreq=18.2f;
	static unsigned int	del=0,wait=0;

        if(!p) return;  // no player in memory means we're not playing

        time_ms += 1000/(oldfreq*(wait+1));

        // pseudo timer environment
	if(del) {
                del--;          // just wait for this time fraction
		return;
        } else
                del = wait;     // last fraction, reset for next one

        p->update();    // call player

        if(oldfreq != p->getrefresh()) {        // new timer rate requested?
		oldfreq = p->getrefresh();
                del = wait = NORMALIZE / oldfreq;
		tmSetNewRate(1192737/(oldfreq*(wait+1)));
	}
}

void setadplugvideo()
/* Setup AdPlay's idea of the video state */
{
	setvideomode(3);
	if(hivideo) load88font();
	clearscreen(backcol);
	hidecursor();
}

bool dosshell(char *cmd)
/* Runs the given command in a DOS shell. Returns true if succeeded. Saves
 * and restores the state of all required variables during the shell command.
 */
{
        int retval;

        setvideoinfo(&dosvideo);
	_heapshrink();
        retval = system(cmd);
        setadplugvideo();

        if(retval) return false; else return true;
}

void listcolors(char *fn, CListWnd &cl)
/* Takes a CListWnd and rebuilds it with the color-sections list of
 * the given filename 'fn'.
 */
{
	ifstream	f(fn,ios::in | ios::nocreate);
	char		vglsec[MAXINILINE],var[MAXINILINE],*dummy;

	if(!f.is_open())			// file not found?
		return;

	cl.removeall();
	do {					// list sections
		f.getline(vglsec,MAXINILINE);
		if(sscanf(vglsec," [%s",var) && *vglsec) {
			dummy = strrchr(var,']');
			*dummy = '\0';
			cl.additem(var);
		}
	} while(!f.eof());
}

void display_help(CTxtWnd &w)
/* Displays AdPlay's main help text into the given CTxtWnd. */
{
	w.erase();
	w.setcaption("Help");
	w.puts("Keyboard Control:\n"
		 "-----------------\n\n"
		 "Up/Down    - Select in Menu\n"
		 "Left/Right - Previous/Next Subsong\n"
		 "PgUp/PgDn  - Scroll Instrument Names Window\n"
		 "Home/End   - Scroll Help / Song Message Window\n"
		 "Return     - Play Song / Change Directory\n"
		 "Space      - Fast Forward\n"
		 "ESC        - Exit to DOS\n"
		 "F1         - Help\n"
		 "F2         - Change Screen Layout\n"
		 "D          - Shell to DOS\n"
		 "M          - Display Song Message\n"
		 "+/-        - Volume Up/Down");
	w.update();
}

void read_stdwnd(CfgParse &cp, char *subsec, char *section, CWindow &w)
/* Sets the configured positions and colors for a given window */
{
        if(!cp.subsection(subsec,section)) {
                printf("subsection '%s' not found (%d)!\n",subsec,cp.geterror());
		return;
        }

        while(!cp.geterror())
                switch(cp.peekvar()) {
                case 2: w.setxy(cp.readlong(),w.posy()); break;
                case 3: w.setxy(w.posx(),cp.readlong()); break;
                case 4: w.resize(cp.readlong(),w.getsizey()); break;
                case 5: w.resize(w.getsizex(),cp.readlong()); break;
                case 6: w.out_setcolor(w.Border,cp.readlong()); break;
                case 7: w.out_setcolor(w.Caption,cp.readlong()); break;
                case 8: w.out_setcolor(w.In,cp.readlong()); break;
		}
}

bool loadconfig(char *fn, char *section)
/* Loads and sets the general configuration out of the file, named by 'fn',
 * using the section 'section' therein. Returns true, if succeeded.
 */
{
        CfgParse cp(fn);

        if(cp.geterror() == CfgParse::NotFound)       // file not found?
		return false;

        // Define config variables
        cp.enum_vars(CONFIG_VARS);

        if(!cp.section(section))        // Jump to section, if defined
		return false;

        do {
                switch(cp.peekvar()) {
                case 1: opl.setport(cp.readlong()); break;
                case 14: oplforce = cp.readbool(); break;
		}
        } while(!cp.geterror());

	return true;
}

bool loadcolors(char *fn, char *section)
/* Loads and sets the color-definition out of the file, named by 'fn',
 * using the section 'section' therein. Returns true, if succeeded.
 */
{
        CfgParse cp(fn);

        if(cp.geterror() == CfgParse::NotFound)       // file not found?
		return false;

        // Defined config variables
        cp.enum_vars(CONFIG_VARS);

        if(!cp.section(section))        // Jump to section, if defined
		return false;

        do {
                switch(cp.peekvar()) {
                case 0: backcol = cp.readlong(); break;
                case 6: colBorder = cp.readlong(); wnds.setcolor(CWindow::Border,colBorder); break;
                case 7: colCaption = cp.readlong(); wnds.setcolor(CWindow::Caption,colCaption); break;
                case 8: colIn = cp.readlong(); wnds.setcolor(CWindow::In,colIn); break;
                case 9: colSelect = cp.readlong(); filesel.setcolor(filesel.Select,colSelect); break;
                case 10: colUnselect = cp.readlong(); filesel.setcolor(filesel.Unselect,colUnselect); break;
		case 11:
                        volbars.setcolor(volbars.Bar,cp.readlong());
                        mastervol.setcolor(mastervol.Bar,cp.readlong());
			break;
		case 12:
                        volbars.setcolor(volbars.Clip,cp.readlong());
                        mastervol.setcolor(mastervol.Clip,cp.readlong());
			break;
		case 13:
			if(cp.readbool()) {
				hivideo = true;
				load88font();
				hidecursor();
			} else {
				hivideo = false;
				setvideomode(3);
				hidecursor();
			}
			break;
		}
        } while(!cp.geterror());

        read_stdwnd(cp,"titlebar",section,titlebar);
        read_stdwnd(cp,"infownd",section,infownd);
        read_stdwnd(cp,"songwnd",section,songwnd);
        read_stdwnd(cp,"instwnd",section,instwnd);
        read_stdwnd(cp,"filesel",section,filesel);
        read_stdwnd(cp,"volbars",section,volbars);
        read_stdwnd(cp,"mastervol",section,mastervol);

        if(cp.subsection("filesel",section))
                while(!cp.geterror())
                        switch(cp.peekvar()) {
                        case 9: filesel.setcolor(filesel.Select,cp.readlong()); break;
                        case 10: filesel.setcolor(filesel.Unselect,cp.readlong()); break;
			}

        if(cp.subsection("volbars",section))
                while(!cp.geterror())
                        switch(cp.peekvar()) {
                        case 11: volbars.setcolor(volbars.Bar,cp.readlong()); break;
                        case 12: volbars.setcolor(volbars.Clip,cp.readlong()); break;
			}

        if(cp.subsection("mastervol",section))
                while(!cp.geterror())
                        switch(cp.peekvar()) {
                        case 11: mastervol.setcolor(mastervol.Bar,cp.readlong()); break;
                        case 12: mastervol.setcolor(mastervol.Clip,cp.readlong()); break;
			}
	return true;
}

void select_colors()
/* Displays the "Change Screen Layout" window and lets the user select
 * a new layout and sets this layout afterwards.
 */
{
	CListWnd	colwnd;
	char		inkey=0;
	bool		ext;

	colwnd.resize(20,10);
	colwnd.setcaption("Layouts");
	colwnd.out_setcolor(colwnd.Border,colBorder);
	colwnd.out_setcolor(colwnd.In,colIn);
	colwnd.out_setcolor(colwnd.Caption,colCaption);
	colwnd.setcolor(colwnd.Select,colSelect);
	colwnd.setcolor(colwnd.Unselect,colUnselect);
        colwnd.center();
	listcolors(configfile,colwnd);
	colwnd.update();

	do {
		if(kbhit())
			if(!(inkey = toupper(getch()))) {
				ext = true;
				inkey = toupper(getch());
			} else
				ext = false;
		else
			inkey = 0;

		if(ext)	// handle all extended keys
			switch(inkey) {
			case 72:	// [Up Arrow] - menu up
				colwnd.select_prev();
				colwnd.update();
				break;
			case 80:	// [Down Arrow] - menu down
				colwnd.select_next();
				colwnd.update();
				break;
			}
		else		// handle all normal keys
			switch(inkey) {
			case 13:	// [Return] - select layout
				loadcolors(configfile,colwnd.getitem(colwnd.getselection()));
				break;
			}
	} while(inkey != 27 && inkey != 13);	// [ESC] - Exit menu
}

void refresh_songinfo(CTxtWnd &w)
/* Refreshes the "Song Info" window with data from the loaded player. */
{
        char tmpstr[80];
        unsigned int time = (unsigned int)time_ms;

	w.erase();
	sprintf(tmpstr,"Subsong : %d / %d",subsong+1,p->getsubsongs()); w.puts(tmpstr);
	sprintf(tmpstr,"Position: %d / %d",p->getorder(),p->getorders()); w.puts(tmpstr);
	sprintf(tmpstr,"Pattern : %d / %d",p->getpattern(),p->getpatterns()); w.puts(tmpstr);
	sprintf(tmpstr,"Row     : %d",p->getrow()); w.puts(tmpstr);
	sprintf(tmpstr,"Speed   : %d",p->getspeed()); w.puts(tmpstr);
	sprintf(tmpstr,"Timer   : %.2f Hz",p->getrefresh()); w.puts(tmpstr);
        sprintf(tmpstr,"Time    : %d:%2d.%2d",time/1000/60,time/1000%60,time/10%100); w.puts(tmpstr);
	w.update();
}

void refresh_songdesc(CTxtWnd &w)
/* Refreshes the "Song Message" window with data from the loaded player. */
{
	w.erase();
	w.setcaption("Song Message");
	if(p)
		w.puts(p->getdesc());
	w.update();
}

void refresh_volbars(CBarWnd &w, CAnalopl &opl)
/* Refreshes the "Volume Bars" window, using the analyzing hardware OPL
 * output driver.
 */
{
	unsigned int i;

	for(i=0;i<9;i++) {
		if(opl.getkeyon(i))
			w.set(63 - opl.getcarriervol(i),i);
		else
			if(w.get(i))
				w.set(w.get(i)-1,i);
	}
	w.update();
}

unsigned int getopt(int n, char **s)
/* Returns an index into the optstr, on which option has been set on the
 * commandline 's' (argv), with 'n' maximum options (argc).
 */
{
	unsigned int	i;

	if(optind >= n)
		return 0;

	for(i=0;i<strlen(optstr);i++) {
		if(s[optind][0] != '-' && s[optind][0] != '/') return 0;
		if(optstr[i] == s[optind][1]) break;
	}
	optind++;
	return (i+1);
}

void fast_forward(const unsigned int ms)
/* Fast forward the specified amount of milliseconds */
{
        float ff;

        if(!p) return;  // Are we actually playing?

        opl.setquiet();

        for(ff=0;ff<ms;ff+=1000/p->getrefresh())
                p->update();

        time_ms += ff;
        opl.setquiet(false);
}

char *extract(char *newfn, archive *a, char *oldfn)
/* Extract file 'oldfn' from archive 'a'. The filename of the extracted file
 * is stored in 'newfn'.
 */
{
	char cmd[256];

	strcpy(cmd,"pkunzip ");
        strcat(cmd,a->getarcname());
        strcat(cmd," \"");
	strcat(cmd,oldfn);
        strcat(cmd,"\" ");
	strcat(cmd,getenv("TMP"));
        if(!dosshell(cmd)) {    // If an error occured...
                puts("An error occured while running the command:");
                printf("%s\n",cmd);
                puts("Please consult the above error messages, if any, and "
                        "press any key to continue...");
                getch();
        }
	wnds.update();
	strcpy(newfn,getenv("TMP"));
	strcat(newfn,"\\");
        strcat(newfn,strrchr(oldfn,'/') ? strrchr(oldfn,'/') + 1 : oldfn);
	return newfn;
}

void play(char *fn)
/* Start playback with file 'fn' */
{
        opl.init();
        p = ap.factory(fn,&opl);     // get corresponding player

        if(!p) {
                // File not supported error
                CTxtWnd errwnd;

                errwnd.resize(26,3);
                errwnd.center();
                errwnd.setcaption("Error!");
                errwnd.out_setcolor(errwnd.Caption,colCaption);
                errwnd.out_setcolor(errwnd.In,colIn);
                errwnd.out_setcolor(errwnd.Border,colBorder);
                errwnd.puts(" Unsupported file type!");
                errwnd.update();
                opl.init();
                while(!getch());
                wnds.update();
                return;
        }

        // Reset playing infos
        last_ms = time_ms = 0.0f; subsong = 0;

        // Update view with file information
        unsigned int i;
        char ins[20];

        // Update titlebar
        titlebar.erase();
        titlebar.outtext("File Type: "); titlebar.puts(p->gettype());
        titlebar.outtext("Title    : "); titlebar.puts(p->gettitle());
        titlebar.outtext("Author   : "); titlebar.puts(p->getauthor());
        titlebar.update();

        refresh_songdesc(infownd);      // Update "Song Info" window

        // Update instruments window
        instwnd.erase();
        for(i=0;i<p->getinstruments();i++) {
                sprintf(ins,"%3d³",i+1);
                instwnd.outtext(ins);
                instwnd.puts(p->getinstrument(i));
        }
        sprintf(ins,"%d Instruments",p->getinstruments());
        instwnd.setcaption(ins);
        instwnd.update();
}

void adjust_volume(int amount)
/* Adjust the main volume by 'amount' units (bounds checking) */
{
        int newvol = (volume + amount);

        if(newvol >= 0 && newvol < 63) {
                volume = newvol;
                opl.setvolume(volume);
                mastervol.set(63 - volume);
                mastervol.update();
        }
}

void activate()
{
        CTxtWnd errwnd;
        bool result;
        char tmpfn[PATH_MAX];

        // Preinitialize an error window, maybe we need it later...
        errwnd.resize(26,3);
        errwnd.center();
        errwnd.setcaption("Error!");
        errwnd.out_setcolor(errwnd.Caption,colCaption);
        errwnd.out_setcolor(errwnd.In,colIn);
        errwnd.out_setcolor(errwnd.Border,colBorder);

        result = filesel.select();

        // Drive not ready error?
        if(filesel.geterror() == FileWnd::Drive_NotReady) {
                errwnd.puts("    Drive not ready!");
                errwnd.update();
                while(!getch());
                wnds.update();
                return;
        }

        if(result) {
                // Start playback
                if(filesel.inarchive()) {
                        char extract_name[PATH_MAX];
                        extract(extract_name,filesel.getarchive(),
                                filesel.getfilename(tmpfn));
                        play(extract_name);
                        remove(extract_name);
                } else
                        play(filesel.getfilename(tmpfn));
        } else {
                // Refresh file manager window
                filesel.refresh();
                filesel.update();
        }
}

void idle_ms(unsigned int ms)
{
        while(time_ms < last_ms+ms) delay(0);
        last_ms = time_ms;
}

int main(int argc, char *argv[])
{
        char            inkey=0,*prgdir,*curdir;
        bool            ext;
	unsigned int	opt;

        cout << ADPLAYVERS << ", Copyright (c) 2000 - 2002 Simon Peter <dn.tlp@gmx.net>" << endl << endl;

        // check that no other instance is running
	if(!strcmp(getenv("ADPLAY"),"S")) {
		cout << "AdPlay already running!" << endl;
		return 3;
	} else
                setenv("ADPLAY","S",1); // flag our instance

        // Build path to default configuration file (in program's directory)
	_splitpath(argv[0],configfile,configfile+2,NULL,NULL);
        strcat(configfile,CONFIGFILE);

        loadconfig(configfile,DEFCONFIG);       // load default configuration

        // parse commandline for general options
	while(opt = getopt(argc,argv))
		switch(opt) {
		case 1:	// display help
		case 2:
			cout << "Options can be set with '-' or '/' respectively." << endl << endl;
			cout << " -?, -h      Display commandline help" << endl <<
				  " -p port     Set OPL2 port" << endl <<
				  " -o          Force OPL2 port" << endl <<
				  " -f file     Use alternate configuration file" << endl <<
				  " -c section  Load another configuration section" << endl <<
				  " -b file     Immediate background playback using specified file" << endl;
			showcursor();
			return 0;
		case 3:	// set OPL2 port
			opl.setport(atoi(argv[optind++]));
			break;
		case 4:	// force OPL2 flag
			oplforce = true;
			break;
		case 7:	// background playback
			if(!opl.detect() && !oplforce) {
				cout << "No OPL2 detected!" << endl;
				showcursor();
				return 2;
			}

			if(!(p = ap.factory(argv[optind],&opl))) {
				cout << "[" << argv[optind] << "]: unsupported file type!" << endl;
				return 1;
			} else {
				cout << "Background playback... (type EXIT to stop)" << endl;
				tmInit(poll_player,0xffff,DEFSTACK);
                                setvideoinfo(&dosvideo);
                                _heapshrink();
                                system(getenv("COMSPEC"));
				tmClose();
				delete p;
				opl.init();
				return 0;
			}
			break;
		}

	if(!opl.detect() && !oplforce) {
		cout << "No OPL2 detected!" << endl;
		showcursor();
		return 2;
	}

        /*** interactive (GUI) mode ***/
        getvideoinfo(&dosvideo);        // Save previous video state

        // register our windows with the window manager
        wnds.reg(titlebar); wnds.reg(filesel); wnds.reg(songwnd);
        wnds.reg(instwnd); wnds.reg(volbars); wnds.reg(mastervol);
        wnds.reg(infownd);

        loadcolors(configfile,DEFCONFIG);       // load default GUI layout

        // reparse commandline for GUI options
        optind = 1;     // reset option parser
	while(opt = getopt(argc,argv))
		switch(opt) {
		case 5:	// set config file
			strcpy(configfile,argv[optind++]);
			loadcolors(configfile,DEFCONFIG);
			break;
		case 6:	// load config section
			loadcolors(configfile,argv[optind++]);
			break;
		}

        // init GUI
	prgdir = getcwd(NULL,0);
        setadplugvideo();
	tmInit(poll_player,0xffff,DEFSTACK);
        titlebar.puts(ADPLAYVERS ", Copyright (c) 2000 - 2002 Simon Peter <dn.tlp@gmx.net>");
	titlebar.puts("This is free software under the terms and conditions of the Nullsoft license.");
	titlebar.puts("Refer to the file README.TXT for more information.");
	songwnd.setcaption("Song Info"); instwnd.setcaption("No Instruments"); volbars.setcaption("VBars");
	titlebar.setcaption(ADPLAYVERS); filesel.setcaption("Directory"); mastervol.setcaption("Vol");
        filesel.refresh();
	mastervol.set(63);
	wnds.update();
	display_help(infownd);

	// main loop
	do {
		if(p) {	// auto-update windows
//                        wait_retrace();
                        idle_ms(1000/70);
                        refresh_songinfo(songwnd);
                        refresh_volbars(volbars,opl);
		}

		if(kbhit())
			if(!(inkey = toupper(getch()))) {
				ext = true;
				inkey = toupper(getch());

/* Enable this if you want the currently pressed key displayed on screen */
/*                                char teststr[10];
                                sprintf(teststr,"%d",inkey);
				titlebar.erase();
				titlebar.outtext(teststr);
                                titlebar.update(); */
			} else
				ext = false;
		else
			inkey = 0;

		if(ext)	// handle all extended keys
			switch(inkey) {
			case 59:	// [F1] - display help
				display_help(infownd);
				break;
			case 60:	// [F2] - change screen layout
				curdir = getcwd(NULL,0);
				chdir(prgdir);
				select_colors();
				chdir(curdir);
				free(curdir);
				clearscreen(backcol);
				wnds.update();
				break;
			case 72:	// [Up Arrow] - menu up
				filesel.select_prev();
				filesel.update();
				break;
			case 80:	// [Down Arrow] - menu down
				filesel.select_next();
				filesel.update();
				break;
			case 75:	// [Left Arrow] - previous subsong
				if(p && subsong) {
					subsong--;
					p->rewind(subsong);
                                        last_ms = time_ms = 0.0f;
				}
				break;
			case 77:	// [Right Arrow] - next subsong
				if(p && subsong < p->getsubsongs()-1) {
					subsong++;
					p->rewind(subsong);
                                        last_ms = time_ms = 0.0f;
				}
				break;
			case 73:	// [Page Up] - scroll up file info box
				instwnd.scroll_up();
				instwnd.update();
				break;
			case 81:	// [Page Down] - scroll down file info box
				instwnd.scroll_down();
				instwnd.update();
				break;
			case 71:	// [Home] - scroll up info window
				infownd.scroll_up();
				infownd.update();
				break;
			case 79:	// [End] - scroll down info window
				infownd.scroll_down();
				infownd.update();
				break;
			}
		else		// handle all normal keys
			switch(inkey) {
                        case 13:        // [Return] - Activate
                                activate();
				break;
			case ' ':	// [Space] - fast forward
                                fast_forward(FF_MSEC);
				break;
			case 'M':	// refresh song info
				refresh_songdesc(infownd);
				break;
			case 'D':	// shell to DOS
                                dosshell(getenv("COMSPEC"));
                                filesel.refresh(); wnds.update();
				break;
                        case '+':       // [+] - Increase volume
                                adjust_volume(-1);
				break;
                        case '-':       // [-] - Decrease volume
                                adjust_volume(+1);
				break;
			}
	} while(inkey != 27);	// [ESC] - Exit to DOS

	// deinit
	tmClose();
	if(p) delete p;
	opl.init();
        setvideoinfo(&dosvideo);
	chdir(prgdir);
	free(prgdir);
	return 0;
}
