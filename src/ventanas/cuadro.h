/**************************************
	Fichero de cabecera de
	las funciones de manejo
	de cuadros de di logo
	de CUADRO.C
**************************************/

#if !defined (CUADRO_H)
#define CUADRO_H

/*** Constantes ***/
#define C_CENT          9999    /* valor para centrar el cuadro */
#define C_CARTECLA      '^'     /* car cter que indica tecla asociada */
#define C_FICH_ANCHO    40      /* anchura cuadro selecci¢n de ficheros */
#define C_FICH_ALTO     14      /* altura cuadro selecci¢n de ficheros */
#define C_CHRDIR1       0x11    /* car. 1 para marcar nombres directorios */
#define C_CHRDIR2       0x10    /* car. 2 para marcar nombres directorios */
#define C_PAUSA         150     /* pausa para pulsaciones de rat¢n */

/* caracteres para botones */
#define CBOTON1		'<'	/* lateral izquierdo del bot¢n */
#define CBOTON2         '>'	/* lateral derecho del bot¢n */
#define CBOTON1_ON	0x11	/* lateral izquierdo del bot¢n resaltado */
#define CBOTON2_ON      0x10	/* lateral derecho del bot¢n resaltado */
#define CBOTONS1	0xdf	/* sombra 1 */
#define CBOTONS2	0xdc	/* sombra 2 */

/* caracteres para zona de INPUT */
#define CINPUT1		' '	/* lateral izquierdo de zona 'input' */
#define CINPUT2         ' '	/* lateral derecho de zona 'input' */
#define CINPUT1_ON	'['	/* lateral izquierdo de 'input' resaltado */
#define CINPUT2_ON      ']'	/* lateral derecho de 'input' resaltado */

/* caracteres para cajas de comprobaci¢n */
#define CCHECK_BOR1     '['     /* lateral izquierdo para caja */
#define CCHECK_BOR2     ']'     /* lateral derecho para caja */
#define CCHECK_MARC     0x07    /* marca para caja actual */
#define CCHECK_ON       'X'     /* indicador de caja activa */
#define CCHECK_OFF      ' '     /* indicador de caja desactivada */

/* c¢digos de tipos de elementos de los cuadros de di logo */
#define C_ELEM_NULO     0       /* elemento vac¡o */
#define C_ELEM_BOTON    1       /* bot¢n */
#define C_ELEM_INPUT    2       /* ventana de entrada */
#define C_ELEM_LISTA    3       /* lista de cadenas de caracteres */
#define C_ELEM_CHECK    4       /* caja de comprobaci¢n */
#define C_ELEM_TEXTO    5       /* texto */

/* c¢digos de acciones para cuadros de di logo */
#define CUADRO_NULA      -1     /* acci¢n nula */
#define CUADRO_SALIDA    -2     /* salida de un cuadro (ESCAPE) */
#define CUADRO_SELECC    -3     /* selecci¢n del elemento actual */
#define CUADRO_SGTE_ELEM -4     /* siguiente elemento */
#define CUADRO_ANT_ELEM  -5     /* elemento anterior */
#define CUADRO_TECLA     -6     /* env¡a tecla */
#define CUADRO_RATON     -7     /* env¡a acci¢n de rat¢n */

/* modos de funcionamiento de elemento de lista */
#define C_LSTNORMAL     0       /* normal */
#define C_LSTPRIMERO    1       /* elemento seleccionado siempre primero */
/* modo de ordenaci¢n de elemento de lista */
#define C_LSTSINORDEN   0       /* lista desordenada */
#define C_LSTORDENADA   1       /* lista ordenada */

/* modos de presentaci¢n de elemento de texto */
#define C_TXTLINEA      0       /* cada l¡nea por separado */
#define C_TXTPASA       1       /* pasa l¡nea a siguiente si no cabe */
#define C_TXTNOBORDE    0       /* no imprime borde alrededor de texto */
#define C_TXTBORDE      1       /* imprime borde alrededor de texto */

/*** Tipos de datos y estructuras ***/
typedef struct _elem {
	int fil, col;         	/* posici¢n del elemento dentro del cuadro */
	int tipo;               /* tipo de elemento */
	void *info;             /* puntero a informaci¢n del elemento */
	struct _elem *ant;	/* puntero elemento anterior, NULL si primero */
	struct _elem *sgte;	/* puntero sgte. elemento, NULL si £ltimo */
} STC_ELEM;

typedef struct {
	STC_VENTANA v;          /* ventana del cuadro */
	STC_ELEM *elem1; 	/* puntero a primer elemento del cuadro */
	BYTE clr_boton;		/* color de elemento bot¢n */
	BYTE clr_input;		/* color de elemento input */
	BYTE clr_sel;		/* color de elemento seleccionado de listas */
	BYTE clr_tecla;		/* color de teclas de activaci¢n */
	BYTE clr_tecla_boton;	/* color de tecla de activaci¢n bot¢n */
	STC_ELEM *elemento;     /* elemento actual */
} STC_CUADRO;

typedef struct {
	int accion;             /* c¢digo de acci¢n */
	int tecla;             	/* c¢digo tecla pulsada (CUADRO_TECLA) */
	int shift;		/* estado teclas 'shift' (CUADRO_TECLA) */
	int fil, col;        	/* posici¢n del rat¢n (CUADRO_RATON) */
} STC_ACCION_C;

/* elemento bot¢n */
typedef struct {
	int ancho;		/* anchura del bot¢n */
	char *texto;            /* texto, C_CARTECLA marca tecla asociada */
	char tecla;             /* tecla asociada */
} STC_ELEM_BOTON;

/* elemento INPUT */
typedef struct {
	int ancho;		/* anchura zona INPUT */
	char *texto;            /* texto, C_CARTECLA marca tecla asociada */
	char tecla;             /* tecla asociada */
	char *cadena;           /* puntero a buffer para la cadena a teclear */
	int longitud;           /* m x. longitud de cadena (sin contar '\0') */
} STC_ELEM_INPUT;

/* elemento lista */
typedef struct _lista {
	char *cadena;           /* puntero a cadena de caracteres */
	struct _lista *sgte;    /* puntero a siguiente elemento */
} STC_LISTA;
typedef struct {
	STC_VENTANA v;		/* ventana para mostrar lista */
	char *texto;            /* texto, C_CARTECLA marca tecla asociada */
	int modo;               /* modo de desplazamiento de la lista */
				/* C_LSTNORMAL=normal */
				/* C_LSTPRIMERO=seleccionado siempre primero */
	int orden;              /* modo de ordenaci¢n de la lista */
				/* C_LSTSINORDEN=desordenada */
				/* C_LSTORDENADA=ordenada */
	char tecla;             /* tecla asociada */
	STC_LISTA *elemento;    /* puntero a primer elemento de la lista */
	int num_elementos;      /* n£mero de elementos en lista */
	int elemento_pr;        /* primer elemento a mostrar */
	int elemento_sel;       /* n§ de elemento seleccionado */
	char *selecc;           /* puntero a cadena de elem. seleccionado */
} STC_ELEM_LISTA;

/* elemento caja de comprobaci¢n */
typedef struct {
	char *texto;            /* texto, C_CARTECLA marca tecla asociada */
	char tecla;             /* tecla asociada */
	int estado;             /* estado: 1 activo, 0 inactivo */
} STC_ELEM_CHECK;

/* elemento texto */
typedef struct {
	STC_VENTANA v;		/* ventana */
	char *texto;            /* texto */
	int modo;               /* modo de impresi¢n */
				/* C_TXTLINEA=cada l¡nea por separado */
				/* C_TXTPASA=pasa l¡nea a sgte. si no cabe */
	int borde;              /* modo de impresi¢n de borde */
				/* C_TXTNOBORDE=no imprime borde */
				/* C_TXTBORDE=borde alrededor de texto */
} STC_ELEM_TEXTO;

/*** Prototipos ***/
void c_crea_cuadro(STC_CUADRO *cuad, char *titulo, int fil, int col,
  int ancho, int alto, BYTE clr_princ, BYTE clr_s1, BYTE clr_s2,
  BYTE clr_boton, BYTE clr_input, BYTE clr_sel, BYTE clr_tecla,
  BYTE clr_tecla_boton);
STC_ELEM *c_crea_elemento(STC_CUADRO *cuad, int tipo, int fil, int col, ...);
void c_dibuja_elemento(STC_CUADRO *cuad, STC_ELEM *e);
void c_abre(STC_CUADRO *cuad);
void c_cierra(STC_CUADRO *cuad);
void c_elimina(STC_CUADRO *cuad);
void c_cambia_color(STC_CUADRO *cuad, BYTE clr_princ, BYTE clr_s1,
  BYTE clr_s2, BYTE clr_boton, BYTE clr_input, BYTE clr_sel, BYTE clr_tecla,
  BYTE clr_tecla_boton, int modo);
int c_lee_input(int fil, int col, int ancho, BYTE clr, char *cadena,
  int longitud);
int c_accion(STC_CUADRO *c, STC_ACCION_C *acc);
int c_gestiona(STC_CUADRO *c);
int c_elem_input(int fil, int col, BYTE clr, STC_ELEM_INPUT *e);
int c_elem_lista(STC_ELEM_LISTA *e, BYTE clr_sel);
int c_mete_en_lista(STC_ELEM_LISTA *e, char *cadena);
void c_borra_lista(STC_ELEM_LISTA *e);
int c_elem_check(STC_ELEM_CHECK *e, STC_CUADRO *cuad, STC_ELEM *elem);
void c_selecc_ficheros(int fil, int col, char *titulo, BYTE clr_princ,
  BYTE clr_s1, BYTE clr_s2, BYTE clr_boton, BYTE clr_input, BYTE clr_tecla,
  BYTE clr_tecla_boton, BYTE clr_sel, char *ruta, char *mascara, char *fichero);

#endif  /* CUADRO_H */

