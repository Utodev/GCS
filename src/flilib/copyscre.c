#include "aascreen.h"

void aa_copy_screen(Vscreen *s, Vscreen *d)
{

i86_wcopy(s->p,d->p,d->psize>>1);
i86_wcopy(s->cmap,d->cmap,(AA_COLORS*3)>>1);

}

void aa_clear_screen(Vscreen *vs)
{

i86_wzero(vs->p,vs->psize>>1);

}
