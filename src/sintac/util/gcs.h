/**************************************
	Fichero de cabecera principal
	de GCS.C
**************************************/

/*** Constantes ***/
#define GCS_VERSION	"4.2"

#define COLOR_FONDO   	7      	/* color fondo de botones */
#define COLOR_PPLANO   	0       /* color texto de botones */
#define COLOR_S1      	15      /* color sombra d‚bil */
#define COLOR_S2      	8       /* color sombra fuerte */
#define COLOR_TECLA  	4       /* color tecla activaci¢n */
#define COLOR_F_CUAD  	0       /* color de fondo de cuadr¡cula */
#define COLOR_B_CUAD  	7       /* color de borde de cuadr¡cula */
#define COLOR_R_CUAD  	15      /* color de relleno de cuadr¡cula */
#define COLOR_M_CUAD	12	/* color de marcador de cuadr¡cula */
#define COLOR_A_CUAD    12	/* color de marcador de anchura */
#define COLOR_F_ASCII 	0	/* color de fondo de tabla ASCII */
#define COLOR_L_ASCII 	15      /* color de los car cteres de tabla ASCII */
#define COLOR_R_ASCII 	12      /* color de marcador de tabla ASCII */
#define COLOR_F_PRB   	0       /* color de fondo de ventana prueba */
#define COLOR_P_PRB   	10      /* color primer plano de ventana prueba */

#define NBOT		18	/* n£mero de botones en cuadro */
#define DEFCAR_X        304	/* posici¢n X de car cter tama¤o real */
#define DEFCAR_Y        64	/*    "     Y  "    "        "    "   */
#define DEFCAR_ANCHO	40	/* anchura de ventana car cter tama¤o real */
#define DEFCAR_ALTO	20	/* altura  "     "       "       "     "   */
#define CUAD_ANCHO    	12      /* anchura de cuadro de la cuadr¡cula */
#define CUAD_ALTO     	16      /* altura de cuadro de la cuadr¡cula */
#define TAB_ROM8x16   	6       /* tabla de 8x16 de la ROM */
#define TAB_ROM8x8    	3       /* tabla de 8x8 de la ROM */
#define ALT_T8X16     	18	/* altura tabla ASCII 8x16 */
#define ALT_T8X8      	10 	/* altura tabla ASCII 8x8 */
#define ALT_C8X16     	18	/* altura cuadr¡cula 8x16 */
#define ALT_C8X8      	10  	/* altura cuadr¡cula 8x8 */
#define NPRB_CAR      	40      /* n§ de caracteres en ventana de prueba */

#define AVISO_CF      	8       /* color fondo ventana aviso */
#define AVISO_CPP     	14      /* color primer plano ventana aviso */
#define AVISO_CS1     	7       /* color sombra d‚bil ventana aviso */
#define AVISO_CS2     	0       /* color sombra fuerte ventana aviso */
#define AVISO_CT      	12      /* color tecla activaci¢n ventana aviso */

/* teclas */
#define T_ESC		0x01	/* ESCAPE */
#define T_IZQ		0x4b	/* cursor izquierda */
#define T_DER		0x4d    /*   "    derecha */
#define T_ARR		0x48    /*   "    arriba */
#define T_ABJ		0x50    /*   "    abajo */
#define T_ESP		0x39	/* espacio */
#define T_BORR		0x53	/* borrar */
#define T_CTRLIZQ	0x73	/* CTRL+cursor izquierda */
#define T_CTRLDER	0x74    /* CTRL+cursor derecha */
#define T_F1		0x3b	/* F1 */
#define T_F2		0x3c    /* F2 */
#define T_F3		0x3d    /* F3 */
#define T_F4		0x3e    /* F4 */
#define T_F5		0x3f    /* F5 */
#define T_F6		0x40    /* F6 */
#define T_F7		0x41    /* F7 */
#define T_F8		0x42    /* F8 */
#define T_F9		0x43    /* F9 */
#define T_F10		0x44    /* F10 */
#define T_SF1		0x54    /* Shift+F1 */
#define T_SF2		0x55	/* Shift+F2 */
#define T_SF3		0x56    /* Shift+F3 */
#define T_SF4		0x57    /* Shift+F4 */
#define T_SF5		0x58    /* Shift+F5 */

/* modos de dibujo de caracteres con rutina imp_chrdef() */
#define C_NORM        	0x00
#define C_AND         	0x08
#define C_OR          	0x10
#define C_XOR         	0x18

/*** Prototipos ***/
int int24_hnd(int errval, int ax, int bp, int si);
void carga_iconos(char *nf_icn, int nicn, STC_ICONO icn[]);
void cuadro_aviso(char *msg);
int cuadro_siono(char *msg);
void dibuja_cuadricula(int x, int y);
void dibuja_marc_ancho(int x, int y, int c);
void dibuja_marc_cuadricula(int x, int y, int orgx, int orgy, int modo);
void cambia_cuadricula(int x, int y, int orgx, int orgy, int rellena);
void imp_chrdef(BYTE far *pchr, int x, int y, int colorf, int color, int alt,
  BYTE modo);
void dibuja_tabla_ascii(int x, int y);
void cambia_caracter(BYTE *car, int x, int y, int bit);
void limpia_caracter(BYTE *car, BYTE byte);
int pulsa_en_ascii(int x, int y);
void dibuja_marcador_ascii(int ascii);
void copia_tabla_rom(BYTE tabla, BYTE far *tabla_car, int alt);
void copia_def_car(BYTE far *origen, BYTE far *destino);
void dibuja_en_cuadricula(void);
void actualiza_tabla(int x, int y, int ascii);
void dibuja_caracter(void);
int graba_def(char *nombre, int type);
int carga_def(char *nombre);
void cuadricula(int x, int y);
void tab_ascii(int x, int y);
void def_caracter(void);
void almacena_definicion(void);
void borra_rejilla(void);
void llena_rejilla(void);
void scroll_def_arr(void);
void scroll_def_abj(void);
void scroll_def_izq(void);
void scroll_def_der(void);
void invertir_def(void);
void gira180(void);
void espejo_def(void);
void cargar_fuente(void);
void grabar_fuente(int type);
void pasa_a8x16(void);
void pasa_a8x8(void);
void prueba_car(void);
void imp_ascii_car(void);
void tablacar_modificada(void);
void rayado_horizontal(void);
void rayado_vertical(void);
void cursiva_izquierda(void);
void cursiva_derecha(void);
void negrita(void);
void inc_ancho(void);
void dec_ancho(void);
int car_anch(int c);