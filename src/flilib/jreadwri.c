#include "jlib.h"

/* read or write a buffer from file; usually you use dos_read() or */
/* dos_write() macros rather than calling this one directly */
long dos_rw(Jfile f, void *buf, long size, int ah)
{
union i86_regs reg;
long written;
unsigned s1;

written=0;
while(size>0) {
	reg.b.ah=ah;
	reg.w.bx=f;
	s1=(size>0x0c000L ?  0xc000 : size);
	reg.w.cx=s1;
	reg.w.dx=i86_ptr_offset(buf);
	reg.w.ds=i86_ptr_seg(buf);

	/* check carry */
	if((i86_dosint(&reg,&reg)) & 1) goto OUT;
	else {
		written+=(unsigned)reg.w.ax;
		size-=(unsigned)reg.w.ax;
		if(s1!=reg.w.ax) goto OUT;
	}
	buf=i86_norm_ptr((char *)buf+s1);
}

OUT:
return(written);
}
