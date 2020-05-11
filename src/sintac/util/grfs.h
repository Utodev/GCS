/**************************************
	Fichero de cabecera principal
	de GRFS.C
**************************************/

/*** Constantes ***/
#define GRFS_VERSION	"1.0"
#define GRFS_ANCHO	80
#define GRFS_ALTO       12

#define ERR_NOEXGRF	1	/* no existe fichero GRF */
#define ERR_GRFNOV	2	/* fichero GRF no v lido */
#define ERR_APERPCX	3	/* error de apertura fichero PCX */
#define ERR_LECTPCX	4	/* error de lectura fichero PCX */
#define ERR_PCXNOV	5	/* fichero PCX no v lido */
#define ERR_LECTGRF	6	/* error de lectura fichero GRF */
#define ERR_ESCRGRF	7	/* error de escritura fichero GRF */
#define ERR_IMGREP	8	/* n£mero de imagen repetido */
#define ERR_NOEXIMG	9	/* no existe esa imagen en fichero */
#define ERR_FTEMP	10	/* error en fichero temporal */

/*** Prototipos ***/
int int24_hnd(int errval, int ax, int bp, int si);
int analiza_args(int argc, char *argv[]);
void activa_vga(void);
void lee_cfg(char *argv0);
void esconde_cursor(void);
void pausa(void);
void imprime_error(char *msg);
int pregunta_siono(char *preg);
unsigned long max_mem(void);
int pon_imgpcx(char *nf_grf, char *nf_pcx, int n_img);
int borra_imgpcx(char *nf_grf, int n_img);
