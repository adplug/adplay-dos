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

// global defines
#define ADPLAYVERS	"AdPlay v1.0"	// AdPlay version string
#define DEFSTACK		(32*1024)		// default stack size for timer-replay in k's
#define DEFPORT		0x388			// default AdLib port
#define MAXINILINE	256			// max. length of a line in the INI-File, incl. 0-char
#define MAXVAR		20			// max. length of a variable name
#define COLORFILE		"colors.ini"	// filename of colors definition file

// global variables
CPlayer			*p=0;
bool				playing=false;
CTxtWnd			titlebar,infownd,songwnd,instwnd;
CListWnd			filesel;
CBarWnd			volbars(9,63),mastervol(1,63);
CWndMan			wnds;
unsigned char		backcol=7;

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

	if(!playing)
		return;

	if(del) {
		del--;
		return;
	} else
		del = wait;

	p->update();
	if(oldfreq != p->getrefresh()) {
		oldfreq = p->getrefresh();
		del = wait = 18.2f / oldfreq;
		tmSetNewRate(1192737/(oldfreq*(wait+1)));
	}
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
	char		vglsec[MAXINILINE],var[MAXVAR],*dummy;

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
		 "Up/Down   - Select in Menu\n"
		 "PgUp/PgDn - Scroll Instrument Names Window\n"
		 "ESC       - Exit to DOS\n"
		 "F1        - Help\n"
		 "D         - Shell to DOS\n"
		 "C         - Change Colormap\n"
		 "M         - Display Song Message\n"
		 "+/-       - Volume Up/Down");
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

bool loadcolors(char *fn, char *section)
{
	ifstream		f(fn,ios::in | ios::nocreate);
	char			vglsec[MAXINILINE],var[MAXVAR],*dummy;
	unsigned int	val;

	if(!f.is_open())			// file not found?
		return false;

	do {					// search section
		f.getline(vglsec,MAXINILINE);
		sscanf(vglsec," [%s",var);
		dummy = strrchr(var,']');
		*dummy = '\0';
	} while(strcmp(section,var) && !f.eof());
	if(strcmp(section,var))	// section not found
		return false;

	do {					// parse section entries
		f >> var; f.ignore(MAXINILINE,'='); f >> val;
		if(!strcmp(var,"Background")) backcol = val;
		if(!strcmp(var,"titleBorder")) titlebar.out_setcolor(titlebar.Border,val);
		if(!strcmp(var,"titleIn")) titlebar.out_setcolor(titlebar.In,val);
		if(!strcmp(var,"titleCaption")) titlebar.out_setcolor(titlebar.Caption,val);
		if(!strcmp(var,"fileselBorder")) filesel.out_setcolor(filesel.Border,val);
		if(!strcmp(var,"fileselSelect")) filesel.setcolor(filesel.Select,val);
		if(!strcmp(var,"fileselUnselect")) filesel.setcolor(filesel.Unselect,val);
		if(!strcmp(var,"fileselCaption")) filesel.out_setcolor(filesel.Caption,val);
		if(!strcmp(var,"infowndBorder")) infownd.out_setcolor(infownd.Border,val);
		if(!strcmp(var,"infowndIn")) infownd.out_setcolor(infownd.In,val);
		if(!strcmp(var,"infowndCaption")) infownd.out_setcolor(infownd.Caption,val);
	} while(!strchr(var,'[') && !f.eof());

	return true;
}

void select_colors()
{
	CListWnd	colwnd;
	char		inkey=0;
	bool		ext;

	colwnd.resize(MAXVAR,10);
	colwnd.setcaption("Colormaps");
	colwnd.setxy(10,10);
	listcolors(COLORFILE,colwnd);
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
			case 13:	// [Return] - select colorsheme
				loadcolors(COLORFILE,colwnd.getitem(colwnd.getselection()));
				break;
			}
	} while(inkey != 27 && inkey != 13);	// [ESC] - Exit menu
}

void refresh_songinfo(CTxtWnd &w)
{
	char			tmpstr[80],*title = new char [p->gettitle().length()+1];

	w.erase();
	sprintf(tmpstr,"Position   : %d / %d",p->getorder(),p->getorders()); w.puts(tmpstr);
	sprintf(tmpstr,"Pattern    : %d / %d",p->getpattern(),p->getpatterns()); w.puts(tmpstr);
	sprintf(tmpstr,"Row        : %d",p->getrow()); w.puts(tmpstr);
	sprintf(tmpstr,"Speed      : %d",p->getspeed()); w.puts(tmpstr);
	sprintf(tmpstr,"Timer      : %.2f Hz",p->getrefresh()); w.puts(tmpstr);
	sprintf(tmpstr,"Instruments: %d\n",p->getinstruments()); w.puts(tmpstr);
	w.update();
}

void refresh_songdesc(CTxtWnd &w)
{
	w.erase();
	w.setcaption("Song Message");
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

int main(int argc, char *argv[])
{
	CAdPlug		ap;
	CAnalopl		opl(DEFPORT);
	char			inkey=0,*prgdir,*curdir;
	bool			ext;
	unsigned char	volume=0;

	cout << ADPLAYVERS << ", (c) 2000 - 2001 Simon Peter (dn.tlp@gmx.net)" << endl;

	if(!strcmp(getenv("ADPLAY"),"S")) {
		cout << "AdPlay already running!" << endl;
		return 3;
	} else
		setenv("ADPLAY","S",1);

/*	if(!opl.detect()) {
		cout << "No OPL2 detected on port " << hex << DEFPORT << "h!" << endl;
		return 2;
	} */

	if(argc > 1)	// commandline playback
		if(!(p = ap.factory(argv[1],&opl))) {
			cout << "[" << argv[1] << "]: unsupported file type!" << endl;
			return 1;
		} else {
			cout << "Background playback... (type EXIT to stop)" << endl;
			tmInit(poll_player,0xffff,DEFSTACK);
			playing = true;
			_heapshrink();
			system(getenv("COMSPEC"));
			playing = false;
			tmClose();
			delete p;
			opl.init();
			return 0;
		}

	// init
	wnds.reg(titlebar); wnds.reg(filesel); wnds.reg(songwnd); wnds.reg(instwnd); wnds.reg(volbars);
	wnds.reg(mastervol); wnds.reg(infownd);
	prgdir = getcwd(NULL,0);
	loadcolors(COLORFILE,"default");
	setvideomode(3);
	load88font();
	hidecursor();
	clearscreen(backcol);
	tmInit(poll_player,0xffff,DEFSTACK);
	titlebar.puts(ADPLAYVERS ", (c) 2000 - 2001 by Simon Peter (dn.tlp@gmx.net) et al.");
	titlebar.puts("Look into the README file for additional author information.");
	titlebar.puts("This is free software under the terms and conditions of the LGPL.");
	songwnd.setcaption("Song Info"); instwnd.setcaption("Instrument Names"); volbars.setcaption("VBars");
	titlebar.setcaption(ADPLAYVERS); filesel.setcaption("File Selector"); mastervol.setcaption("Main Vol");
	volbars.resize(11,17); volbars.setxy(55,25);
	titlebar.resize(80,5); titlebar.setxy(0,0);
	infownd.resize(61,20); infownd.setxy(19,5);
	songwnd.resize(25,8); songwnd.setxy(55,42);
	instwnd.resize(36,25); instwnd.setxy(19,25);
	filesel.resize(19,45); filesel.setxy(0,5);
	listfiles(filesel);
	mastervol.resize(14,17); mastervol.setxy(66,25); mastervol.set(63);
	wnds.update();
	display_help(infownd);

	// main loop
	do {
		wait_retrace();

		if(playing) {	// auto-update windows
			refresh_songinfo(songwnd);
			refresh_volbars(volbars,opl);
		}

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
			case 59:	// [F1] - display help
				display_help(infownd);
				break;
			case 72:	// [Up Arrow] - menu up
				filesel.select_prev();
				filesel.update();
				break;
			case 80:	// [Down Arrow] - menu down
				filesel.select_next();
				filesel.update();
				break;
			case 73:	// [Page Up] - scroll up file info box
				instwnd.scroll_up();
				instwnd.update();
				break;
			case 81:	// [Page Down] - scroll down file info box
				instwnd.scroll_down();
				instwnd.update();
				break;
			}
		else		// handle all normal keys
			switch(inkey) {
			case 13:	// [Return] - play file / go to directory
				if(isdirectory(filesel.getitem(filesel.getselection()))) {
					chdir(filesel.getitem(filesel.getselection()));
					filesel.selectitem(0);
					listfiles(filesel);
					filesel.update();
					break;
				}
				playing = false;
				opl.init();
				if(!(p = ap.factory(filesel.getitem(filesel.getselection()),&opl))) {
					CTxtWnd errwnd;
					errwnd.resize(26,3);
					errwnd.setxy(10,10);
					errwnd.setcaption("Error!");
					errwnd.puts(" Unsupported file type!");
					errwnd.update();
					while(!getch());
					errwnd.hide();
					errwnd.update();
					break;
				} else {
					unsigned int	i;

					titlebar.erase();
					titlebar.outtext("File Type: "); titlebar.puts(p->gettype());
					titlebar.outtext("Title    : "); titlebar.puts(p->gettitle());
					titlebar.outtext("Author   : "); titlebar.puts(p->getauthor());
					titlebar.update();
					refresh_songdesc(infownd);
					instwnd.erase();
					for(i=0;i<p->getinstruments();i++)
						instwnd.puts(p->getinstrument(i));
					instwnd.update();
					playing = true;
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
				load88font();
				clearscreen(backcol);
				hidecursor();
				listfiles(filesel);
				wnds.update();
				break;
			case 'C':	// switch color scheme
				curdir = getcwd(NULL,0);
				chdir(prgdir);
				select_colors();
				chdir(curdir);
				free(curdir);
				clearscreen(backcol);
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
