/**************************************
    Fichero de cabecera de
    las funciones de manejo
    de editor de texto
    de EDITOR.C
**************************************/

#if !defined (EDITOR_H)
#define EDITOR_H

/*** Constantes ***/
#define E_MAXNUMLIN     5000    /* m ximo n£mero de l¡neas del editor */
#define E_MAXLNGLIN     256     /* m xima longitud de una l¡nea */
#define RETURN          0x000d  /* c¢digo ASCII de RETURN */
#define TAB             0x0009  /* c¢digo ASCII de tabulaci¢n */
#define E_FLECHARR      0x18    /* car cter de flecha arriba */
#define E_FLECHABJ      0x19    /* car cter de flecha abajo */
#define E_FLECHIZQ      0x1b    /* car cter de flecha izquierda */
#define E_FLECHDER      0x1a    /* car cter de flecha derecha */
#define E_CARRELL1      0xb2    /* car cter de relleno 1 */
#define E_CARRELL2      0xfe    /* car cter de relleno 2 */
#define E_CHR1F         '['     /* 1er car. de zona nombre de fichero */
#define E_CHR2F         ']'     /* £ltimo car. de zona nombre de fichero */
#define E_CHRMOD        0x0f    /* car cter de texto modificado */
#define E_CHRNOMOD      ' '     /* car cter de texto no modificado */
#define E_EDITA         0       /* se permite editar el texto */
#define E_SOLOLECT      1       /* s¢lo se permite leer el texto */
#define E_PAUSA1        200     /* pausa para primera pulsaci¢n rat¢n */
#define E_PAUSA2        50      /* pausa para sucesivas pulsaciones rat¢n */
#define E_CX            32      /* m ximo n£mero de caracteres especiales */
#define E_PX            50      /* m ximo n£mero de palabras clave/grupo */

#define CB_FIL          7       /* fila del cuadro de b£squeda */
#define CB_COL          14      /* columna del cuadro de b£squeda */
#define CB_ANCHO        50      /* anchura del cuadro de b£squeda */
#define CB_ALTO         8       /* altura del cuadro de b£squeda */

/* c¢digos de scan de teclas especiales */
#define TCUR_IZQ        0x4b    /* cursor izquierda */
#define TCUR_DER        0x4d    /* cursor derecha */
#define TCUR_ABJ        0x50    /* cursor abajo */
#define TCUR_ARR        0x48    /* cursor arriba */
#define TPAG_ABJ        0x51    /* p gina abajo */
#define TPAG_ARR        0x49    /* p gina arriba */
#define TFIN            0x4f    /* fin */
#define TORG            0x47    /* origen */
#define TCTR_FIN        0x75    /* CTRL + fin */
#define TCTR_ORG        0x77    /* CTRL + origen */
#define TESC            0x01    /* ESCAPE */
#define TDEL            0x0e    /* DELETE */
#define TSUP            0x53    /* borrar */
#define TINS            0x52    /* INSERT */
#define TCTR_INS        0x92    /* CTRL + INSERT */
#define TCTR_SUP        0x93    /* CTRL + borrar */
#define TCTR_L          0x26    /* CTRL + L */
#define TCTR_Y          0x15    /* CTRL + Y */
#define TCTR_X          0x2d    /* CTRL + X */
#define TCTR_C          0x2e    /* CTRL + C */
#define TCTR_V          0x2f    /* CTRL + V */
#define TF1             0x3b    /* F1 */
#define TF2             0x3c    /* F2 */
#define TF3             0x3d    /* F3 */
#define TF4             0x3e    /* F4 */
#define TF5             0x3f    /* F5 */
#define TF6             0x40    /* F6 */
#define TF7             0x41    /* F7 */
#define TF8             0x42    /* F8 */
#define TF9             0x43    /* F9 */
#define TF10            0x44    /* F10 */
#define TSF1            0x54    /* Shift+F1 */

/*** Tipos de datos y estructuras ***/
#if !defined (BYTE)
#define BYTE unsigned char
#endif

/* estructura de l¡neas de bloque de texto */
typedef struct _bloque {
    char *lin;              /* puntero a texto de l¡nea actual */
    struct _bloque *sgte;   /* puntero a siguiente l¡nea */
} STC_BLOQUE;

/* estructura con datos de posici¢n del cursor del editor */
typedef struct {
    int lin, col;       /* posici¢n dentro del texto */
    int filcur, colcur; /*     "       "   de la ventana */
} CURSOR_ED;

/*** Prototipos ***/
void e_inicializa(char *fich, int fil, int col, int ancho, int alto,
  BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_bloque,
  BYTE clr_comentario, BYTE clr_palabraclv, BYTE clr_especial,
  BYTE clr_princc, BYTE clr_s1c, BYTE clr_s2c, BYTE clr_botonc,
  BYTE clr_inputc, BYTE clr_teclac, BYTE clr_tecla_botonc, char *fsintx,
  int modo);
void e_elimina(void);
void e_cambia_color(BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_bloque,
  BYTE clr_comentario, BYTE clr_palabraclv, BYTE clr_especial,BYTE clr_princc, BYTE clr_s1c, BYTE clr_s2c, BYTE clr_botonc,
  BYTE clr_inputc, BYTE clr_teclac, BYTE clr_tecla_botonc, int modo);
void e_borra_texto(void);
int e_editor(void);
void e_dibuja_editor(void);
int e_carga_texto(char *fich);
int e_graba_texto(char *fich);
void e_nombre_fichero(char *nf);
int e_inicia_busqueda(void);
int e_continua_busqueda(void);
int e_modificado(void);
void e_cambia_modo(int modo);
int e_carga_textox(char *fich, long desplz);
void e_vacia_bloque(void);
int e_graba_bloque(char *nfich);
int e_inserta_bloque(void);
void e_pon_cursor(int lin, int col);
char *e_palabra_cursor(void);
void e_cursor_guarda(CURSOR_ED *cur);
void e_cursor_recupera(CURSOR_ED *cur);

#endif  /* EDITOR_H */
