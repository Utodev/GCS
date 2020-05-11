#include "aai86.h"
#include "aados.h"
#include "aaflisav.h"

static Errval write_a_fframe(Jfile ff,	/* FLI file returned by fli_create */
  Fli_head *fh, 		 	/* header inited by fli_create */
  Vscreen *this, Vscreen *last,		/* current and previous frame */
  int compress_type, 	     		/* FLI_BRUN, FLI_LC, etc. */
  int frame_counts)	     		/* 0 on ring frame, otherwise 1 */
{
Cbuf *cbuf;
long fsize;
Pixel *lpixels, *lcmap;
Errval err=AA_SUCCESS;

if((cbuf=aa_malloc(FLI_CBUF_SIZE))==NULL) return(AA_ERR_NOMEM);

if(last==NULL) {
	lpixels=NULL;
	lcmap=NULL;
}
else {
	lpixels=last->p;
	lcmap=last->cmap;
}

fsize=fli_comp_frame(cbuf,lpixels,lcmap,this->p,this->cmap,compress_type);
if(dos_write(ff,cbuf,fsize)!=fsize) err=AA_ERR_SHORTWRITE;

aa_free(cbuf);
fh->size+=fsize;
fh->frame_count+=frame_counts;

return(err);
}

Errval fli_write_next(Jfile ff,	/* FLI file returned by fli_create */
  Fli_head *fh, 		/* same header used by fli_create */
  Vscreen *this, 		/* current frame */
  Vscreen *last)		/* previous frame */
{

return(write_a_fframe(ff,fh,this,last,
  (fh->frame_count==0 ? FLI_BRUN : FLI_LC),1));
}

/* write the 'ring frame', that is the difference between the first and */
/* last frame of a fli; pass in the final frame of the FLI in last_frame; */
/* firstf_buf will be loaded with the first frame of the FLI as a side effect */
Errval fli_end(Jfile ff, Fli_head *fh, Vscreen *end_frame, Vscreen *firstf_buf)
{
long lastpos;
Errval err;

lastpos=dos_tell(ff);
if(dos_seek(ff,(long)sizeof(*fh),DOS_SEEK_START)<AA_SUCCESS) 
  return(AA_ERR_SEEK);
err=fli_read_display_frame(ff,firstf_buf,FALSE);
if(err<AA_SUCCESS) return(err);

if(dos_seek(ff,lastpos,DOS_SEEK_START)<AA_SUCCESS) return(AA_ERR_SEEK);
err=write_a_fframe(ff,fh,firstf_buf,end_frame,FLI_LC,0);
if(err<AA_SUCCESS) return(err);

if(dos_seek(ff,0L,DOS_SEEK_START)<AA_SUCCESS) return(AA_ERR_SEEK);
fh->flags=(FLI_FINISHED | FLI_LOOPED);
if(dos_write(ff,fh,(long)sizeof(*fh))!=sizeof(*fh)) return(AA_ERR_SHORTWRITE);

return(AA_SUCCESS);
}

Jfile fli_create(char *fliname, Fli_head *fh, int speed)
{
Jfile ff;
Errval err;

if((ff=dos_create(fliname))==0)	return(AA_ERR_CANTMAKE);

i86_bzero(fh,sizeof(*fh));	/* zero out counts and so forth */
fh->type=FLIH_MAGIC;
fh->size=sizeof(*fh);
fh->width=320;
fh->height=200;
fh->bits_a_pixel=8;
fh->speed=speed;
if(dos_write(ff,fh,(long)sizeof(*fh))!=sizeof(*fh)) {
	dos_close(ff);
	return(AA_ERR_SHORTWRITE);
}

return(ff);
}
