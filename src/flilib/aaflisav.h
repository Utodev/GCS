#ifndef AAFLISAV_H
#define AAFLISAV_H

#ifndef AAFLI_H
#include "aafli.h"
#endif /* AAFLI_H */

/* compress a frame into a RAM buffer; return size of frame */
long fli_comp_frame(Cbuf *comp_buf,	/* buffer FLI_CBUF_SIZE or bigger */
  Pixel *last_screen, Cmap *last_cmap,	/* data from previous frame */
  Pixel *this_screen, Cmap *this_cmap,	/* data for this frame */
  int type);				/* FLI_BRUN?  FLI_LC? */

/* open FLI file and write out first frame; return file handle if things */
/* go well, error code otherwise */
Jfile fli_create(char *fliname, /* file name */
  Fli_head *fh, 		/* fli_create will initialize this */
  int speed);			/* speed in 1/70th of a second */

/* write succeeding frames of a FLI */
Errval fli_write_next(Jfile ff, /* FLI file returned by fli_create */
  Fli_head *fh, 		/* same header used by fli_create */
  Vscreen *this, 		/* current frame */
  Vscreen *last);		/* previous frame */

/* finish up writing a FLI file... */
/* write the 'ring frame', that is the difference between the first and */
/* last frame of a fli; pass in the final frame of the FLI in last_frame */
/* firstf_buf will be loaded with the first frame of the FLI as a side */
/* effect */
Errval fli_end(Jfile ff, Fli_head *fh, Vscreen *end_frame, Vscreen *firstf_buf);

#endif /* AAFLISAV_H */
