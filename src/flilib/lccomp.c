/* some C code that mixes with the assembler code in comp.asm and skip.asm */
/* to make up compressed pixel packets suitable for incorporation into a */
/* FLI file; see also writefli.c */
#include "aai86.h"
#include "aaflisav.h"
#include "aafii.h"

#define INERTIA 4

static char *sbrc_line(Pixel *s1, Pixel *s2, Pixel *cbuf, int count)
{
register int wcount;
int i, op_count, same_count, next_match, bcount;
register char *c;

op_count=0;
c=cbuf+1;
for(;;)	{
	if(count<=0) goto OUT;

	/* first find out how many bytes to skip... */
	wcount=i86_bcompare(s1,s2,count);
	if((count-=wcount)<=0) goto OUT;	/* same until the end... */
	/* if skip is longer than 255 have to break it up into smaller ops */
	while(wcount>255) {
		s1+=255+1;
		s2+=255;
		wcount-=255+1;
		/* make dummy copy 1 op */
		*c++=255;
		*c++=1;
		*c++=*s2++;
		op_count++;
	}
	/* save initial skip and move screen pointer to 1st different byte */
	*c++=wcount;
	s1+=wcount;
	s2+=wcount;
	op_count++;

	/* if have skipped to near the end do a literal copy... */
	if(count<=INERTIA) {
		*c++=count;
		i86_bcopy(s2,c,count);
		c+=count;
		goto OUT;
	}

	/* now look for a run of same... */
	bcount=count;
	if(bcount>FLI_MAX_RUN) bcount=FLI_MAX_RUN;

	wcount=i86_bsame(s2,bcount);
	/* it's worth doing a same thing thing */
	if(wcount>=INERTIA) {
		next_match=fii_tnskip(s1,s2,wcount,INERTIA);

		/* if it's in our space and a decent size */
		/* we'll cut short same run for the skip */
		if(next_match<wcount) wcount = next_match;

		*c++=-wcount;
		*c++=*s2;
		s1+=wcount;
		s2+=wcount;
		count-=wcount;
	}
	/* doing a literal copy; what can we do to make it short? */
	else {
		/* figure out how long until the next worthwhile "skip" */
		/* have wcount of stuff we can't skip through. */
		wcount=fii_tnsame(s2,fii_tnskip(s1,s2,bcount,INERTIA-1),
		  INERTIA);
		/* say copy positive count as lit copy op, and put bytes */
		/* to copy into the compression buffer */
		*c++=wcount;
		i86_bcopy(s2,c,wcount);
		s1+=wcount;
		s2+=wcount;
		c+=wcount;
		count-=wcount;
	}
}

OUT:
*cbuf=op_count;

return(i86_norm_ptr(c));
}

Cbuf *fii_lccomp(Pixel *s1, Pixel *s2, USHORT *cbuf, int width, int height)
{
int skip_count, lcount, j;
Pixel *c, *oc;
unsigned acc, last_real;
long total;

/* find out how many lines of s1 and s2 are the same */
acc=(width>>1);		/* SHORTS in line */
j=height;
skip_count=0;
total=0;

while(--j>=0) {
	if(i86_wcompare(s1,s2,acc)!=acc) break;
	s1+=width;
	s2+=width;
	skip_count++;
}

/* if all same do special case for empty frame */
if(skip_count==height) return((Cbuf *)(cbuf+1));

/* store offset of 1st real line and set up for main line-at-a-time loop */
*cbuf++=skip_count;
height-=skip_count;
c=(char *)(cbuf+1);
last_real=0;	/* keep track of last moving line */

for(j=1; j<=height; j++) {
	oc=c;

	/* if whole line is the same, set op count to 0 */
	if(i86_wcompare(s1,s2,acc)==acc) *c++=0;	
	else {
		c=sbrc_line(s1,s2,c,width);	/* compress line */
		last_real=j;
	}

	total+=i86_ptr_to_long(c)-i86_ptr_to_long(oc);
	if(total>=60000L) return(NULL);

	s1+=width;
	s2+=width;
}

/* set # of lines in compression to last real, removing empty bottom lines */
/* from buffer */
*cbuf=last_real;
c-=height-last_real;

return(i86_enorm_ptr(c));
}
