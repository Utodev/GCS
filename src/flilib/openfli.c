#include "aafli.h"

Jfile fli_open(char *fliname, Fli_head *fh)
{
Jfile ff;

if((ff=dos_open(fliname,DOS_READ_ONLY))==0) return(AA_ERR_CANTFIND);

if(dos_read(ff,fh,(long)sizeof(*fh))==sizeof(*fh)) {
	if(fh->type==FLIH_MAGIC) return(ff);
}

dos_close(ff);

return(AA_ERR_BADTYPE);
}
