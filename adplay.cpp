/*
 * AdPlay/DOS - AdPlug DOS Frontend
 * Copyright (c) 2001 - 2006 Simon Peter <dn.tlp@gmx.net>
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

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <conio.h>
#include <adplug/adplug.h>
#include <adplug/analopl.h>
#include <window/txtgfx.h>
#include <window/window.h>
#include <window/wndman.h>

// DJGPP includes
#ifdef DJGPP
#	include <dir.h>
#	include <sys/stat.h>
#	include <unistd.h>
#	include <crt0.h>
#endif

#if defined HAVE_GCC_TIMER_H
#	include <gcctimer.h>
#else
#	error No suitable timer handling library found
#endif

#define TEMPNAM	tempnam

// Decide which _splitpath to use
#ifdef DJGPP
#	define SPLITPATH	fnsplit
#else
#	error No _splitpath equivalent found
#endif

#include "cfgparse.h"
#include "arcfile.h"
#include "filewnd.h"
#include "helptxt.h"

// global defines
#define ADPLAYVERS      "AdPlay 1.7"    // AdPlay version string
#define DEFSTACK        (32*1024)       // stack size for timer-replay code
#define CONFIGFILE      "adplay.ini"    // filename of default configuration file
#define DEFCONFIG       "default"       // name of default configuration section
#define FF_MSEC         5000            // milliseconds to fast forward
#define NORMALIZE       70.0f           // Timer frequency to normalize to

// Valid configuration variables
#define CONFIG_VARS     "Background\0AdlibPort\0PosX\0PosY\0SizeX\0SizeY\0" \
        "ColBorder\0ColCaption\0ColIn\0ColSelect\0ColUnselect\0ColBar\0" \
        "ColClip\0HighRes\0Force\0ColFileSel\0ColFileUnsel\0ColDirSel\0" \
        "ColDirUnsel\0ColDriveSel\0ColDriveUnsel\0Section\0ColFocus\0" \
	"Database\0ColSupportedSel\0ColSupportedUnsel\0SortBy\0OnSongEnd\0"

/*
 * Lock all memory under DJGPP. We're using most of it in a hardware interrupt
 * handler and we strive to support all kinds of DPMI hosts. Locking all the
 * right memory manually would be way too tedious.
 */
#ifdef DJGPP
int _crt0_startup_flags = _CRT0_FLAG_LOCK_MEMORY | _CRT0_FLAG_NO_LFN;
const static int PATH_MAX = 256;
#endif

// global variables
static CAdPlugDatabase mydb;                     // Global Database instance
static CAnalopl opl;                             // The only output device
static CPlayer *p=0;                             // Main player (0 = none loaded)
static CWndMan wnds;                             // Window manager
static CTxtWnd titlebar,infownd,songwnd,instwnd; // Windows
static FileWnd filesel;                          // File selector window
static CBarWnd volbars(9,63),mastervol(1,63);    // More windows
static unsigned char backcol=7;                  // Background color
static unsigned int subsong;
static int myoptind = 1;
static bool hivideo=true,oplforce=false;         // Configuration
static volatile float time_ms=0.0f;              // Current playing time in ms
static unsigned long totaltime = 0;              // Total time of current subsong
static const char *optstr = "h?pofcbq";          // Commandline options
static char configfile[PATH_MAX];                // Path to configfile
static VideoInfo dosvideo;                       // Stores previous (DOS's) video settings
static int volume=0;                             // Main volume
static float last_ms=0.0f;                       // helper variable for delay_ms()
static volatile bool inpoll = false;             // Flag set if in poll_player()
static volatile bool dopoll = false;             // Flag set if timer processing enabled
static char *tmpfn = 0;                          // Unique, temporary filename
static bool in_help;                             // Flag set when help is displayed
static volatile bool firsttime = true;           // First time playing flag for each song
static int onsongend = 0;                        // What to do on song end

// Debug precautions
#ifdef DEBUG
#define DEBUG_FILE      "adplay.log"     // File to log to

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

static void poll_player(void)
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

  if(!dopoll) return;	// Timer interrupt processing enabled?
  if(!p) return;	// no player in memory means we're not playing
  if(inpoll) return; else inpoll = true;  // Running...

  time_ms += 1000/(oldfreq*(wait+1));

  // pseudo timer environment
  if(del) {
    del--;          // just wait for this time fraction
    inpoll = false; return;
  } else
    del = wait;     // last fraction, reset for next one

  firsttime = p->update();    // call player

  if(oldfreq != p->getrefresh()) {        // new timer rate requested?
    oldfreq = p->getrefresh();
    del = wait = (unsigned int)(NORMALIZE / oldfreq);
#ifdef HAVE_WCC_TIMER_H
    tmSetNewRate(1192737/(oldfreq*(wait+1)));
#elif defined HAVE_GCC_TIMER_H
    timer_setrate((unsigned short)(1192737/(oldfreq*(wait+1))));
#endif
  }

  inpoll = false;         // ...and out again!
}

static void setadplugvideo()
  /* Setup AdPlay's idea of the video state */
{
  setvideomode(3);
  if(hivideo) load88font();
  clearscreen(backcol);
  hidecursor();
}

static bool dosshell(char *cmd)
  /* Runs the given command in a DOS shell. Returns true if succeeded. Saves
   * and restores the state of all required variables during the shell command.
   */
{
  int retval;

  setvideoinfo(&dosvideo);
  retval = system(cmd);
  setadplugvideo();

  if(retval) return false; else return true;
}

#define MAXINILINE	256

static void listcolors(char *fn, CListWnd &cl)
  /* Takes a CListWnd and rebuilds it with the color-sections list of
   * the given filename 'fn'.
   */
{
  std::ifstream f(fn, std::ios_base::in);
  char vglsec[MAXINILINE],var[MAXINILINE],*dummy;
  CListWnd::Item *item;

  if(!f.is_open()) return;                // file not found?

  cl.removeall();
  do {					// list sections
    f.getline(vglsec,MAXINILINE);
    if(sscanf(vglsec," [%s",var) && *vglsec) {
      dummy = strrchr(var,']');
      *dummy = '\0';
      item = new CListWnd::Item;
      item->settext(var);
      cl.additem(item);
    }
  } while(!f.eof());
}

#undef MAXINILINE

static void display_help(CTxtWnd &w)
  /* Displays AdPlay's main help text into the given CTxtWnd. */
{
  w.erase();
  w.setcaption("Help");
  w.format(ONLINE_HELP);
  in_help = true;
}

static bool read_stdwnd(CfgParse &cp, const char *subsec, const char *section,
  			CWindow &w)
  /* Sets the configured positions for a given window. Returns true if
   * succeeded, false otherwise.
   */
{
  if(!cp.subsection(subsec,section)) return false;

  while(!cp.geterror())
    switch(cp.peekvar()) {
    case 2: w.setxy(cp.readlong(),w.posy()); break;
    case 3: w.setxy(w.posx(),cp.readlong()); break;
    case 4: w.resize(cp.readlong(),w.getsizey()); break;
    case 5: w.resize(w.getsizex(),cp.readlong()); break;
    }

  if(cp.geterror() == CfgParse::Invalid) return false; else return true;
}

static bool loadconfig(const char *fn, const char *section)
  /* Loads and sets only the general configuration out of the file, named by
   * 'fn', using the section 'section' therein. Returns true, if succeeded.
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
    case 23: mydb.load(cp.readstr()); break;
    case 26:
      if(!stricmp(cp.readstr(), "name"))
	filesel.sortby = FileWnd::SortByName;
      else if(!stricmp(cp.readstr(), "extension"))
	filesel.sortby = FileWnd::SortByExtension;
      break;
    case 27:
      if(!stricmp(cp.readstr(), "nothing")) onsongend = 0;
      else if(!stricmp(cp.readstr(), "rewind")) onsongend = 1;
      else if(!stricmp(cp.readstr(), "stop")) onsongend = 2;
      break;
    }
  } while(!cp.geterror());

  if(cp.geterror() == CfgParse::Invalid) return false; else return true;
}

static bool loadcolors(const char *fn, const char *section)
  /* Loads and sets only the color-definition out of the file, named by 'fn',
   * using the section 'section' therein. Returns true, if succeeded.
   */
{
  CfgParse cp(fn);

  if(cp.geterror() == CfgParse::NotFound)       // file not found?
    return false;

  // Define config variables
  cp.enum_vars(CONFIG_VARS);

  // Read sizes/positions of windows
  read_stdwnd(cp,"titlebar",section,titlebar);
  read_stdwnd(cp,"infownd",section,infownd);
  read_stdwnd(cp,"songwnd",section,songwnd);
  read_stdwnd(cp,"instwnd",section,instwnd);
  read_stdwnd(cp,"filesel",section,filesel);
  read_stdwnd(cp,"volbars",section,volbars);
  read_stdwnd(cp,"mastervol",section,mastervol);

  // Read GUI color configuration
  if(!cp.section(section)) return false;
  do {
    switch(cp.peekvar()) {
    case 0: backcol = cp.readlong(); break;
    case 6: CWindow::setcolor(CWindow::Border,cp.readlong()); break;
    case 7: CWindow::setcolor(CWindow::Caption,cp.readlong()); break;
    case 8: CWindow::setcolor(CWindow::In,cp.readlong()); break;
    case 9: CWindow::setcolor(CWindow::Select,cp.readlong()); break;
    case 10: CWindow::setcolor(CWindow::Unselect,cp.readlong()); break;
    case 11: CWindow::setcolor(CWindow::Bar,cp.readlong()); break;
    case 12: CWindow::setcolor(CWindow::Clip,cp.readlong()); break;
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
    case 15: FileWnd::setfilecolor(FileWnd::FileSel,cp.readlong()); break;
    case 16: FileWnd::setfilecolor(FileWnd::FileUnsel,cp.readlong()); break;
    case 17: FileWnd::setfilecolor(FileWnd::DirSel,cp.readlong()); break;
    case 18: FileWnd::setfilecolor(FileWnd::DirUnsel,cp.readlong()); break;
    case 19: FileWnd::setfilecolor(FileWnd::DriveSel,cp.readlong()); break;
    case 20: FileWnd::setfilecolor(FileWnd::DriveUnsel,cp.readlong()); break;
    case 21: loadcolors(fn,cp.readstr()); break;
    case 22: CWindow::setcolor(CWindow::Focus,cp.readlong()); break;
    case 24: FileWnd::setfilecolor(FileWnd::SupportedSel,cp.readlong()); break;
    case 25: FileWnd::setfilecolor(FileWnd::SupportedUnsel,cp.readlong()); break;
    }
  } while(!cp.geterror());

  if(in_help) display_help(infownd);      // Reformat help display, if active
  if(cp.geterror() == CfgParse::Invalid) return false; else return true;
}

static void select_colors()
  /* Displays the "Change Screen Layout" window and lets the user select
   * a new layout and sets this layout afterwards.
   */
{
  CListWnd	colwnd;
  char		inkey=0;
  bool		ext;

  colwnd.resize(20,10); colwnd.setcaption("Layouts"); colwnd.center();
  listcolors(configfile,colwnd); colwnd.update();

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
      case 73:	// [PgUp] - Up 5 items
	colwnd.select_prev(5);
	colwnd.update();
	break;
      case 81:	// [PgDn] - Down 5 items
	colwnd.select_next(5);
	colwnd.update();
	break;
      case 71:	// [Home] - Go to beginning
	colwnd.setselection(0);
	colwnd.update();
	break;
      case 79:	// [End] - Go to end
	colwnd.setselection(0xFFFF);
	colwnd.update();
	break;
      }
    else		// handle all normal keys
      switch(inkey) {
      case 13:	// [Return] - select layout
	loadcolors(configfile,colwnd.getselection()->gettext());
	break;
      }
  } while(inkey != 27 && inkey != 13);	// [ESC] - Exit menu
}

static void refresh_songinfo(CTxtWnd &w)
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
  sprintf(tmpstr,"Total   : %lu:%2lu.%2lu",totaltime/1000/60,totaltime/1000%60,
	  totaltime/10%100); w.puts(tmpstr);
  w.update();
}

static void refresh_songdesc(CTxtWnd &w)
  /* Refreshes the "Song Message" window with data from the loaded player. */
{
  w.erase();
  w.setcaption("Song Message");
  if(p) w.puts(p->getdesc().c_str());
  w.update();
  in_help = false;
}

static void refresh_volbars(CBarWnd &w, CAnalopl &opl)
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

static unsigned int getopt(int n, char **s)
  /* Returns an index into the optstr, on which option has been set on the
   * commandline 's' (argv), with 'n' maximum options (argc).
   */
{
  unsigned int	i;

  if(myoptind >= n)
    return 0;

  for(i=0;i<strlen(optstr);i++) {
    if(s[myoptind][0] != '-' && s[myoptind][0] != '/') return 0;
    if(optstr[i] == s[myoptind][1]) break;
  }
  myoptind++;
  return (i+1);
}

static void fast_forward(const unsigned int ms)
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

static char *extract(char *newfn, archive *a, char *oldfn)
  /* Extract file 'oldfn' from archive 'a'. The filename of the extracted file
   * is stored in 'newfn'. 0 is returned if an error occured.
   */
{
  char cmd[256];

  dbg_printf("*** extract(newfn,a,\"%s\") ***\n",oldfn);

  if(!tmpfn) return 0;    // No possible unique filename?
  strcpy(cmd,"pkunzip ");
  strcat(cmd,a->getarcname());
  strcat(cmd," \"");
  strcat(cmd,oldfn);
  strcat(cmd,"\" ");
  strcat(cmd,tmpfn);

  dbg_printf("running command: \"%s\"\n",cmd);

  if(!dosshell(cmd)) {    // If an error occured...
    dbg_printf("An error occured while shelling!\n");
    puts("An error occured while running the command:");
    printf("%s\n",cmd);
    puts("Please consult the above error messages, if any, and "
	 "press any key to continue...");
    getch();
    wnds.update();
    return 0;
  }
  wnds.update();
  strcpy(newfn,tmpfn);
  strcat(newfn,"\\");
  strcat(newfn,strrchr(oldfn,'/') ? strrchr(oldfn,'/') + 1 : oldfn);

  dbg_printf("Returning: %s\n",newfn);
  dbg_printf("--- extract ---\n");
  return newfn;
}

static void reset_windows()
  /* Resets all windows to the initial state at startup. */
{
  unsigned int i;

  titlebar.erase();
  titlebar.format(ADPLAYVERS ", Copyright (c) 2000 - 2006 Simon Peter <dn.tlp@gmx.net>\n"
		  "This is free software under the terms and conditions of the Nullsoft license.\n"
		  "Refer to the file README.TXT for more information.");
  instwnd.setcaption("No Instruments"); instwnd.erase();
  for(i=0;i<9;i++) volbars.set(0,i); 
  songwnd.erase(); if(!in_help) infownd.erase(); wnds.update();
}

static void stop()
  /* Stops currently playing tune. Does nothing, if not playing. */
{
  if(!p) return;
  dbg_printf("stop(): A player is running. Stopping it.\n");
  dopoll = false; while(inpoll) ;         // Wait for timer routine
  delete p; p = 0;
  opl.init();
  dopoll = true;
}

static void play(char *fn)
  /* Start playback with file 'fn' */
{
  dbg_printf("*** play(\"%s\") ***\n",fn);

  stop();         // Stop eventually already playing player
  dopoll = false;	// Critical section... (stop timer)
  p = CAdPlug::factory(fn,&opl);  // get corresponding player

  if(!p) {        // File not supported error
    dbg_printf("AdPlug's factory returned a NULL pointer! Bail out!\n");
    CErrWnd::message("Unsupported file type!");
    opl.init();
    while(!getch());
    reset_windows();
    dbg_printf("--- play() ---\n");
    return;
  }

  dbg_printf("We got a player! Reset infos and start playing...\n");

  // Reset playing infos
  last_ms = time_ms = 0.0f; subsong = 0; totaltime = p->songlength(subsong);

  dopoll = true;	// ...End Critical section (resume timer)

  // Update view with file information
  unsigned int i;
  char ins[20];

  // Update titlebar
  titlebar.erase();
  titlebar.outtext("File Type: "); titlebar.puts(p->gettype().c_str());
  titlebar.outtext("Title    : "); titlebar.puts(p->gettitle().c_str());
  titlebar.outtext("Author   : "); titlebar.puts(p->getauthor().c_str());
  titlebar.update();

  refresh_songdesc(infownd);      // Update "Song Info" window

  // Update instruments window
  instwnd.erase();
  for(i=0;i<p->getinstruments();i++) {
    sprintf(ins,"%3d³",i+1);
    instwnd.outtext(ins);
    instwnd.puts(p->getinstrument(i).c_str());
  }
  sprintf(ins,"%d Instruments",p->getinstruments());
  instwnd.setcaption(ins);
  instwnd.update();

  dbg_printf("--- play() ---\n");
}

static void adjust_volume(int amount)
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

static void activate()
  /* This function knows what to do, when a file inside the file manager is
   * selected by the user.
   */
{
  bool result;
  char tmpfn[PATH_MAX];

  dbg_printf("activate(): Querying filesel.select()...\n");
  result = filesel.select();

  // Drive not ready error?
  if(filesel.geterror() == FileWnd::Drive_NotReady) {
    dbg_printf("activate(): filesel.geterror() said \"Drive not ready\".\n");
    CErrWnd::message("Drive not ready!");
    while(!getch());
    wnds.update();
    return;
  }

  if(result) {    // It's our file!
    dbg_printf("activate(): It's our file! Try playback...\n");
    // Start playback
    if(filesel.inarchive()) {
      char extract_name[PATH_MAX];
      if(!extract(extract_name,filesel.getarchive(),
		  filesel.getfilename(tmpfn)))
	return;
      play(extract_name);
      remove(extract_name);
    } else
      play(filesel.getfilename(tmpfn));
  } else {        // File manager knows how to handle...
    dbg_printf("activate(): filesel knows how to handle...\n");
    // Refresh file manager window
    filesel.refresh();
    filesel.update();
  }
}

static void idle_ms(unsigned int ms)
  /* Delays (idles) for an amount of 'ms' milliseconds.
   * Use this function instead of the C library's delay() function!
   */
{
  while(time_ms < last_ms+ms) delay(0);
  last_ms = time_ms;
}

static void window_cycle(bool backward = false)
{
  CWindow *focus = CWindow::getfocus();

  if(!backward) {
    if(focus == &filesel) infownd.setfocus();
    else if(focus == &infownd) instwnd.setfocus();
    else if(focus == &instwnd) filesel.setfocus();
  } else {
    if(focus == &filesel) instwnd.setfocus();
    else if(focus == &infownd) filesel.setfocus();
    else if(focus == &instwnd) infownd.setfocus();
  }

  wnds.update();
}

int main(int argc, char *argv[])
{
  char          inkey=0, *prgdir, *curdir, *program_name;
  bool          ext, validcfg, quit = false, bkgply = false, batchply = false;
  unsigned int	opt, prgdrive, i;
  CWindow       *focus;

#ifdef DEBUG
  f_log = fopen(DEBUG_FILE,"wt");
#endif

  std::cout << ADPLAYVERS << ", Copyright (c) 2000 - 2006 Simon Peter <dn.tlp@gmx.net>" << std::endl << std::endl;

  // check that no other instance is running
  {
    char *adplayenv = getenv("ADPLAY");

    if(adplayenv && !strcmp(adplayenv,"S")) {
      std::cout << "AdPlay already running!" << std::endl;
      exit(EXIT_FAILURE);
    } else
      setenv("ADPLAY","S",1); // flag our instance
  }

  // Build program executable name
  program_name = strrchr(argv[0], '\\') ? strrchr(argv[0], '\\') + 1 : argv[0];

  CAdPlug::debug_output("debug.log"); // Redirect AdPlug's debug to file
  // Build path to default configuration file (in program's directory)
  SPLITPATH(argv[0],configfile,configfile+2,NULL,NULL);
  strcat(configfile,CONFIGFILE);

  loadconfig(configfile,DEFCONFIG);       // load default configuration

  // parse commandline for general options
  while((opt = getopt(argc,argv)))
    switch(opt) {
    case 1:	// display help
    case 2:
      std::cout << "Usage: " << program_name << " [options]" << std::endl << std::endl;
      std::cout << "Options can be set with '-' or '/' respectively." << std::endl << std::endl;
      std::cout << " -?, -h      Display commandline help" << std::endl <<
	" -p port     Set OPL2 port" << std::endl <<
	" -o          Force OPL2 port" << std::endl <<
	" -f file     Use alternate configuration file" << std::endl <<
	" -c section  Load another configuration section" << std::endl <<
	" -b file     Immediate background playback using " <<
	"specified file" << std::endl <<
	" -q files    Immediate (batch mode) playback using " <<
	"specified files" << std::endl;
      showcursor();
      exit(EXIT_SUCCESS);
    case 3:	// set OPL2 port
      opl.setport(atoi(argv[myoptind++]));
      break;
    case 4: // force OPL2 port
      oplforce = true;
      break;
    case 7:	// background playback
      bkgply = true;
      break;
    case 8: // batch mode playback
      batchply = true;
      break;
    }

  // Bail out if OPL2 not detected and not force
  if(!opl.detect() && !oplforce) {
    std::cout << "No OPL2 detected!" << std::endl;
    showcursor();
    exit(EXIT_FAILURE);
  }

  // Hand our database to AdPlug
  CAdPlug::set_database(&mydb);

  /*** Background playback mode ***/
  if(bkgply)
    if(!(p = CAdPlug::factory(argv[myoptind],&opl))) {
      std::cout << "[" << argv[myoptind] << "]: unsupported file type!" << std::endl;
      exit(EXIT_FAILURE);
    } else {
      std::cout << "Background playback... (type EXIT to stop)" << std::endl;
#ifdef HAVE_WCC_TIMER_H
      tmInit(poll_player,0xffff,DEFSTACK);
#elif defined HAVE_GCC_TIMER_H
      timer_init(poll_player);
#endif
      dopoll = true;
      system(getenv("COMSPEC"));
#ifdef HAVE_WCC_TIMER_H
      tmClose();
#elif defined HAVE_GCC_TIMER_H
      timer_deinit();
#endif
      stop();
      exit(EXIT_SUCCESS);
    }

  /*** Batch playback mode ***/
  if(batchply) {
#ifdef HAVE_WCC_TIMER_H
    tmInit(poll_player,0xffff,DEFSTACK);
#elif defined HAVE_GCC_TIMER_H
    timer_init(poll_player);
#endif

    for(i = myoptind; i < argc; i++)
      if(!(p = CAdPlug::factory(argv[i],&opl))) {
	std::cout << "[" << argv[i] << "]: unsupported file type!" << std::endl;
#ifdef HAVE_WCC_TIMER_H
	tmClose();
#elif defined HAVE_GCC_TIMER_H
	timer_deinit();
#endif
	exit(EXIT_FAILURE);
      } else {
	dopoll = firsttime = true;
	std::cout << "Playing [" << argv[i] << "] ..." << std::endl;
	while(firsttime) ;	// busy waiting
	stop();
	dopoll = false;
      }

#ifdef HAVE_WCC_TIMER_H
    tmClose();
#elif defined HAVE_GCC_TIMER_H
    timer_deinit();
#endif
    exit(EXIT_SUCCESS);
  }

  /*** interactive (GUI) mode ***/
  getvideoinfo(&dosvideo);        // Save previous video state

  // register our windows with the window manager
  wnds.reg(titlebar); wnds.reg(filesel); wnds.reg(songwnd);
  wnds.reg(instwnd); wnds.reg(volbars); wnds.reg(mastervol);
  wnds.reg(infownd);

  // load default GUI layout
  validcfg = loadcolors(configfile,DEFCONFIG);

  // reparse commandline for GUI options
  myoptind = 1;     // reset option parser
  while((opt = getopt(argc,argv)))
    switch(opt) {
    case 5:	// set config file
      strcpy(configfile,argv[myoptind++]);
      if(loadcolors(configfile,DEFCONFIG))
	validcfg = true;
      break;
    case 6:	// load config section
      loadcolors(configfile,argv[myoptind++]);
      break;
    }

  // bail out if no configfile could be loaded
  if(!validcfg) {
    std::cout << "No valid default GUI layout could be loaded!" << std::endl;
    exit(EXIT_FAILURE);
  }

  // init GUI
  if((tmpfn = TEMPNAM(getenv("TEMP"),"_AP")))
  mkdir(tmpfn, S_IWUSR);

  prgdir = getcwd(NULL, PATH_MAX); _dos_getdrive(&prgdrive);
  setadplugvideo();
#ifdef HAVE_GCC_TIMER_H
  timer_init(poll_player);
#endif
  songwnd.setcaption("Song Info"); volbars.setcaption("VBars");
  titlebar.setcaption(ADPLAYVERS); filesel.setcaption("Directory");
  mastervol.setcaption("Vol"); filesel.refresh(); mastervol.set(63);
  display_help(infownd); filesel.setfocus(); reset_windows();

  // main loop
  do {
    if(p) {	// auto-update windows
      //                        wait_retrace();
      idle_ms(1000/70);
      refresh_songinfo(songwnd);
      refresh_volbars(volbars,opl);

      if(onsongend && !firsttime) {	// song ended
	switch(onsongend) {
	case 1:	// auto-rewind
	  dopoll = false; while(inpoll) ;	// critical section...
	  p->rewind(subsong);
	  last_ms = time_ms = 0.0f;
	  dopoll = true;	// ...End critical section
	  break;
	case 2:	// stop playback
	  stop();
	  reset_windows();
	  break;
	}
      }
    }

    // Check for keypress and read in, if any
    if(kbhit()) {
      if(!(inkey = toupper(getch()))) {
	ext = true;
	inkey = toupper(getch());
      } else
	ext = false;

      focus = CWindow::getfocus(); // cache focused window
      dbg_printf("main(): Key pressed: %d %s\n",
		 inkey, ext ? "(Ext)" : "(Norm)");
    } else
      inkey = 0;

    if(ext)	// handle all extended keys
      switch(inkey) {
      case 15:        // [Shift]+[TAB] - Back cycle windows
	window_cycle(true);
	break;
      case 59:	// [F1] - display help
	display_help(infownd);
	infownd.setfocus();
	wnds.update();
	break;
      case 60:	// [F2] - change screen layout
	curdir = getcwd(NULL, PATH_MAX);
	chdir(prgdir);
	select_colors();
	chdir(curdir);
	free(curdir);
	clearscreen(backcol);
	filesel.refresh();
	wnds.update();
	break;
      case 72:        // [Up Arrow] - scroll up
	if(focus == &filesel) {
	  filesel.select_prev();
	  filesel.update();
	} else if(focus == &infownd) {
	  infownd.scroll_up();
	  infownd.update();
	} else if(focus == &instwnd) {
	  instwnd.scroll_up();
	  instwnd.update();
	}
	break;
      case 80:        // [Down Arrow] - scroll down
	if(focus == &filesel) {
	  filesel.select_next();
	  filesel.update();
	} else if(focus == &infownd) {
	  infownd.scroll_down();
	  infownd.update();
	} else if(focus == &instwnd) {
	  instwnd.scroll_down();
	  instwnd.update();
	}
	break;
      case 75:	// [Left Arrow] - previous subsong
	if(p && subsong) {
	  subsong--;
	  dopoll = false; while(inpoll) ;	// critical section...
	  totaltime = p->songlength(subsong);
	  p->rewind(subsong);
	  last_ms = time_ms = 0.0f;
	  dopoll = true;	// ...End critical section
	}
	break;
      case 77:	// [Right Arrow] - next subsong
	if(p && subsong < p->getsubsongs()-1) {
	  subsong++;
	  dopoll = false; while(inpoll) ;	// critical section...
	  totaltime = p->songlength(subsong);
	  p->rewind(subsong);
	  last_ms = time_ms = 0.0f;
	  dopoll = true;	// ...End critical section
	}
	break;
      case 73:        // [Page Up] - scroll up half window
	if(focus == &filesel) {
	  filesel.select_prev(filesel.getsizey() / 2);
	  filesel.update();
	} else if(focus == &infownd) {
	  infownd.scroll_up(infownd.getsizey() / 2);
	  infownd.update();
	} else if(focus == &instwnd) {
	  instwnd.scroll_up(instwnd.getsizey() / 2);
	  instwnd.update();
	}
	break;
      case 81:        // [Page Down] - scroll down half window
	if(focus == &filesel) {
	  filesel.select_next(filesel.getsizey() / 2);
	  filesel.update();
	} else if(focus == &infownd) {
	  infownd.scroll_down(infownd.getsizey() / 2);
	  infownd.update();
	} else if(focus == &instwnd) {
	  instwnd.scroll_down(instwnd.getsizey() / 2);
	  instwnd.update();
	}
	break;
      case 71:        // [Home] - scroll to start
	if(focus == &filesel) {
	  filesel.setselection(0);
	  filesel.update();
	} else if(focus == &infownd) {
	  infownd.scroll_set(0);
	  infownd.update();
	} else if(focus == &instwnd) {
	  instwnd.scroll_set(0);
	  instwnd.update();
	}
	break;
      case 79:        // [End] - scroll to end
	if(focus == &filesel) {
	  filesel.setselection(0xffff);
	  filesel.update();
	} else if(focus == &infownd) {
	  infownd.scroll_set(0xffff);
	  infownd.update();
	} else if(focus == &instwnd) {
	  instwnd.scroll_set(0xffff);
	  instwnd.update();
	}
	break;
      }
    else		// handle all normal keys
      switch(inkey) {
      case 9:         // [TAB] - Cycle through windows
	window_cycle();
	break;
      case 13:        // [Return] - Activate
	if(focus == &filesel)
	  activate();
	break;
      case 27:        // [ESC] - Stop music / Exit to DOS
	if(p) {
	  stop();
	  reset_windows();
	} else
	  quit = true;
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
  } while(!quit);

  // deinit
#ifdef HAVE_WCC_TIMER_H
    tmClose();
#elif defined HAVE_GCC_TIMER_H
    timer_deinit();
#endif
  stop();
  setvideoinfo(&dosvideo);
  {
    unsigned int dummy;
    _dos_setdrive(prgdrive, &dummy);
  }
  chdir(prgdir);
  free(prgdir);
  if(tmpfn) { rmdir(tmpfn); free(tmpfn); }
#ifdef DEBUG
  dbg_printf("main(): clean shutdown.\n");
  fclose(f_log);
#endif
  return EXIT_SUCCESS;
}
