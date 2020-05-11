#include <alloc.h>

void *aa_malloc(unsigned size)
{

return(malloc(size));
}

void aa_free(void *pt)
{

free(pt);
}
