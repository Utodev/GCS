/**************************************
	Fichero de cabecera de
	CAPGRF.C
**************************************/

/*** Constantes ***/
#define VERSION		"1.1"

#define MIN_ANCHO       8	/* m¡nima anchura permitida */
#define MAX_ANCHO	640-16	/* m xima    "       "      */
#define MIN_ALTO        8	/* m¡nima altura permitida */
#define MAX_ALTO	350-16	/* m xima    "       "     */
#define MAX_GRF		100	/* m ximo n£mero de iconos */
#define MAX_TAMBLQ	65000U	/* m ximo tama¤o de bloque gr fico */
#define MEM_RESERVA	32000U	/* memoria de reserva */

/* c¢digos de 'scan' de teclado */
#define T_ESC		0x01    /* ESCAPE */
#define T_DER		0x4d	/* cursor derecha */
#define T_IZQ           0x4b	/* cursor izquierda */
#define T_ORG		0x47	/* origen */
#define T_FIN		0x4f	/* fin */
#define T_BORRAR	0x53	/* borrar */
#define T_F1		0x3b	/* F1 */
#define T_F2		0x3c	/* F2 */
#define T_F3		0x3d	/* F3 */

/*** Tipos de datos y estructuras ***/
typedef struct {
	unsigned long tam;	/* tama¤o en bytes */
	unsigned char *grf;	/* puntero a bloque gr fico */
} STC_GRF;

/*** Prototipos ***/
int int24_hnd(int errval, int ax, int bp, int si);
void beep(void);
void msg_error(char *msg);
int cuadro_siono(char *msg);
void cargar_pcx(void);
void restaura_pcx(void);
void captura_grf(void);
void libera_memgrf(void);
void dibuja_fondo_ver(void);
void ver_grf(void);
void grabar_grf(void);
void cargar_grf(void);
void borrar_grf(int ngrf);
