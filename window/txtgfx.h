/*
 * txtgfx.h - Textmode Graphics Library, by Simon Peter (dn.tlp@gmx.net)
 *
 * This library offers commonly used textmode BIOS and hardware graphics
 * functions.
 */

#ifndef H_TXTGFX
#define H_TXTGFX

#ifdef __cplusplus
extern "C" {
#endif

void clearscreen(unsigned char col);
/* clear screen with given color and reset cursor position
 *
 * col	= color to reset screen with
 */

unsigned char getvideomode(void);
/* Get BIOS video mode
 *
 * Returns:
 * Video mode number as reported by BIOS.
 */

void setvideomode(unsigned char mode);
/* set BIOS video mode
 *
 * mode	= BIOS video mode number
 */

void load88font(void);
/* load & activate VGA 8x8 font (enables 50 rows) */

void showcursor(void);
/* show hardware cursor on screen */

void hidecursor(void);
/* hide hardware cursor */

void setcolor(unsigned char newcol);
/* set text output color
 *
 * newcol	= BIOS color number
 */

void settextposition(unsigned char ypos, unsigned char xpos);
/* set cursor position
 *
 * ypos	= Cursor Y-Position
 * xpos	= Cursor X-Position
 */

void outchar(char c);
/* write a char directly into video memory
 *
 * c	= character to write to video memory
 */

void outtext(char *str);
/* write a string directly into video memory
 *
 * str	= string to write to video memory
 */

extern void wait_retrace(void);
/* wait for vertical retrace */
#pragma aux wait_retrace = \
	"mov dx,03dah" \
"nope: in al,dx" \
	"test al,8" \
	"jz nope" \
"yepp: in al,dx" \
	"test al,8" \
	"jnz yepp" \
	modify [al dx];

#ifdef __cplusplus
}
#endif

#endif
