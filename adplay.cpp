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

#include "txtgfx.h"
#include "window.h"
#include "timer.h"
#include "bars.h"

#include "adplug.h"
#include "analopl.h"

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
CTxtWnd			titlebar(80,5,0,0),infownd(61,20,19,5),songwnd(25,8,55,42),instwnd(36,25,19,25);
CListWnd			filesel(19,45,0,5);
CBarWnd			volbars(9,63),mastervol(1,63);
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
	w.clear();
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
	w.redraw();
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
		if(!strcmp(var,"titleBorder")) titlebar.setcolor(titlebar.Border,val);
		if(!strcmp(var,"titleIn")) titlebar.setcolor(titlebar.In,val);
		if(!strcmp(var,"titleCaption")) titlebar.setcolor(titlebar.Caption,val);
		if(!strcmp(var,"fileselBorder")) filesel.setcolor(filesel.Border,val);
		if(!strcmp(var,"fileselSelect")) filesel.setcolor(filesel.Select,val);
		if(!strcmp(var,"fileselUnselect")) filesel.setcolor(filesel.Unselect,val);
		if(!strcmp(var,"fileselCaption")) filesel.setcolor(filesel.Caption,val);
		if(!strcmp(var,"infowndBorder")) infownd.setcolor(infownd.Border,val);
		if(!strcmp(var,"infowndIn")) infownd.setcolor(infownd.In,val);
		if(!strcmp(var,"infowndCaption")) infownd.setcolor(infownd.Caption,val);
	} while(!strchr(var,'[') && !f.eof());

	return true;
}

void select_colors()
{
	CListWnd	colwnd(MAXVAR,10);
	char		inkey=0;
	bool		ext;

	colwnd.setcaption("Colormaps");
	colwnd.center();
	listcolors(COLORFILE,colwnd);
	colwnd.redraw();

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
				colwnd.redraw();
				break;
			case 80:	// [Down Arrow] - menu down
				colwnd.select_next();
				colwnd.redraw();
				break;
			}
		else		// handle all normal keys
			switch(inkey) {
			case 13:	// [Return] - select colorsheme
				loadcolors(COLORFILE,colwnd.getitem(colwnd.getselection()));
				clearscreen(backcol);
				filesel.redraw();
				if(playing)
					infownd.redraw();
				break;
			}
	} while(inkey != 27 && inkey != 13);	// [ESC] - Exit menu
}

void refresh_songinfo(CTxtWnd &w)
{
	char			tmpstr[80],*title = new char [p->gettitle().length()+1];

	w.clear();
	sprintf(tmpstr,"Position   : %d / %d",p->getorder(),p->getorders()); w.puts(tmpstr);
	sprintf(tmpstr,"Pattern    : %d / %d",p->getpattern(),p->getpatterns()); w.puts(tmpstr);
	sprintf(tmpstr,"Row        : %d",p->getrow()); w.puts(tmpstr);
	sprintf(tmpstr,"Speed      : %d",p->getspeed()); w.puts(tmpstr);
	sprintf(tmpstr,"Timer      : %.2f Hz",p->getrefresh()); w.puts(tmpstr);
	sprintf(tmpstr,"Instruments: %d\n",p->getinstruments()); w.puts(tmpstr);
	w.redraw();
}

void refresh_songdesc(CTxtWnd &w)
{
	w.clear();
	w.setcaption("Song Message");
	w.puts(p->getdesc());
	w.redraw();
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
	w.redraw();
}

int main(int argc, char *argv[])
{
	CAdPlug		ap;
	CAnalopl		opl(DEFPORT);
	char			inkey=0,*prgdir,*curdir;
	bool			ext;
	unsigned char	volume=0;

	cout << ADPLAYVERS << ", (c) 2000 - 2001 Simon Peter (dn.tlp@gmx.net)" << endl;

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
	prgdir = getcwd(NULL,0);
	loadcolors(COLORFILE,"default");
	setvideomode(3);
	load88font();
	hidecursor();
	clearscreen(backcol);
	tmInit(poll_player,0xffff,DEFSTACK);
	titlebar.clear(); songwnd.clear(); infownd.clear(); instwnd.clear();
	titlebar.puts(ADPLAYVERS ", (c) 2000 - 2001 by Simon Peter (dn.tlp@gmx.net) et al.");
	titlebar.puts("Look into the README file for additional author information.");
	titlebar.puts("This is free software under the terms and conditions of the LGPL.");
	titlebar.setcaption(ADPLAYVERS);
	titlebar.redraw();
	filesel.setcaption("File Selector");
	listfiles(filesel);
	filesel.redraw();
	songwnd.setcaption("Song Info");
	instwnd.setcaption("Instrument Names");
	volbars.setcaption("VBars");
	mastervol.setcaption("Main Vol");
	volbars.resize(11,17); volbars.setxy(55,25);
	mastervol.resize(14,17); mastervol.setxy(66,25); mastervol.set(63);
	songwnd.redraw(); instwnd.redraw(); volbars.redraw(); mastervol.redraw();
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
				filesel.redraw();
				break;
			case 80:	// [Down Arrow] - menu down
				filesel.select_next();
				filesel.redraw();
				break;
			case 73:	// [Page Up] - scroll up file info box
				instwnd.scroll_up();
				instwnd.redraw();
				break;
			case 81:	// [Page Down] - scroll down file info box
				instwnd.scroll_down();
				instwnd.redraw();
				break;
			}
		else		// handle all normal keys
			switch(inkey) {
			case 13:	// [Return] - play file / go to directory
				if(isdirectory(filesel.getitem(filesel.getselection()))) {
					chdir(filesel.getitem(filesel.getselection()));
					filesel.selectitem(0);
					listfiles(filesel);
					filesel.redraw();
					break;
				}
				playing = false;
				opl.init();
				if(!(p = ap.factory(filesel.getitem(filesel.getselection()),&opl))) {
					CTxtWnd	errwnd(26,3);
					errwnd.setcaption("Error!");
					errwnd.puts(" Unsupported file type!");
					errwnd.center();
					errwnd.redraw();
					while(!getch());
					errwnd.hide();
					errwnd.redraw();
					break;
				} else {
					unsigned int	i;

					titlebar.clear();
					titlebar.outtext("File Type: "); titlebar.puts(p->gettype());
					titlebar.outtext("Title    : "); titlebar.puts(p->gettitle());
					titlebar.outtext("Author   : "); titlebar.puts(p->getauthor());
					titlebar.redraw();
					refresh_songdesc(infownd);
					instwnd.clear();
					for(i=0;i<p->getinstruments();i++)
						instwnd.puts(p->getinstrument(i));
					instwnd.redraw();
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
				clearscreen(backcol);
				hidecursor();
				listfiles(filesel);
				filesel.redraw();
				break;
			case 'C':	// switch color scheme
				curdir = getcwd(NULL,0);
				chdir(prgdir);
				select_colors();
				chdir(curdir);
				free(curdir);
				break;
			case '+':
				if(volume) {
					volume--;
					opl.setvolume(volume);
					mastervol.set(63 - volume);
					mastervol.redraw();
				}
				break;
			case '-':
				if(volume < 63) {
					volume++;
					opl.setvolume(volume);
					mastervol.set(63 - volume);
					mastervol.redraw();
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
