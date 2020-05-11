#include "aafli.h"
#include "aaclock.h"

Errval fli_until(char *fliname,	/* name of fli to play */
  int speed,
  AAivec until)			/* function to call to see when to stop */
{
Jfile ff;
Fli_head fhead;
long frame1off, time, last_time, dest_time;
int err, loop=0, i, cur_frame;

aa_goclock();
if((ff=fli_open(fliname,&fhead))<0) return((Errval)ff);

cur_frame=0;
if(!(*until)(cur_frame,(int)fhead.frame_count,loop)) goto OUT;
if((err=fli_next_frame(ff))>=AA_SUCCESS) frame1off=dos_tell(ff);

last_time=aa_getclock();
if(speed<0) speed=fhead.speed;
speed *= AA_CLOCK_SCALE;

for(;;)	{
	dos_seek(ff,frame1off,DOS_SEEK_START);
	for(i=0; i<fhead.frame_count; i++) {
		cur_frame++;
		dest_time=last_time+speed;
		/* busy busy busy wait until it's time... */
		for(;;)	{
			if(!(*until)(cur_frame,(int)fhead.frame_count,loop))
			  goto OUT;
			time=aa_getclock();
			if(time>=dest_time) {
				last_time=time;
				break;
			}
		}
		if((err=fli_next_frame(ff))<AA_SUCCESS)	goto OUT;
	}

	loop++;
}

OUT:
dos_close (ff);

return(err);
}
