/*
 * AdPlay - AdPlug DOS Frontend, by Simon Peter (dn.tlp@gmx.net)
 */

#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include <malloc.h>
#include <direct.h>
#include <env.h>

#include "adplug.h"
#include "analopl.h"
#include "timer.h"
#include "txtgfx.h"
#include "window.h"
#include "wndman.h"
#include "cfgparse.h"

// global defines
#define ADPLAYVERS	"AdPlay v1.0"	// AdPlay version string
#define DEFSTACK		(32*1024)		// default stack size for timer-replay in k's
#define CONFIGFILE	"adplay.ini"	// filename of default configuration file
#define DEFCONFIG		"default"		// name of default configuration section
#define FF_MSEC		5000			// milliseconds to fast forward

// global variables
CAnalopl			opl;
CPlayer			*p=0;
CWndMan			wnds;
CTxtWnd			titlebar,infownd,songwnd,instwnd;
CListWnd			filesel;
CBarWnd			volbars(9,63),mastervol(1,63);
unsigned char		backcol=7,colIn=7,colBorder=7,colCaption=7,colSelect=0x70,colUnselect=7;
unsigned int		subsong,optind=1;
bool				hivideo=true,oplforce=false;
volatile unsigned int	time_ms;
char				*optstr = "h?pofcb";
char				configfile[PATH_MAX];

extern void wait_retrace(void);
#pragma aux wait_retrace = \
	"mov dx,03dah" \
"nope: in al,dx" \
	"test al,8" \
	"jz nope" \
"yepp: in al,dx" \
	"test al,8" \
	"jnz yepp" \
	modify [al dx];

void poll_player(void)
{
	static float		oldfreq=0.0f;
	static unsigned int	del=0,wait=0;

	if(!p)
		return;

	if(del) {
		time_ms += 1000/oldfreq/(wait+1);
		del--;
		return;
	} else {
		del = wait;
		time_ms += 1000/oldfreq;
	}

	p->update();
	if(oldfreq != p->getrefresh()) {
		oldfreq = p->getrefresh();
		del = wait = 18.2f / oldfreq;
		tmSetNewRate(1192737/(oldfreq*(wait+1)));
	}
}

void window_center(CWindow &w)
{
	if(hivideo)
		w.setxy((80-w.getsizex())/2,(50-w.getsizey())/2);
	else
		w.setxy((80-w.getsizex())/2,(25-w.getsizey())/2);
}

void listfiles(CListWnd &fl)
/* takes a CListWnd and rebuilds it with a filelist of the current working directory */
{
	DIR *dirp,*direntp;
	char *wd;

	fl.removeall();
	wd = getcwd(NULL,0);
	dirp = opendir(wd);
	while(direntp = readdir(dirp))	// add directories
		if(direntp->d_attr & _A_SUBDIR)
			fl.additem(direntp->d_name);
	rewinddir(dirp);
	while(direntp = readdir(dirp))	// add files
		if(!(direntp->d_attr & _A_SUBDIR))
			fl.additem(direntp->d_name);
	closedir(dirp);
	free(wd);
}

void listcolors(char *fn, CListWnd &cl)
/* takes a CListWnd and rebuilds it with the color-sections list of the given fn */
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

bool isdirectory(char *str)
{
	unsigned int attr;

	_dos_getfileattr(str,&attr);
	if(attr & _A_SUBDIR)
		return true;
	else
		return false;
}

void read_stdwnd(CfgParse &cp, char *subsec, CWindow &w)
{
	unsigned int	var;

	if(!cp.subsection(subsec))
		return;

	while((var = cp.peekvar()) != cp.nitems() + 1)
		switch(var) {
		case 2: w.setxy(cp.readint(),w.posy()); break;
		case 3: w.setxy(w.posx(),cp.readint()); break;
		case 4: w.resize(cp.readint(),w.getsizey()); break;
		case 5: w.resize(w.getsizex(),cp.readint()); break;
		case 6: w.out_setcolor(w.Border,cp.readint()); break;
		case 7: w.out_setcolor(w.Caption,cp.readint()); break;
		case 8: w.out_setcolor(w.In,cp.readint()); break;
		}
}

bool loadcolors(char *fn, char *section)
{
	CfgParse		cp(fn);
	unsigned int	var;
	unsigned int 	val;
	char			str[MAXINILINE];

	if(cp.geterror() == ERR_NOTFOUND)
		return false;

	cp.enum_vars("Background\0AdlibPort\0PosX\0PosY\0SizeX\0SizeY\0ColBorder\0ColCaption\0ColIn\0ColSelect\0"
			 "ColUnselect\0ColBar\0ColClip\0HighRes\0Force\0");

	if(!cp.section(section))
		return false;

	while((var = cp.peekvar()) != cp.nitems() + 1)
		switch(var) {
		case 0: backcol = cp.readint(); break;
		case 1: opl.setport(cp.readint()); break;
		case 6: colBorder = cp.readint(); wnds.setcolor(CWindow::Border,colBorder); break;
		case 7: colCaption = cp.readint(); wnds.setcolor(CWindow::Caption,colCaption); break;
		case 8: colIn = cp.readint(); wnds.setcolor(CWindow::In,colIn); break;
		case 9: colSelect = cp.readint(); filesel.setcolor(filesel.Select,colSelect); break;
		case 10: colUnselect = cp.readint(); filesel.setcolor(filesel.Unselect,colUnselect); break;
		case 11:
			val = cp.readint();
			volbars.setcolor(volbars.Bar,val);
			mastervol.setcolor(mastervol.Bar,val);
			break;
		case 12:
			val = cp.readint();
			volbars.setcolor(volbars.Clip,val);
			mastervol.setcolor(mastervol.Clip,val);
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
		case 14:
			if(cp.readbool())
				oplforce = true;
			else
				oplforce = false;
			break;
		}

	read_stdwnd(cp,"titlebar",titlebar);
	read_stdwnd(cp,"infownd",infownd);
	read_stdwnd(cp,"songwnd",songwnd);
	read_stdwnd(cp,"instwnd",instwnd);

	if(cp.subsection("filesel"))
		while((var = cp.peekvar()) != cp.nitems() + 1)
			switch(var) {
			case 2: filesel.setxy(cp.readint(),filesel.posy()); break;
			case 3: filesel.setxy(filesel.posx(),cp.readint()); break;
			case 4: filesel.resize(cp.readint(),filesel.getsizey()); break;
			case 5: filesel.resize(filesel.getsizex(),cp.readint()); break;
			case 6: filesel.out_setcolor(filesel.Border,cp.readint()); break;
			case 7: filesel.out_setcolor(filesel.Caption,cp.readint()); break;
			case 8: filesel.out_setcolor(filesel.In,cp.readint()); break;
			case 9: filesel.setcolor(filesel.Select,cp.readint()); break;
			case 10: filesel.setcolor(filesel.Unselect,cp.readint()); break;
			}

	if(cp.subsection("volbars"))
		while((var = cp.peekvar()) != cp.nitems() + 1)
			switch(var) {
			case 2: volbars.setxy(cp.readint(),volbars.posy()); break;
			case 3: volbars.setxy(volbars.posx(),cp.readint()); break;
			case 4: volbars.resize(cp.readint(),volbars.getsizey()); break;
			case 5: volbars.resize(volbars.getsizex(),cp.readint()); break;
			case 6: volbars.out_setcolor(volbars.Border,cp.readint()); break;
			case 7: volbars.out_setcolor(volbars.Caption,cp.readint()); break;
			case 8: volbars.out_setcolor(volbars.In,cp.readint()); break;
			case 11: volbars.setcolor(volbars.Bar,cp.readint()); break;
			case 12: volbars.setcolor(volbars.Clip,cp.readint()); break;
			}

	if(cp.subsection("mastervol"))
		while((var = cp.peekvar()) != cp.nitems() + 1)
			switch(var) {
			case 2: mastervol.setxy(cp.readint(),mastervol.posy()); break;
			case 3: mastervol.setxy(mastervol.posx(),cp.readint()); break;
			case 4: mastervol.resize(cp.readint(),mastervol.getsizey()); break;
			case 5: mastervol.resize(mastervol.getsizex(),cp.readint()); break;
			case 6: mastervol.out_setcolor(mastervol.Border,cp.readint()); break;
			case 7: mastervol.out_setcolor(mastervol.Caption,cp.readint()); break;
			case 8: mastervol.out_setcolor(mastervol.In,cp.readint()); break;
			case 11: mastervol.setcolor(mastervol.Bar,cp.readint()); break;
			case 12: mastervol.setcolor(mastervol.Clip,cp.readint()); break;
			}
	return true;
}

void select_colors()
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
	window_center(colwnd);
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
{
	char			tmpstr[80],*title = new char [p->gettitle().length()+1];

	w.erase();
	sprintf(tmpstr,"Subsong : %d / %d",subsong+1,p->getsubsongs()); w.puts(tmpstr);
	sprintf(tmpstr,"Position: %d / %d",p->getorder(),p->getorders()); w.puts(tmpstr);
	sprintf(tmpstr,"Pattern : %d / %d",p->getpattern(),p->getpatterns()); w.puts(tmpstr);
	sprintf(tmpstr,"Row     : %d",p->getrow()); w.puts(tmpstr);
	sprintf(tmpstr,"Speed   : %d",p->getspeed()); w.puts(tmpstr);
	sprintf(tmpstr,"Timer   : %.2f Hz",p->getrefresh()); w.puts(tmpstr);
	sprintf(tmpstr,"Time    : %d:%2d.%2d",time_ms/1000/60,time_ms/1000%60,time_ms/10%100); w.puts(tmpstr);
	w.update();
}

void refresh_songdesc(CTxtWnd &w)
{
	w.erase();
	w.setcaption("Song Message");
	if(p)
		w.puts(p->getdesc());
	w.update();
}

void refresh_volbars(CBarWnd &w, CAnalopl &opl)
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

int main(int argc, char *argv[])
{
	CAdPlug		ap;
	char			inkey=0,*prgdir,*curdir;
	bool			ext;
	unsigned char	volume=0;
	unsigned int	opt;

	cout << ADPLAYVERS << ", (c) 2000 - 2001 Simon Peter (dn.tlp@gmx.net) et al." << endl << endl;

	if(!strcmp(getenv("ADPLAY"),"S")) {
		cout << "AdPlay already running!" << endl;
		return 3;
	} else
		setenv("ADPLAY","S",1);

	// init
	wnds.reg(titlebar); wnds.reg(filesel); wnds.reg(songwnd); wnds.reg(instwnd); wnds.reg(volbars);
	wnds.reg(mastervol); wnds.reg(infownd);
	_splitpath(argv[0],configfile,configfile+2,NULL,NULL); strcat(configfile,CONFIGFILE);
	loadcolors(configfile,DEFCONFIG);	// load default config

	// read commandline
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
		case 5:	// set config file
			strcpy(configfile,argv[optind++]);
			loadcolors(configfile,DEFCONFIG);
			break;
		case 6:	// load config section
			loadcolors(configfile,argv[optind++]);
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
				_heapshrink();
				showcursor();
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

	// interactive mode
	prgdir = getcwd(NULL,0);
	setvideomode(3);
	if(hivideo) load88font();
	hidecursor();
	clearscreen(backcol);
	tmInit(poll_player,0xffff,DEFSTACK);
	titlebar.puts(ADPLAYVERS ", (c) 2000 - 2001 by Simon Peter (dn.tlp@gmx.net) et al.");
	titlebar.puts("Look into the README file for additional author information.");
	titlebar.puts("This is free software under the terms and conditions of the LGPL.");
	songwnd.setcaption("Song Info"); instwnd.setcaption("No Instruments"); volbars.setcaption("VBars");
	titlebar.setcaption(ADPLAYVERS); filesel.setcaption("Directory"); mastervol.setcaption("Vol");
	listfiles(filesel);
	mastervol.set(63);
	wnds.update();
	display_help(infownd);

	// main loop
	do {
		if(p) {	// auto-update windows
			wait_retrace();
			refresh_songinfo(songwnd);
			refresh_volbars(volbars,opl);
		}

		if(kbhit())
			if(!(inkey = toupper(getch()))) {
				ext = true;
				inkey = toupper(getch());

/*				char teststr[10];
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
					time_ms = 0;
				}
				break;
			case 77:	// [Right Arrow] - next subsong
				if(p && subsong < p->getsubsongs()-1) {
					subsong++;
					p->rewind(subsong);
					time_ms = 0;
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
			case 13:	// [Return] - play file / change directory
				if(isdirectory(filesel.getitem(filesel.getselection()))) {
					chdir(filesel.getitem(filesel.getselection()));
					filesel.selectitem(0);
					listfiles(filesel);
					filesel.update();
					break;
				}
				opl.init();
				if(!(p = ap.factory(filesel.getitem(filesel.getselection()),&opl))) {
					CTxtWnd errwnd;
					errwnd.resize(26,3);
					window_center(errwnd);
					errwnd.setcaption("Error!");
					errwnd.out_setcolor(errwnd.Caption,colCaption);
					errwnd.out_setcolor(errwnd.In,colIn);
					errwnd.out_setcolor(errwnd.Border,colBorder);
					errwnd.puts(" Unsupported file type!");
					errwnd.update();
					opl.init();
					while(!getch());
					wnds.update();
					break;
				} else {
					unsigned int	i;
					char			ins[20];

					subsong = 0;
					titlebar.erase();
					titlebar.outtext("File Type: "); titlebar.puts(p->gettype());
					titlebar.outtext("Title    : "); titlebar.puts(p->gettitle());
					titlebar.outtext("Author   : "); titlebar.puts(p->getauthor());
					titlebar.update();
					refresh_songdesc(infownd);
					instwnd.erase();
					for(i=0;i<p->getinstruments();i++) {
                                                sprintf(ins,"%3d³",i+1);
						instwnd.outtext(ins);
						instwnd.puts(p->getinstrument(i));
					}
					sprintf(ins,"%d Instruments",p->getinstruments());
					instwnd.setcaption(ins);
					instwnd.update();
					time_ms = 0;
				}
				break;
			case ' ':	// [Space] - fast forward
				if(p) {
					unsigned int ff;
					opl.setquiet();
					for(ff=0;ff<FF_MSEC;ff+=1000/p->getrefresh())
						p->update();
					time_ms += ff;
					opl.setquiet(false);
				}
				break;
			case 'M':	// refresh song info
				refresh_songdesc(infownd);
				break;
			case 'D':	// shell to DOS
				setvideomode(3);
				showcursor();
				_heapshrink();
				system(getenv("COMSPEC"));
				setvideomode(3);
				if(hivideo) load88font();
				clearscreen(backcol);
				hidecursor();
				listfiles(filesel);
				wnds.update();
				break;
			case '+':
				if(volume) {
					volume--;
					opl.setvolume(volume);
					mastervol.set(63 - volume);
					mastervol.update();
				}
				break;
			case '-':
				if(volume < 63) {
					volume++;
					opl.setvolume(volume);
					mastervol.set(63 - volume);
					mastervol.update();
				}
				break;
			}
	} while(inkey != 27);	// [ESC] - Exit to DOS

	// deinit
	tmClose();
	if(p) delete p;
	opl.init();
	setvideomode(3);
	showcursor();
	chdir(prgdir);
	free(prgdir);
	return 0;
}
