#ifndef AAFII_H
#define AAFII_H

#ifndef AAFLI_H
#include "aafli.h"
#endif /* AAFLI_H */

/* longest run for byte-oriented compression */
#define FLI_MAX_RUN 127

/* low level decompression functions */
void fii_unbrun(Cbuf *cbuf, Pixel *screen, int linect);	/* 1st frame */
void fii_unlccomp(Cbuf *cbuf, Pixel *screen);		/* delta frames */
void fii_mem_fcuncomp(Cbuf *cbuf, Cmap *cmap);		/* colors to memory */
void fii_reg_fcuncomp(Cbuf *cbuf);	     		/* colors to registers */

/* low level assembler routines used by fii_lccomp to do a single line */
/* find out how far until have the next match of mustmatch or more pixels */
int fii_tnskip(Pixel *s1, Pixel *s2, int bcount, int mustmatch);
/* find out how far until next run of identical pixels mustmatch long */
int fii_tnsame(Pixel *s, int bcount, int mustmatch);

/* mid level routines to compress 1st frame, delta frames, and color maps */
/* run length compress a single frame using Animator 8086 1st frame technique */
Cbuf *fii_brun(Pixel *s1, USHORT *cbuf, int width, int height);
/* delta compress a single frame using Animator 8086 byte-oriented scheme */
Cbuf *fii_lccomp(Pixel *s1, Pixel *s2, USHORT *cbuf, int width, int height);
/* compress an rgb triples color map doing 'skip' compression */
Cbuf *fii_fccomp(Cmap *s1, Cmap *s2, USHORT *cbuf, int count);

#endif /* AAFII_H */
