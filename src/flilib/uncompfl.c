#include "jlib.h"
#include "aafli.h"
#include "aafii.h"

/* ok, now we've read in a frame of a FLI/FLX file ok; given a screen that */
/* has the last frame on it and the data for this frame, this function */
/* will switch through the chunks of the frame data updating the screen */
/* in the process; the 'colors' parameter indicates whether we should */
/* update the hardware color map as well as the ram echo */
void fli_uncomp(Vscreen *f, Fli_frame *frame, Cbuf *cbuf, Boolean see_colors)
{
int j;
struct fli_chunk *chunk;
Cbuf *cb1;

if(see_colors) aa_wait_vblank();

for(j=0; j<frame->chunks; j++) {
	chunk=(struct fli_chunk *)cbuf;	/* start with chunk header */
	cb1=(Cbuf *)(chunk+1);		/* point to data past chunk header */
	switch(chunk->type) {
		case FLI_COLOR :
			if(see_colors) fii_reg_fcuncomp(cb1);
			fii_mem_fcuncomp(cb1,f->cmap);
			break;
		case FLI_LC:
			fii_unlccomp(cb1,f->p);
			break;
		case FLI_BLACK :
			i86_wzero(f->p,32000);
			break;
		case FLI_BRUN :
			fii_unbrun(cb1,f->p,f->h);
			break;
		case FLI_COPY :
			i86_wcopy(cb1,f->p,32000);
			break;
	}
	cbuf=i86_norm_ptr(cbuf+chunk->size);
}

}
