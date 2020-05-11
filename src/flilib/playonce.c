#include "aafli.h"

Boolean fli_until_finished(int cur_frame, int frame_count, int cur_loop)
{

return(cur_frame<frame_count);
}

/* play fli once */
Errval fli_once(char *fliname)
{

return(fli_until(fliname,-1,fli_until_finished));
}
