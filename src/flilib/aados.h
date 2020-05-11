#ifndef AADOS_H
#define AADOS_H

#ifndef AATYPES_H
#include "aatypes.h"
#endif /* AATYPES_H */

typedef int Jfile;

Jfile dos_open(char *title, int mode); /* open a file that already exists.*/
/* defines for mode parameter to dos_open  */
#define DOS_READ_ONLY 0
#define DOS_WRITE_ONLY 1
#define DOS_READ_WRITE 2

Jfile dos_create(char *title);	/* create a new read/write file */
void dos_close(Jfile f);   	/* close file */

/* file read/write, normally don't use dos_rw, but go through macros */
long dos_rw(Jfile f, void *buf, long size, int ah);
#define dos_read(f,b,size) dos_rw(f,b,size,0x3f)
#define dos_write(f,b,size) dos_rw(f,b,size,0x40)

long dos_seek (Jfile f, long offset, int mode);
long dos_tell (Jfile f);
/* defines for mode parameter to dos_seek */
#define DOS_SEEK_START		0
#define DOS_SEEK_RELATIVE	1
#define DOS_SEEK_END		2

#endif /* AADOS_H */
