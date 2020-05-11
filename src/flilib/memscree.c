#include "aai86.h"
#include "aascreen.h"

Vscreen *aa_alloc_mem_cel(int x, int y, int w, int h)
{
Vscreen *vs;

if((vs=aa_malloc(sizeof(*vs)))!=NULL) {
	i86_bzero(vs,sizeof(*vs));
	vs->x=x;
	vs->y=y;
	vs->bpr=vs->w=w;
	vs->h=h;
	vs->psize=(long)w*h;
	if((vs->allocedp=aa_malloc((unsigned)vs->psize+16))==NULL) {
		aa_free(vs);
		return(NULL);
	}
	if((vs->cmap=aa_malloc(AA_COLORS*3))==NULL) {
		aa_free_mem_screen(vs);
		return(NULL);
	}
	/* force even paragraph allignment */
	vs->p=i86_make_ptr(0,i86_ptr_seg(vs->allocedp)+1);
}

return(vs);
}

Vscreen *aa_alloc_mem_screen(void)
{

return(aa_alloc_mem_cel(0,0,AA_XMAX,AA_YMAX));
}

void aa_free_mem_screen(Vscreen *vs)
{

if(vs) {
	if(vs->cmap) aa_free(vs->cmap);
	if(vs->allocedp) aa_free(vs->allocedp);
	aa_free(vs);
}

}
