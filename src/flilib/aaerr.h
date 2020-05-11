#ifndef AAERR_H
#define AAERR_H

#define AA_SUCCESS		0	/* no Problem! */
#define AA_ERR_MISC		-1	/* unclassified error */
#define AA_ERR_NOMEM		-2	/* not enough memory */
#define AA_ERR_CANTFIND 	-3	/* file doesn't exist */
#define AA_ERR_CANTMAKE 	-4	/* can't create file */
#define AA_ERR_BADTYPE 		-5	/* file is not the right type */
#define AA_ERR_BADDATA 		-6	/* file damaged internally */
#define AA_ERR_WRONGREZ 	-7	/* file isn't 320x200 */
#define AA_ERR_SHORTREAD 	-8 	/* not all of file is there */
#define AA_ERR_SHORTWRITE 	-9	/* couldn't write all of file */
#define AA_ERR_SEEK		-10	/* error during file seek */

typedef int Errval;
char *fli_error_message(Errval err);	/* return error message */

#endif /* AAERR_H */
