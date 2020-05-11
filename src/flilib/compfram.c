#include "aai86.h"
#include "aaflisav.h"
#include "aafii.h"

#define FLI_EMPTY_DCOMP 8  /* size returned by fii functions */
			   /* to indicate no change */

static Cbuf *full_cmap(Cbuf *cbuf, Cmap *cmap)
{

*cbuf++=1;
*cbuf++=0;
*cbuf++=0;
*cbuf++=0;
i86_bcopy(cmap,cbuf,AA_COLORS*3);

return(i86_norm_ptr(cbuf+AA_COLORS*3));
}

long fli_comp_frame(
  Cbuf *comp_buf,	/* buffer, should be FLI_CBUF_SIZE or bigger */
  Pixel *last_screen, Cmap *last_cmap, 	/* data from previous frame */
  Pixel *this_screen, Cmap *this_cmap,	/* data for this frame */
  int type)				/* FLI_BRUN?  FLI_LCCOMP? */
{
Cbuf *c;
struct fli_frame *frame;
struct fli_chunk *chunk;

frame=(struct fli_frame *)comp_buf;
i86_wzero(frame,sizeof(*frame)/sizeof(SHORT));
chunk=(struct fli_chunk *)(frame+1);

/* 1st make the color map chunk */
if(type==FLI_BRUN) c=full_cmap((char *)(chunk+1),this_cmap);
else c=fii_fccomp(last_cmap,this_cmap,(USHORT *)(chunk+1),AA_COLORS);

chunk->type=FLI_COLOR;
chunk->size=i86_ptr_to_long(c)-i86_ptr_to_long(chunk);

if(chunk->size==FLI_EMPTY_DCOMP) c=(char *)chunk;
else frame->chunks=1;
chunk=(struct fli_chunk *)c;

switch(type) {
	case FLI_LC :
		c=fii_lccomp(last_screen,this_screen,(USHORT *)(chunk+1),
		  320,200);
		break;
	case FLI_BRUN :
		c=fii_brun(this_screen,(USHORT *)(chunk+1),320,200);
		break;
}

if(c==NULL) {
	chunk->size=64000L+sizeof(chunk);
	chunk->type=FLI_COPY;
	c=i86_norm_ptr((char *)(chunk+1)+64000L);
	i86_wcopy(this_screen,chunk+1,32000);
}
else {
	chunk->type=type;
	chunk->size=i86_ptr_to_long(c)-i86_ptr_to_long(chunk);
}

if(chunk->size==FLI_EMPTY_DCOMP) c=(char *)chunk;
else frame->chunks++;
frame->type=FLIF_MAGIC;
frame->size=i86_ptr_to_long(c)-i86_ptr_to_long(comp_buf);

return(frame->size);
}
