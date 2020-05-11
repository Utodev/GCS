#include "aai86.h"
#include "aados.h"

/* reposition file pointer */
long dos_seek(Jfile f, long offset, int mode)
{
union i86_regs reg;

reg.b.ah=0x42;
reg.b.al=mode;
reg.w.bx=f;
reg.w.cx=i86_ptr_seg((void *)offset);
reg.w.dx=i86_ptr_offset((void *)offset);

/* check carry */
if(i86_dosint(&reg,&reg) & 1) return(-1);
else return(i86_make_long(reg.w.ax, reg.w.dx));

}
