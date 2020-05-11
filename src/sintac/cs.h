/**************************************
	Fichero de cabecera principal
 	de CS.C
**************************************/

/*** Constantes ***/
#define NF_CSERR	"CS.ERR"	/* fichero con mensajes de error */
#define CCOMP_FIL       5       /* fila cuadro principal de compilador */
#define CCOMP_COL       2       /* columna "       "     "      "      */
#define CCOMP_ANCHO     76      /* anchura "       "     "      "      */
#define CCOMP_ALTO      14      /* altura  "       "     "      "      */
#define CERR_FIL        5       /* fila cuadro de errores */
#define CERR_COL        0       /* columna "       "     "      "      */
#define CERR_ANCHO      80      /* anchura "       "     "      "      */
#define CERR_ALTO       14      /* altura  "       "     "      "      */
#define FIL_SECCS       5       /* fila para imprimir nombres de secciones */
#define COL_SECCS       15      /* columna "    "        "    "      "     */
#define MARCA_1		"<"	/* marca para generaci¢n de ficheros temp. */
#define MARCA_2		"<-"	/* marca para proceso de compilaci¢n */

/* constantes para tipo_lin() */
#define L_COMENT	0
#define L_CONST         1
#define L_MARCA         2
#define L_NULA          3
#define L_NORM          4

/*** Prototipos ***/
int int24_hnd(int errval, int ax, int bp, int si);
int c_break(void);
void fin_prg(int codigo);
void men_info(char *men);
char *men_error(int cod_err);
void imp_error(unsigned cod_err, unsigned long nlin);
size_t reserva_memoria(void);
void crea_ficheros_temp(void);
int analiza_args(int argc, char *argv[]);
void construye_nfsal(void);
char *lee_linea(FILE *f_ent, char *lin, unsigned long *nlin);
int tipo_lin(char *lin);
COD_ERR chequea_conx(BYTE num_loc, BYTE *conx, unsigned *desp_conx,
  BYTE *err_conx, int *num_conx);
void codifica(BYTE *mem, unsigned bytes_mem);
void lee_cfg(char *argv0);
void esconde_cursor(void);
void marca_seccion(int secc, char *marca);
void compila_seccs(size_t memoria);

