/**************************************
	Fichero de cabecera de
	las funciones de manejo
	de ventanas de VENTANAG.C
**************************************/

#if !defined (VENTANAG_H)
#define VENTANAG_H

/*** Constantes ***/
#define VG_LINEA_LINEA	0       /* imprimir l¡nea a l¡nea */
#define VG_PASA_LINEA   1       /* pasa de una l¡nea a otra */
#define VG_NORELLENA    0       /* no rellena con espacios imprimir cadena */
#define VG_RELLENA      1       /* rellena con espacios al imprimir cadena */

/* tipos de borde para ventanas */
#define VG_BORDE0	0	/* borde normal */
#define VG_BORDE1	1	/* borde normal+borde interior */

/*** Tipos de datos y estructuras ***/
#if !defined (BYTE)
#define BYTE unsigned char
#endif

typedef struct {
	int fil, col;         	/* posici¢n de la ventana en pantalla */
	int ancho, alto;      	/* dimensiones de la ventana */
	BYTE clr_fondo;         /* color de fondo de la ventana */
	BYTE clr_pplano;	/* color de primer plano de la ventana */
	BYTE clr_s1;            /* color para sombra 1 */
	BYTE clr_s2;           	/* color para sombra 2 */
	int borde;		/* tipo de borde */
	char *titulo;           /* puntero a cadena con texto encabezamiento */
	BYTE far *fondo;        /* puntero a buffer donde guardar fondo */
	int modo_texto;         /* modo de impresi¢n de texto */
	int filc, colc;       	/* posici¢n de impresi¢n dentro de ventana */
	BYTE clr_textof;       	/* color de fondo del texto de ventana */
	BYTE clr_textop;	/* color de primer plano del texto de ventana */
} STC_VENTANAG;

/*** Prototipos ***/
void vg_impcar(int fil, int col, BYTE car, BYTE colorf, BYTE color);
void vg_crea(STC_VENTANAG *ventana, int fil, int col, int ancho,
  int alto, BYTE clr_fondo, BYTE clr_pplano, BYTE clr_s1, BYTE clr_s2,
  char *titulo);
void vg_dibuja(STC_VENTANAG *ventana, int rellena);
void vg_abre(STC_VENTANAG *ventana);
void vg_cierra(STC_VENTANAG *ventana);
void vg_pon_cursor(STC_VENTANAG *ventana, int fil, int col);
void vg_impc(STC_VENTANAG *ventana, char car);
void vg_impcad(STC_VENTANAG *ventana, char *cad, int rellena);
void vg_modo_texto(STC_VENTANAG *ventana, int modo);
void vg_borra(STC_VENTANAG *ventana);
void vg_color(STC_VENTANAG *ventana, BYTE fondo, BYTE pplano);
void vg_borde(STC_VENTANAG *ventana, int borde);
void vg_scroll_arr(STC_VENTANAG *ventana);
void vg_scroll_abj(STC_VENTANAG *ventana);

#endif  /* VENTANAG_H */
