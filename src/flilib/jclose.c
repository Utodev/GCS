#include "jlib.h"

/* close file */
void dos_close(Jfile f)
{
union i86_regs reg;

reg.b.ah=0x3e;
reg.w.bx=f;
i86_dosint(&reg,&reg);

}
