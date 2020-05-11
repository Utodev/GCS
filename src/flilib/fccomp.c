#include "aai86.h"
#include "aaflisav.h"
#include "aafii.h"

/* compress an rgb triples color map just doing 'skip' compression */
Cbuf *fii_fccomp(Cmap *s1, Cmap *s2, USHORT *cbuf, int count)
{
USHORT wcount, i;
Cbuf *c;
USHORT op_count, dif_count, same_count, next_match, bcount;
Cmap *s2x;
USHORT c3;

c=(Cbuf *)(cbuf+1);
op_count=0;
count*=3;
wcount=i86_wcompare(s1,s2,count>>1);
wcount <<= 1;
if(wcount==count) return(c);	/* stupid way to say got nothing... */

for(;;)	{
	/* first find out how many words to skip... */
	c3=(i86_bcompare(s1,s2,count)/3);
	wcount=c3*3;
	if((count-=wcount)==0) goto OUT;	/* same until the end... */
	*c++=c3;
	s1+=wcount;
	s2+=wcount;
	op_count++;

	/* figure out how long until the next worthwhile "skip" */
	dif_count=0;
	bcount=count;
	for(;;)	{
		wcount=i86_bcontrast(s1,s2,bcount)/3;
		dif_count+=wcount;
		wcount*=3;
		s1+=wcount;
		s2+=wcount;
		bcount-=wcount;
		if(bcount>=3) {
			if((wcount=i86_bcompare(s1,s2,3))==3) break;
			else {
				dif_count+=1;
				s1+=3;
				s2+=3;
				bcount-=3;
			}
		}
		else break;
	}
	*c++=dif_count;
	dif_count*=3;
	s2-=dif_count;
	count-=dif_count;
	for(;;)	{
		if(dif_count==0) break;
		dif_count-=1;
		*c++=*s2++;
	}
	if(count<=0) break;
}

OUT:
*cbuf=op_count;

return(i86_enorm_ptr(c));
}
