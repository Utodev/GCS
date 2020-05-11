#ifndef AACLOCK_H
#define AACLOCK_H

void aa_goclock(void);	/* initialize clock */
long aa_getclock(void);	/* unfiltered 4608 hz clock; will sometimes go */
		      	/* briefly backwards, this is ok for FLI file playing */

#define AA_CLOCK_HZ	4608  	/* frequency of clock */
#define AA_MONITOR_HZ 	70	/* frequency of Monitor */
#define AA_CLOCK_SCALE 	(AA_CLOCK_HZ/AA_MONITOR_HZ)

#endif /* AACLOCK_H */
