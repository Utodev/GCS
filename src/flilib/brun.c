#include "aai86.h"
#include "aaflisav.h"
#include "aafii.h"

static char *fii_brun_comp_line(Pixel *s1, Pixel *cbuf, int count)
{
int wcount, op_count, dif_count, same_count;
register char *c;
register int bcount;
char *start_dif;

c=cbuf+1;
op_count=0;
start_dif=s1;
dif_count=0;
for(;;)	{
	if(count<3) {
		dif_count+=count;
		while(dif_count>0) {
			bcount=(dif_count<FLI_MAX_RUN ? dif_count : 
			  FLI_MAX_RUN);
			*c++=-bcount;
			dif_count-=bcount;
			while(--bcount>=0) *c++=*start_dif++;
			op_count++;
		}
		*cbuf=op_count;
		return(i86_norm_ptr(c));
	}
	else {
		bcount=(count<FLI_MAX_RUN ? count : FLI_MAX_RUN );
		if((wcount=i86_bsame(s1,bcount))>=3) {
			while(dif_count>0) {
				bcount=(dif_count<FLI_MAX_RUN ? dif_count : 
				  FLI_MAX_RUN);
				*c++=-bcount;
				dif_count-=bcount;
				while(--bcount>=0) *c++=*start_dif++;
				op_count++;
			}
			*c++=wcount;
			*c++=*s1;
			op_count++;
			s1+=wcount;
			count-=wcount;
			start_dif=s1;
		}
		else {
			dif_count++;
			s1++;
			count-=1;
		}
	}
}

}

Cbuf *fii_brun(Pixel *s1, USHORT *cbuf, int width, int height)
{
register char *c;
char *oc;
long total = 0;

/* store offset of 1st real line and set up for main line-at-a-time loop */
c=(char *)(cbuf);
while(--height>=0) {
	oc=c;
	c=fii_brun_comp_line(s1,c,width);
	total+=i86_ptr_to_long(c)-i86_ptr_to_long(oc);
	if(total>=60000L) return(NULL);
	s1+=width;
}

return(i86_enorm_ptr(c));
}
