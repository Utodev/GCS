#include <bios.h>
#include "aafli.h"

Boolean fli_until_key(int cur_frame, int frame_count, int cur_loop)
{

if(bioskey(1)) {
	bioskey(0);
	return(FALSE);
}
else return(TRUE);

}

/* play fli looping forever until any key is hit */
Errval fli_play(char *fliname)
{

return(fli_until(fliname,-1,fli_until_key));
}
