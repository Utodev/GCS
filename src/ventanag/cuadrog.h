/**************************************
	Fichero de cabecera de
	las funciones de manejo
	de cuadros de di logo
	de CUADROG.C
**************************************/

#if !defined (CUADROG_H)
#define CUADROG_H

/*** Constantes ***/
#define CG_MAXELEM      15      /* m ximo n§ de elementos en un cuadro */
#define CG_CENT         9999    /* valor para centrar el cuadro */
#define CG_CARTECLA     '^'     /* car cter que indica tecla asociada */
#define CG_FLECHARR     0x18    /* car cter flecha arriba */
#define CG_FLECHABJ     0x19    /* car cter flecha abajo */
#define CG_FICH_ANCHO   34      /* anchura cuadro selecci¢n de ficheros */
#define CG_FICH_ALTO    14      /* altura cuadro selecci¢n de ficheros */
#define CG_CHRDIR1      0x11    /* car. 1 para marcar nombres directorios */
#define CG_CHRDIR2      0x10    /* car. 2 para marcar nombres directorios */
#define CG_PAUSA        150     /* pausa para pulsaciones de rat¢n */

/* c¢digos de tipos de elementos de los cuadros de di logo */
#define CG_ELEM_NULO    0       /* elemento vac¡o */
#define CG_ELEM_BOTON   1       /* bot¢n */
#define CG_ELEM_INPUT   2       /* ventana de entrada */
#define CG_ELEM_LISTA   3       /* lista de cadenas de caracteres */
#define CG_ELEM_CHECK   4       /* caja de comprobaci¢n */
#define CG_ELEM_TEXTO   5       /* texto */

/* c¢digos de acciones para cuadros de di logo */
#define CUADROG_NULA     	-1     	/* acci¢n nula */
#define CUADROG_SALIDA    	-2     	/* salida de un cuadro (ESCAPE) */
#define CUADROG_SELECC    	-3     	/* selecci¢n del elemento actual */
#define CUADROG_SGTE_ELEM 	-4     	/* siguiente elemento */
#define CUADROG_ANT_ELEM  	-5     	/* elemento anterior */
#define CUADROG_TECLA     	-6     	/* env¡a tecla */
#define CUADROG_RATON     	-7   	/* env¡a acci¢n de rat¢n */

/* modos de funcionamiento de elemento de lista */
#define CG_LSTNORMAL    0       /* normal */
#define CG_LSTPRIMERO   1       /* elemento seleccionado siempre primero */
/* modo de ordenaci¢n de elemento de lista */
#define CG_LSTSINORDEN  0       /* lista desordenada */
#define CG_LSTORDENADA  1       /* lista ordenada */

/* modos de presentaci¢n de elemento de texto */
#define CG_TXTLINEA     0       /* cada l¡nea por separado */
#define CG_TXTPASA      1       /* pasa l¡nea a siguiente si no cabe */
#define CG_TXTNOBORDE   0       /* no imprime borde alrededor de texto */
#define CG_TXTBORDE     1       /* imprime borde alrededor de texto */

/*** Tipos de datos y estructuras ***/
typedef struct _elemg {
	int fil, col;		/* posici¢n del elemento dentro del cuadro */
	int tipo;               /* tipo de elemento */
	void *info;             /* puntero a informaci¢n del elemento */
	struct _elemg *ant; 	/* puntero elemento anterior, NULL si primero */
	struct _elemg *sgte;	/* puntero sgte. elemento, NULL si £ltimo */
} STC_ELEMG;

typedef struct {
	STC_VENTANAG v;		/* ventana del cuadro */
	STC_ELEMG *elem1;	/* puntero a primer elemento del cuadro */
	BYTE clr_teclaf;        /* color fondo teclas de activaci¢n */
	BYTE clr_teclap;	/* color primer plano teclas activ. */
	STC_ELEMG *elemento;	/* elemento actual */
} STC_CUADROG;

typedef struct {
	int accion;             /* c¢digo de acci¢n */
	int tecla;             	/* c¢digo tecla pulsada (CUADROG_TECLA) */
	int shift;		/* estado teclas 'shift' (CUADROG_TECLA) */
	int fil, col;        	/* posici¢n del rat¢n (CUADROG_RATON) */
} STC_ACCION_CG;

/* elemento bot¢n */
typedef struct {
	int ancho;		/* anchura del bot¢n */
	char *texto;            /* texto bot¢n, CG_CARTECLA marca tecla asoc. */
	char tecla;             /* tecla asociada */
} STC_ELEM_BOTONG;

/* elemento INPUT */
typedef struct {
	int ancho;		/* anchura zona INPUT */
	char *texto;            /* texto, CG_CARTECLA marca tecla asociada */
	char tecla;             /* tecla asociada */
	char *cadena;           /* puntero a buffer para la cadena a teclear */
	int longitud;           /* m x. longitud de cadena (sin contar '\0') */
} STC_ELEM_INPUTG;

/* elemento lista */
typedef struct _lista {
	char *cadena;           /* puntero a cadena de caracteres */
	struct _lista *sgte;    /* puntero a siguiente elemento */
} STC_LISTAG;
typedef struct {
	STC_VENTANAG v;		/* ventana para mostrar lista */
	char *texto;            /* texto, CG_CARTECLA marca tecla asociada */
	int modo;               /* modo de desplazamiento de la lista */
				/* CG_LSTNORMAL=normal */
				/* CG_LSTPRIMERO=seleccionado siempre primero */
	int orden;              /* modo de ordenaci¢n de la lista */
				/* CG_LSTSINORDEN=desordenada */
				/* CG_LSTORDENADA=ordenada */
	char tecla;             /* tecla asociada */
	STC_LISTAG *elemento;   /* puntero a primer elemento de la lista */
	int num_elementos;      /* n£mero de elementos en lista */
	int elemento_pr;        /* primer elemento a mostrar */
	int elemento_sel;       /* n§ de elemento seleccionado */
	char *selecc;           /* puntero a cadena de elem. seleccionado */
} STC_ELEM_LISTAG;

/* elemento caja de comprobaci¢n */
typedef struct {
	char *texto;            /* texto, CG_CARTECLA marca tecla asociada */
	char tecla;             /* tecla asociada */
	int estado;             /* estado: 1 activo, 0 inactivo */
} STC_ELEM_CHECKG;

/* elemento texto */
typedef struct {
	STC_VENTANAG v;		/* ventana */
	char *texto;            /* texto */
	int modo;               /* modo de impresi¢n */
				/* CG_TXTLINEA=cada l¡nea por separado */
				/* CG_TXTPASA=pasa l¡nea a sgte. si no cabe */
	int borde;              /* modo de impresi¢n de borde */
				/* CG_TXTNOBORDE=no imprime borde */
				/* CG_TXTBORDE=borde alrededor de texto */
} STC_ELEM_TEXTOG;

/*** Prototipos ***/
void cg_crea_cuadro(STC_CUADROG *cuad, char *titulo, int fil, int col,
  int ancho, int alto, BYTE clr_fondo, BYTE clr_pplano, BYTE clr_s1,
  BYTE clr_s2, BYTE clr_teclaf, BYTE clr_teclap);
STC_ELEMG *cg_crea_elemento(STC_CUADROG *cuad, int tipo, int fil, int col,
  ...);
int cg_dibuja_elemento(STC_CUADROG *cuad, STC_ELEMG *e);
void cg_abre(STC_CUADROG *cuad);
void cg_cierra(STC_CUADROG *cuad);
void cg_elimina(STC_CUADROG *cuad);
int cg_lee_input(int fil, int col, int ancho, BYTE colorf, BYTE colorp,
  char *cadena, int longitud);
int cg_accion(STC_CUADROG *c, STC_ACCION_CG *acc);
int cg_gestiona(STC_CUADROG *c);
int cg_elem_input(int fil, int col, BYTE colorf, BYTE colorp,
  STC_ELEM_INPUTG *e);
int cg_elem_lista(STC_ELEM_LISTAG *e);
int cg_mete_en_lista(STC_ELEM_LISTAG *e, char *cadena);
void cg_borra_lista(STC_ELEM_LISTAG *e);
int cg_elem_check(STC_ELEM_CHECKG *e, STC_CUADROG *cuad, STC_ELEMG *elem);
void cg_selecc_ficheros(int fil, int col, char *titulo, BYTE clr_fondo,
  BYTE clr_pplano, BYTE clr_s1, BYTE clr_s2, BYTE clr_teclaf, BYTE clr_teclap,
  char *ruta, char *mascara, char *fichero);

#endif  /* CUADROG_H */

