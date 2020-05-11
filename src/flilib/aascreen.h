#ifndef AASCREEN_H
#define AASCREEN_H

#ifndef AATYPES_H
#include "aatypes.h"
#endif /* AATYPES_H */

/* graphics types */
typedef unsigned char Cmap;
typedef unsigned char Pixel;
typedef unsigned char Bitplane;

/* constants pertaining to 320x200 256 color mode mostly */
#define AA_VGA_SCREEN	((Pixel *)0xa0000000)
#define AA_XMAX 	320
#define AA_YMAX 	200
#define AA_BPR 		320
#define AA_COLORS 	256

/* this structure is something we can draw on */
struct vscreen {
	int x, y;		/* upper left corner in screen coordinates */
	unsigned w, h;		/* width, height */
	unsigned bpr;		/* bytes per row of image p */
	Pixel *p;		/* starts on segment boundary */
	Cmap *cmap;
	long psize;		/* size of pixels */
	Pixel *allocedp;	/* for memory based screens only */
};
typedef struct vscreen Vscreen;

extern Vscreen aa_screen;
extern Cmap aa_colors[];	/* software echo of color map */

Boolean aa_open_vga_screen(void);	/* opens 256 color screen */
void aa_close_vga_screen(void);
/* open a screen can draw on but not see */
Vscreen *aa_alloc_mem_screen(void);	
/* for screens not full size */
Vscreen *aa_alloc_mem_cel(int x, int y, int w, int h);
void aa_free_mem_screen(Vscreen *ms);	/* dispose of a memory screen */
void aa_copy_screen(Vscreen *source, Vscreen *dest);
void aa_clear_screen(Vscreen *vs);
void aa_wait_vblank(void);		/* wait until in vertical blank */

#endif /* AASCREEN_H */
