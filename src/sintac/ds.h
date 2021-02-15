/**************************************
	Fichero de cabecera principal
	de DS.C
**************************************/

/*** Constantes ***/
#define WDEB_FIL    0           /* fila ventana debugger */
#define WDEB_COL    0           /* columna ventana debugger */
#define WDEB_ANCHO  40          /* anchura ventana debugger */
#define WDEB_ALTO   4           /* altura ventana debugger */
#define WDEB_COLORF 0           /* color fondo ventana debugger */
#define WDEB_COLOR  15          /* color primer plano ventana debugger */
#define CUR_DEBUG   (char)'Û'   /* cursor para ventana debugger */
#define LNG_LINDEB  40          /* m x. longitud l¡nea debugger */

#define WERR_FIL    10          /* fila ventana de errores */
#define WERR_COL    0           /* columna ventana de errores */
#define WERR_ANCHO  40          /* anchura ventana de errores */
#define WERR_ALTO   3           /* altura ventana de errores */
#define WERR_COLORF 4           /* color fondo ventana de errores */
#define WERR_COLOR  14          /* color primer plano ventana de errores */

#define WJSJ_FIL    7           /* fila ventana presentaci¢n */
#define WJSJ_COL    0           /* columna ventana presentaci¢n */
#define WJSJ_ANCHO  40          /* anchura ventana presentaci¢n */
#define WJSJ_ALTO   12          /* altura ventana presentaci¢n */

/*** Prototipos ***/
int int24_hnd(int errval, int ax, int bp, int si);

#if DEBUGGER==1
int saca_pal(BYTE num_pal,BYTE tipo_pal);
void imp_condacto(BYTE indir, BYTE npar, BYTE *pro_d);
BYTE inp_deb(void);
void imp_flag(BYTE flag);
void guarda_debugger(void);
void recupera_debugger(void);
void imp_debugger(BYTE indir, BYTE npar, BYTE *pro_d, BYTE flag, char *txt_deb);
void debugger(BYTE indir, BYTE npar, BYTE *pro_d);
#endif

void modo_video(int modo);
void carga_bd(char *nombre);
void codifica(BYTE *mem, unsigned bytes_mem);
void inic(void);
void frd(FILE *fbd, void *buff, size_t tam, size_t cant);
char mayuscula(char c);
void m_err(BYTE x, char *m, int flag);
void actualiza_raton(void);