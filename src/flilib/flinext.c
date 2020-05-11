#include "aados.h"
#include "aafli.h"

Errval fli_read_display_frame(Jfile ff, Vscreen *v, Boolean see_colors)
{
Fli_frame fframe;
Cbuf *cbuf;

if(dos_read(ff,&fframe,(long)sizeof(fframe))!=sizeof(fframe))
  return(AA_ERR_SHORTREAD);
if(fframe.type!=FLIF_MAGIC) return(AA_ERR_BADDATA);

/* may be an empty frame */
if((fframe.size-=sizeof(fframe))!=0) {
	if((cbuf=aa_malloc((unsigned)fframe.size))==NULL) return(AA_ERR_NOMEM);
	if(dos_read(ff,cbuf,fframe.size)!=fframe.size) {
		aa_free(cbuf);
		return(AA_ERR_SHORTREAD);
	}
	fli_uncomp(v,&fframe,cbuf,see_colors);
	aa_free(cbuf);
}

return(AA_SUCCESS);
}

Errval fli_next_frame(Jfile ff)
{

return(fli_read_display_frame(ff,&aa_screen,TRUE));
}

