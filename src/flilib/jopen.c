#include "jlib.h"

/* open a file that already exists */
Jfile dos_open(char *title, int mode)
{
union i86_regs reg;

reg.b.ah=0x3d;		/* open file */
reg.b.al=mode;		/* read/write etc... */
reg.w.dx=i86_ptr_offset(title);
reg.w.ds=i86_ptr_seg(title);

/* check carry */
if(i86_dosint(&reg,&reg) & 1) return(0);
else return(reg.w.ax);

}
