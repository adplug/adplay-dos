/*
 * window.h - Textmode window library
 * Copyright (c) 2001, 2002, 2006 Simon Peter <dn.tlp@gmx.net>
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

#ifndef H_WINDOW_DEFINED
#define H_WINDOW_DEFINED

class CWindow
{
public:
        // Attributes
        enum Color {Border, In, Caption, Select, Unselect, Bar, Clip, Focus};

        // Static members
        static void setcolor(Color c, unsigned char v); // sets window color
        static unsigned char getcolor(Color c);         // returns window color
        static CWindow *getfocus();     // Returns currently focused window

        // (De-)Constructors
	CWindow();
	virtual ~CWindow();

        // Members
        void setcaption(const char *newcap);    // sets new window caption
        char *getcaption();                     // returns current window caption

        void center();  // center window on screen
	void setxy(unsigned char newx, unsigned char newy);	// sets new on-screen x/y position
        unsigned char posx() { return x; };
        unsigned char posy() { return y; };

	void resize(unsigned char newx, unsigned char newy);	// resizes the window
        unsigned char getsizex() { return sizex; };
        unsigned char getsizey() { return sizey; };

        void setfocus();                // Sets focus to this window

        void redraw();                  // redraws the window on screen
        virtual void update() {};       // updates window information

protected:
        static unsigned char color[];

        char *wndbuf;                   // inner-window "text" buffer
        unsigned char *colmap;          // inner-window color map
        unsigned char insizex,insizey;  // inner-window sizes
        bool autocolor;                 // automatically colorize output flag

	// tools
        void puts(char *str);           // like puts(), but in the window
        void outtext(char *str);        // outputs text, but does no linefeed
        void outc(char c);              // outputs the char c

	void setcursor(unsigned int newx, unsigned int newy)	// set window-cursor to new position
	{ curpos = newy*insizex+newx; };
        unsigned int wherex()           // returns cursor x-position
	{ return (curpos % insizex); };
        unsigned int wherey()           // returns cursor y-position
	{ return (curpos / insizex); };
        unsigned int getcursor()        // returns absolute cursor position inside buffer
	{ return curpos; };

        void clear();   // clears the window and resets cursor position

private:
	// positions, sizes & colors
        unsigned char x,y,sizex,sizey;  // window position and size
        unsigned int curpos;            // cursor position inside window text buffer

	// buffers
        char *caption;                  // window caption

        // Focus marker
        static CWindow *focus;          // Points on focused window
};

class CTxtWnd: public CWindow
{
public:
	CTxtWnd();
	~CTxtWnd()
	{ delete [] txtbuf; };

        void outtext(const char *str);          // outputs text, but does no linefeed
        void puts(const char *str)              // like puts(), but in the window
	{ outtext(str); outtext("\n"); };
        void format(const char *str);           // Fits the given string nicely into the window

        bool scroll_set(unsigned int line);
        bool scroll_down(unsigned int amount = 1);
        bool scroll_up(unsigned int amount = 1);

        void erase();                           // clears text buffer

	void update();

private:
	unsigned int txtpos,bufsize,start;
	char *txtbuf;
};

class CListWnd: public CWindow
{
public:
        class Item {
        public:
                enum Color {Selected, Unselected};

                Item();
                ~Item();

                void settext(const char *str);
                void setcolor(Color c, unsigned char v);

                char *gettext() { return text; }
                unsigned char getcolor(Color c) { return color[c]; }

        private:
                char *text;
                unsigned char color[2];
        };

	CListWnd();
        ~CListWnd();

        void additem(Item *newitem);    // insert item at beginning of list
        void removeall();               // removes all items from the list
        Item *getitem(unsigned int nr); // gets item at position nr
        void insertitem(Item *newitem, unsigned int nr);        // inserts item after position nr

        // Item selection
        bool select_next(unsigned int amount = 1);
        bool select_prev(unsigned int amount = 1);
        bool setselection(unsigned int nr);
        Item *getselection()            // returns the selected item
        { return selected->item; };

        void scroll_down();             // scroll down by 1 position
        void scroll_up();               // scroll up by 1 position

	void update();

private:
        struct ItemList {
                Item *item;
                ItemList *prev, *next;
        } *il, *selected, *start;

        unsigned int selpos;
};

class CBarWnd: public CWindow
{
public:
	CBarWnd(unsigned int n, unsigned int nmax);
        ~CBarWnd() { delete [] bars; };

	void set(unsigned int v, unsigned int n = 0);
	unsigned int get(unsigned int n = 0)
	{ return bars[n]; };

	void update();

private:
        unsigned int *bars,nbars,max;
};

class CErrWnd
{
public:
        static void message(const char *errtxt, const char *caption = "Error");
};

#endif
