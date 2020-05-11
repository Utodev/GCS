/**************************************
	Fichero de cabecera principal
	de LKS.C
**************************************/

/*** Constantes ***/
#define ERR_ANCHO       42      /* anchura cuadro de errores */
#define ERR_ALTO        8       /* altura    "    "    "     */

/*** Prototipos ***/
int int24_hnd(int errval, int ax, int bp, int si);
void lee_cfg(char *argv0);
void esconde_cursor(void);
int analiza_args(int argc, char *argv[]);
void construye_nfexe(void);
void imprime_error(char *msg);
unsigned long max_mem(void);
void copia_fichero(int horg, int hdest);
void linkar(void);
