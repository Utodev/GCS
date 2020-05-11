/**************************************
	Fichero de cabecera de
	las funciones de manejo
	de cuadros de botones
	de BOTONG.C
**************************************/

#if !defined (BOTONG_H)
#define BOTONG_H

/*** Constantes ***/
#define BG_MAXBOT	25	/* m ximo n£mero de botones en un cuadro */
#define BG_CENT         9999    /* valor para centrar el cuadro */

/*** Tipos de datos y estructuras ***/
typedef struct {
	unsigned char *icn;	/* puntero a gr fico del icono (NULL ninguno) */
	int x, y;		/* coordenadas relativas al origen del bot¢n */
} STC_ICONO;

typedef struct {
	int fil, col;		/* posici¢n del bot¢n en ventana */
	int ancho, alto;	/* dimensiones del bot¢n */
	STC_ICONO icono;	/* datos del icono */
} STC_BOTONG;

typedef struct {
	STC_VENTANAG v;		/* ventana del cuadro de botones */
	int nbot_horz;  	/* n£mero de botones en horizontal */
	int nbot_vert;		/*   "    "     "    "  vertical   */
	STC_BOTONG bot[BG_MAXBOT];	/* datos de botones */
} STC_CUAD_BOTONG;

/*** Prototipos ***/
void bg_crea(STC_CUAD_BOTONG *cbot, int fil, int col, int nbot_horz,
  int nbot_vert, int ancho_bot, int alto_bot, BYTE clr_fondo, BYTE clr_pplano,
  BYTE clr_s1, BYTE clr_s2, char *titulo, STC_ICONO dat_icn[]);
void bg_dibuja(STC_CUAD_BOTONG *cbot);
void bg_abre(STC_CUAD_BOTONG *cbot);
void bg_cierra(STC_CUAD_BOTONG *cbot);
int bg_pulsado(STC_CUAD_BOTONG *cbot, int x, int y);

#endif  /* BOTONG_H */
