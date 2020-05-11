/**************************************
	Fichero de cabecera de
	las funciones de gesti¢n
	de ayuda de AYUDA.C
**************************************/

#if !defined (AYUDA_H)
#define AYUDA_H

/*** Constantes ***/
#define A_EXTAYD        ".AYD"  /* extensi¢n fichero de ayuda */
#define A_EXTIYD        ".IYD"  /* extensi¢n fichero ¡ndice ayuda */
#define A_ERRANCHO      40      /* anchura de cuadro para mensajes error */
#define A_ERRALTO       7       /* altura de cuadro para mensajes error */
#define A_CHRSECC1      '¯'     /* car cter de inicio secci¢n de ayuda */
#define A_CHRSECC2      '®'     /* car cter de fin secci¢n de ayuda */
#define A_CHRREF        'ð'     /* car cter de referencia */
#define A_LNGLINAYD     81      /* m xima longitud+1 de l¡nea de ayuda */
#define A_LNGREF        41      /* m xima longitud+1 de referencia */

/*** Tipos de datos y estructuras ***/
typedef struct {
	char txt[A_LNGREF];     /* texto de la referencia */
	fpos_t pos;             /* posici¢n de secci¢n en fichero de ayuda */
} STC_REFAYD;

/*** Prototipos ***/
void a_inicializa(int fil, int col, int ancho, int alto, int ancho_vref,
  BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_boton, BYTE clr_input,
  BYTE clr_tecla, BYTE clr_tecla_boton, BYTE clr_selecc);
void a_elimina(void);
void a_cambia_color(BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_boton, 
  BYTE clr_input, BYTE clr_tecla, BYTE clr_tecla_boton, BYTE clr_selecc);
int a_crea_fichero_indices(char *nfayd);
int a_ayuda(char *nfayd, char *tema);

#endif  /* AYUDA_H */
