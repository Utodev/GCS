/**************************************
	Fichero de cabecera de
	las funciones de manejo
	de men£s de MENU.C
**************************************/

#if !defined (MENU_H)
#define MENU_H

/*** Constantes ***/
#define MAX_NUMOPCIONES 20      /* m ximo n§ de opciones de un men£ */
#define MENU_HORZ       0       /* tipo de men£ horizontal */
#define MENU_VERT       1       /* tipo de men£ vertical */
#define MENU_FIJO       0x80    /* activa men£ fijo, que no se cierra */
#define MENU_NFIJO      0x7f    /* para comprobaciones de tipo de men£ */
#define CAR_FINOPC      ':'     /* fin de texto de opci¢n */
#define CAR_SEPAR       '|'     /* separador de opciones */
#define CAR_TECLA       '^'     /* marca tecla de activaci¢n */
#define SEPARADOR       'Ä'     /* car cter separador */

/* contenido de matriz de colores */
#define MCLR_PRINCIPAL	0
#define MCLR_S1		1
#define MCLR_S2		2
#define MCLR_TECLA      3
#define MCLR_SELECC	4

/* c¢digos de acciones para men£s */
#define MENU_NULA       -1      /* acci¢n nula */
#define MENU_SALIDA     -2      /* salida de un men£ (ESCAPE) */
#define MENU_IZQUIERDA  -3      /* movimiento de cursor izquierda */
#define MENU_DERECHA    -4      /* movimiento de cursor derecha */
#define MENU_ARRIBA     -5      /* movimiento de cursor arriba */
#define MENU_ABAJO      -6      /* movimiento de cursor abajo */
#define MENU_SELECCIONA -7      /* selecci¢n de opci¢n */
#define MENU_TECLA      -8      /* env¡a tecla */
#define MENU_RATON      -9      /* env¡a acci¢n de rat¢n */

/*** Tipos de datos y estructuras ***/
#if !defined (BYTE)
#define BYTE unsigned char
#endif

typedef struct {
	char *opcion;           /* puntero a texto de la opci¢n */
				/* "" si £ltima opci¢n */
	char tecla;             /* c¢digo ASCII de la tecla de activaci¢n */
	int fil, col;         	/* posici¢n de la opci¢n en la ventana */
	int lng_opcion;         /* longitud del texto de la opci¢n */
} STC_OPCION;

typedef struct {
	STC_VENTANA v;        	/* ventana del men£ */
	STC_OPCION opc[MAX_NUMOPCIONES];        /* opciones del men£ */
	BYTE tipo;              /* tipo de men£: MENU_HORZ o MENU_VERT */
	int num_opciones;       /* n£mero de opciones */
	int separ_opc;        	/* separaci¢n entre opciones */
				/* s¢lo MENU_HORZ */
	int opcion;             /* £ltima opci¢n seleccionada */
} STC_MENU;

typedef struct {
	int accion;             /* c¢digo de acci¢n */
	char tecla;             /* tecla pulsada (MENU_TECLA) */
	int fil, col;         	/* posici¢n del rat¢n (MENU_RATON) */
} STC_ACCION;

/*** Prototipos ***/
STC_MENU *m_crea(BYTE tipo, char *titulo, char *opciones, int fil,
  int col, int separ_opciones);
void m_abre(STC_MENU *menu);
void m_cierra(STC_MENU *menu);
void m_elimina(STC_MENU *menu);
void m_cambia_color(STC_MENU *menu);
void m_resalta_opcion(STC_MENU *menu, int resalta);
int m_accion(STC_MENU *menu, STC_ACCION *acc);
int m_elige_opcion(STC_MENU *menu);
void m_color(BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_tecla,
  BYTE clr_sel);

#endif  /* MENU_H */

