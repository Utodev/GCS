#include "aascreen.h"

Cmap aa_colors[AA_COLORS*3]={
	/* 32 level grey scale */
 	0, 0, 0,	 2, 2, 2,	 4, 4, 4,	 6, 6, 6,
 	8, 8, 8,	10,10,10,	12,12,12,	14,14,14,
	16,16,16,	18,18,18,	20,20,20,	22,22,22,
	24,24,24,	26,26,26,	28,28,28,	30,30,30,
	33,33,33,	35,35,35,	37,37,37,	39,39,39,
	41,41,41,	43,43,43,	45,45,45,	47,47,47,
	49,49,49,	51,51,51,	53,53,53,	55,55,55,
	57,57,57,	59,59,59,	61,61,61,	63,63,63,
	/* a hand threaded 6x6x6 rgb cube */
	63,51,51,	63,63,51,	51,63,51,	51,63,63,
	51,51,63,	63,51,63,	63,39,39,	63,51,39,
	63,63,39,	51,63,39,	39,63,39,	39,63,51,
	39,63,63,	39,51,63,	39,39,63,	51,39,63,
	63,39,63,	63,39,51,	63,27,27,	63,39,27,
	63,51,27,	63,63,27,	51,63,27,	39,63,27,
	27,63,27,	27,63,39,	27,63,51,	27,63,63,
	27,51,63,	27,39,63,	27,27,63,	39,27,63,
	51,27,63,	63,27,63,	63,27,51,	63,27,39,
	63,15,15,	63,27,15,	63,39,15,	63,51,15,
	63,63,15,	51,63,15,	39,63,15,	27,63,15,
	15,63,15,	15,63,27,	15,63,39,	15,63,51,
	15,63,63,	15,51,63,	15,39,63,	15,27,63,
	15,15,63,	27,15,63,	39,15,63,	51,15,63,
	63,15,63,	63,15,51,	63,15,39,	63,15,27,
	63, 3,15,	63, 3, 3,	63,15, 3,	63,27, 3,
	63,39, 3,	63,51, 3,	63,63, 3,	51,63, 3,
	39,63, 3,	27,63, 3,	15,63, 3,	 3,63, 3,
 	3,63,15,	 3,63,27,	 3,63,39,	 3,63,51,
 	3,63,63,	 3,51,63,	 3,39,63,	 3,27,63,
 	3,15,63,	 3, 3,63,	15, 3,63,	27, 3,63,
	39, 3,63,	51, 3,63,	63, 3,63,	63, 3,51,
	63, 3,39,	63, 3,27,	51, 3,15,	51, 3, 3,
	51,15, 3,	51,27, 3,	51,39, 3,	51,51, 3,
	39,51, 3,	27,51, 3,	15,51, 3,	 3,51, 3,
 	3,51,15,	 3,51,27,	 3,51,39,	 3,51,51,
 	3,39,51,	 3,27,51,	 3,15,51,	 3, 3,51,
	15, 3,51,	27, 3,51,	39, 3,51,	51, 3,51,
	51, 3,39,	51, 3,27,	39, 3,15,	39, 3, 3,
	39,15, 3,	39,27, 3,	39,39, 3,	27,39, 3,
	15,39, 3,	 3,39, 3,	 3,39,15,	 3,39,27,
 	3,39,39,	 3,27,39,	 3,15,39,	 3, 3,39,
	15, 3,39,	27, 3,39,	39, 3,39,	39, 3,27,
	27, 3,15,	27, 3, 3,	27,15, 3,	27,27, 3,
	15,27, 3,	 3,27, 3,	 3,27,15,	 3,27,27,
 	3,15,27,	 3, 3,27,	15, 3,27,	27, 3,27,
	15, 3, 3,	15,15, 3,	 3,15, 3,	 3,15,15,
 	3, 3,15,	15, 3,15,	27,15,15,	27,27,15,
	15,27,15,	15,27,27,	15,15,27,	27,15,27,
	39,15,15,	39,27,15,	39,39,15,	27,39,15,
	15,39,15,	15,39,27,	15,39,39,	15,27,39,
	15,15,39,	27,15,39,	39,15,39,	39,15,27,
	51,15,15,	51,27,15,	51,39,15,	51,51,15,
	39,51,15,	27,51,15,	15,51,15,	15,51,27,
	15,51,39,	15,51,51,	15,39,51,	15,27,51,
	15,15,51,	27,15,51,	39,15,51,	51,15,51,
	51,15,39,	51,15,27,	51,27,27,	51,39,27,
	51,51,27,	39,51,27,	27,51,27,	27,51,39,
	27,51,51,	27,39,51,	27,27,51,	39,27,51,
	51,27,51,	51,27,39,	51,39,39,	51,51,39,
	39,51,39,	39,51,51,	39,39,51,	51,39,51,
	39,27,27,	39,39,27,	27,39,27,	27,39,39,
	27,27,39,	39,27,39,	 3, 3, 3,	15,15,15,
	27,27,27,	39,39,39,	51,51,51,	63,63,63,
 	/* 3 empty slots */
	63,22, 3,	39, 7, 5,	36,36,63,
 	/* colors the system would like to use for interface */
	 0, 0, 0,       22,22,22,	38,38,38,	52,52,52,	
	63, 0, 0,
};

Vscreen aa_screen={
	0,0,AA_XMAX,AA_YMAX,AA_XMAX,
	AA_VGA_SCREEN,
	aa_colors,
	(long)AA_XMAX*(long)AA_YMAX,
	NULL
};