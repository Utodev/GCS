#include "jlib.h"

/* return current file position  */
long dos_tell(Jfile f)
{

return(dos_seek(f,0L,1));
}
