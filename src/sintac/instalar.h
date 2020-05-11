/**************************************
	Fichero de cabecera principal
	de INSTALAR.C
**************************************/

/*** Constantes ***/
#define MENU_FIL        5       /* fila men£ principal */
#define MENU_COL        21      /* columna "     "     */
#define CDIR_ANCHO      68      /* anchura cuadro directorios */
#define CDIR_ALTO       11      /* altura cuadro directorios */
#define NUM_FICH_INST   100     /* m ximo n£mero de ficheros para instalar */
#define LNG_NOMBRE      41      /* longitud m xima de nombre de fichero + 1 */
#define LNG_SUBDIR	41	/* longitud m xima de subdirectorio + 1 */

/*** Tipos de datos y estructuras ***/
typedef struct {
	char nombre[LNG_NOMBRE];        /* nombre del fichero */
	int tipo;                       /* tipo de fichero */
	int disco;                      /* n§ disco de instalaci¢n */
	char subdir[LNG_SUBDIR];	/* subdirectorio */
} STC_DATFICH;

/*** Prototipos ***/
int int24_hnd(int errval, int ax, int bp, int si);
void inicializa_cfg(void);
void esconde_cursor(void);
void ajusta_ndir(char *dir);
void directorios(void);
void error(char *msg);
void lee_datos_fich(char *nf_datfich);
unsigned long max_mem(void);
int copia(char *origen, char *destino);
void beep(void);
void instalar(void);
