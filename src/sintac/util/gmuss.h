/**************************************
	Fichero de cabecera principal
	de MUSS.C
**************************************/

/*** Constantes ***/
#define GMUSS_VERSION	"1.0"
#define GMUSS_ANCHO	80
#define GMUSS_ALTO	25

#define MAX_NOTAS	2000

/*** Prototipos ***/
int int24_hnd(int errval, int ax, int bp, int si);
int analiza_args(int argc, char *argv[]);
void activa_vga(void);
void lee_cfg(char *argv0);
void esconde_cursor(void);
void imprime_error(char *msg);
int pregunta_siono(char *preg);
void elige_nota(STC_SND *snd);
void reproduce(int nota_i, int nota_f);
