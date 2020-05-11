/**************************************
	Fichero de cabecera de
	las funciones de manejo
	de ventanas de VENTANA.C
**************************************/

#if !defined (VENTANA_H)
#define VENTANA_H

/*** Constantes ***/
#define V_LINEA_LINEA   0       /* imprimir l¡nea a l¡nea */
#define V_PASA_LINEA    1       /* pasa de una l¡nea a otra */
#define V_NORELLENA     0       /* no rellena con espacios imprimir cadena */
#define V_RELLENA       1       /* rellena con espacios al imprimir cadena */

/*** Tipos de datos y estructuras ***/
#if !defined (BYTE)
#define BYTE unsigned char
#endif

typedef struct {
	int fil, col;         	/* posici¢n de la ventana en pantalla */
	int ancho, alto;      	/* dimensiones de la ventana */
	int sombra;		/* 1 si la ventana tiene sombra, 0 si no */
	BYTE clr_princ;         /* color principal de la ventana */
	BYTE clr_s1;            /* color para sombra 1 */
	BYTE clr_s2;            /* color para sombra 2 */
	char *borde;            /* puntero a cadena con caracteres de borde */
	char *titulo;           /* puntero a cadena con texto encabezamiento */
	BYTE far *fondo;        /* puntero a buffer donde guardar fondo */
	int modo_texto;         /* modo de impresi¢n de texto */
	int filc, colc;         /* posici¢n de impresi¢n dentro de ventana */
	BYTE clr_texto;         /* color del texto de la ventana */
} STC_VENTANA;

/*** Prototipos ***/
void v_impcar(int fil, int col, BYTE car, BYTE clr);
void v_crea(STC_VENTANA *ventana, int fil, int col, int ancho,
  int alto, BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, char *titulo,
  int sombra);
void v_dibuja(STC_VENTANA *ventana, int rellena);
void v_abre(STC_VENTANA *ventana);
void v_cierra(STC_VENTANA *ventana);
void v_pon_cursor(STC_VENTANA *ventana, int fil, int col);
void v_impc(STC_VENTANA *ventana, char car);
void v_impcad(STC_VENTANA *ventana, char *cad, int rellena);
void v_modo_texto(STC_VENTANA *ventana, int modo);
void v_borra(STC_VENTANA *ventana);
void v_color(STC_VENTANA *ventana, BYTE clr);
void v_borde_def(char *borde);
void v_borde(STC_VENTANA *ventana, char *borde);
void v_scroll_arr(STC_VENTANA *ventana, int n);
void v_scroll_abj(STC_VENTANA *ventana, int n);
void v_scroll_izq(STC_VENTANA *ventana, int n);
void v_scroll_der(STC_VENTANA *ventana, int n);

#endif  /* VENTANA_H */
