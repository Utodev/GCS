#include "jlib.h"

/* create a file to read/write */
Jfile dos_create(char *title)
{
union i86_regs reg;

reg.b.ah=0x3c;
reg.w.cx=0;
reg.w.dx=i86_ptr_offset(title);
reg.w.ds=i86_ptr_seg(title);

/* check carry */
if(i86_dosint(&reg,&reg) & 1) return(0);
else return(reg.w.ax);

}
