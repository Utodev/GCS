/**************************************
      Fichero de cabecera principal
      del sistema SINTAC
**************************************/

#if !defined (SINTAC_H)
#define SINTAC_H

/*** Constantes ***/
#define MODO0_COL  80           /* columnas en el modo 0 (640x480x16) */
#define MODO0_FIL  30           /* filas en el modo 0 */
#define MODO1_COL  40           /* columnas en el modo 1 (320x200x256) */
#define MODO1_FIL  25           /* filas en el modo 1 */

#define N_SECCS    6            /* n£mero de secciones en base de datos */
#define LONG_LIN   129          /* longitud m xima de l¡nea de entrada */
#define MARCA_S    (char)'\\'   /* marca de inicio de secci¢n */
#define CHR_COMENT (char)';'    /* indicador de comentario */
#define CHR_DELIM  (char)'@'    /* delimitador de texto */
#define MARCA_CNX  (char)'#'    /* marca de inicio de conexi¢n */
#define CHR_NOPAL  (char)'_'    /* car cter indicador 'cualquier palabra' */
#define MARCA_ETQ  (char)'$'    /* marca de etiqueta */
#define BAND_0     (char)'O'    /* car cter para band. usuario desactivada */
#define BAND_1     (char)'X'    /* car cter para band. usuario activada */
#define LNGCOND    7            /* longitud de un condacto */
#define LABELS     100          /* m ximo n£mero de etiquetas */
#define LONGETQ    14           /* longitud m xima de etiqueta */
#define FSKIP      100          /* m ximo n£mero de saltos (SKIP) 'forward' */
#define FPROCESS   2000         /* m ximo n£mero de llamadas 'forward' */
#define NCONST     1000         /* m ximo n£mero de constantes */
#define LNGCONST   14           /* longitud m xima de constantes */
#define VARS       256          /* n£mero de variables */
#define BANDS      32           /* n£mero de banderas/8 */
#define MAXLONG    128          /* m xima longitud de la l¡nea de entrada */
				/* en el int‚rprete */

#define V_MOV      14           /* m ximo n£m. para verbos de movimiento */
#define N_CONV     20           /*    "    "     "  nombres convertibles */
#define N_PROP     50           /*    "    "     "     "    propios */
#ifdef DAAD
	#define LONGPAL    5            /* longitud de palabra de vocabulario */
	#define MAX_VAL	   255			/* m ximo valor para las variables (flags) */
#else
	#define LONGPAL    6            /* longitud de palabra de vocabulario */
#endif
#if SHARE==0
#define NUM_PAL    2500         /* m ximo n£mero de palabras en vocabulario */
#define MAX_MSY    255          /*   "      "    "  Mensajes del Sistema */
#define MAX_MSG    255          /*   "      "    "  Mensajes */
#define MAX_LOC    252          /*   "      "    "  Localidades */
#define MAX_OBJ    255          /*   "      "    "  Objetos */
#define MAX_PRO    255          /*   "      "    "  Procesos */
#define MAX_TMSG   255          /*   "      "    "  Tablas de mensajes */
#else
#define NUM_PAL    200          /* m ximo n£mero de palabras en vocabulario */
#define MAX_MSY    256          /*   "      "    "  Mensajes del Sistema */
#define MAX_MSG    25           /*   "      "    "  Mensajes */
#define MAX_LOC    25           /*   "      "    "  Localidades */
#define MAX_OBJ    25           /*   "      "    "  Objetos */
#define MAX_PRO    25           /*   "      "    "  Procesos */
#define MAX_TMSG   3            /*   "      "    "  Tablas de mensajes */
#endif
#define NUM_MSY    32           /* n£mero m¡nimo de Mensajes del Sistema */
#define TAM_MEM    0xffdc     	/* tama¤o (bytes) de buffers de compilador */

#define BANCOS_RAM 2    /* n£mero de bancos de memoria para RAMSAVE/RAMLOAD */
#define N_VENT     10   /* m ximo n£mero de ventanas */
#define N_BORD     9    /* m ximo n£mero de tipos de borde */

/* orden de las secciones */
#define VOC 0
#define MSY 1
#define MSG 2
#define LOC 3
#define OBJ 4
#define PRO 5

#define _VERB  0        /* valor para verbo */
#define _NOMB  1        /*   "     "  nombre */
#define _ADJT  2        /*   "     "  adjetivo */
#define _CONJ  3        /*   "     "  conjunci¢n */
#define NO_PAL 255	/* valor de palabra nula */

#define NO_CREADO  252  /* n£mero de localidad para objs. no creados */
#define PUESTO     253  /*   "    "      "      "     "   puestos */
#define COGIDO     254  /*   "    "      "      "     "   cogidos */
#define LOC_ACTUAL 255  /*   "    "      "     que equivale a loc. actual */

/* cadena de reconocimiento de fichero de base de datos */
/* los 2 caracteres finales indican la versi¢n de la base de datos */
/* las versiones disponibles son: */
/*      'T1' primera versi¢n, modo texto */
/*      'T2' segunda versi¢n, modo texto */
/*      'G1' primera versi¢n, modo gr fico */
/*      'G2' segunda versi¢n, modo gr fico */
/*	'G3' tercera versi¢n, modo gr fico */
#define SRECON  "JSJ SINTAC G3"
#define L_RECON 13              /* longitud de la cadena de reconocimiento */

/* cadena de reconocimiento de fichero de fuente */
/* el £ltimo car cter indica la versi¢n de fichero */
/* versiones disponibles: */
/*      "1"     fuente de 8x14 */
/*      "2"     fuentes de 8x16 y 8x8 */
/*	"3"	fuentes de 8x16 y 8x8 proporcionales */
#define RECON_FUENTE  "JSJ SINTAC FNT3"
#define LONG_RECON_F  15        /* longitud de la cadena de reconocimiento */

/* prefijo de indirecci¢n */
#define INDIR   255

/* nombre de fichero donde se guardar n los errores del compilador */
#define NF_ERR  "$SINT$.ERR"

/* nombre de fichero de configuraci¢n */
#define NF_CFG  "SINTAC.CFG"

/*** Macros ***/
/* macro para codificar BYTES */
#define CODIGO(b) (BYTE)0xff-(BYTE)b

/*** Tipos de datos y estructuras ***/
#if !defined (BYTE)
#define BYTE unsigned char
#endif

typedef enum {FALSE=0, TRUE} BOOLEAN;

/* estructura de palabra de vocabulario */
struct palabra {
	char p[LONGPAL+1];      /* palabra */
	BYTE num;		/* valor */
	BYTE tipo;		/* tipo */
};

/* estructura de cabecera de fichero de base de datos */
typedef struct {
	char srecon[L_RECON+1];         /* cadena de reconocimiento */
	long fpos_voc;                  /* pos. en fichero de vocabulario */
	BYTE v_mov;                     /* m x. n£m. de verbo de movimiento */
	BYTE n_conv;                    /*  "    "    " nombre convertible */
	BYTE n_prop;                    /*  "    "    " nombre propio */
	int pal_voc;                    /* n£mero de palabras en vocabulario */
	long fpos_msy;                  /* pos. en fichero de mens. sistema */
	BYTE num_msy;                   /* n£mero de mensajes del sistema */
	unsigned bytes_msy;             /* memoria para mensajes del sistema */
	long fpos_msg[MAX_TMSG];        /* pos. en fichero de tablas de */
					/* mensajes, 0 si tabla no existe */
	BYTE num_msg[MAX_TMSG];         /* n£mero de mensajes (por tabla) */
	unsigned bytes_msg[MAX_TMSG];   /* memoria para mensajes (por tabla) */
	long fpos_loc;                  /* pos. en fichero de localidades */
	BYTE num_loc;                   /* n£mero de localidades */
	unsigned bytes_loc;             /* memoria para localidades */
	unsigned bytes_conx;            /*   "      "   conexiones */
	long fpos_obj;                  /* pos. en fichero de objetos */
	BYTE num_obj;                   /* n£mero de objetos */
	unsigned bytes_obj;             /* memoria para objetos */
	long fpos_pro;                  /* pos. en fichero de procesos */
	BYTE num_pro;                   /* n£mero de procesos */
	unsigned bytes_pro;             /* memoria para procesos */
} CAB_SINTAC;

/* estructura de c¢digo de retorno de error */
typedef struct {
	int codigo;
	unsigned long linea;
} STC_ERR;

/* estructura de etiqueta */
typedef struct {
	char etq[LONGETQ+1];    /* para guardar nombre etiqueta */
	BYTE *petq;             /*   "     "    direcci¢n etiqueta */
} STC_ETIQUETA;

/* estructura para saltos (SKIP) 'forward' */
typedef struct {
	char etq[LONGETQ+1];    /* nombre etiqueta a sustituir */
	BYTE *fsk;              /* d¢nde sustituir etiqueta */
	unsigned long nl;       /* n£mero de l¡nea en archivo de entrada */
} STC_SKPFORWARD;

/* estructura para llamadas a procesos 'forward' */
typedef struct {
	BYTE numpro;            /* n£mero de proceso al que se llama */
	unsigned long nl;       /* l¡nea de archivo de entrada d¢nde llama */
} STC_PRCFORWARD;

/* estructura para constantes */
typedef struct {
	char cnst[LNGCONST+1];  /* nombre de la constante */
	BYTE valor;             /* valor de la constante */
} STC_CONSTANTE;

/* estructura del fichero de configuraci¢n */
typedef struct {
	char dir_sintac[MAXPATH];       /* dir. del sistema SINTAC */
	char dir_bd[MAXPATH];           /* dir. bases de datos */
	char dir_util[MAXPATH];         /* dir. de utilidades */
	BYTE color_men;                 /* colores de men£s de opciones */
	BYTE color_mens1;
	BYTE color_mens2;
	BYTE color_mentec;
	BYTE color_mensel;
	BYTE color_ved;                 /* colores de ventana de edici¢n */
	BYTE color_veds1;
	BYTE color_veds2;
	BYTE color_vedblq;
	BYTE color_vedcoment;
	BYTE color_vedesp;
	BYTE color_vedpalclv;
	BYTE color_dlg;                 /* colores de cuadros de di logo */
	BYTE color_dlgs1;
	BYTE color_dlgs2;
	BYTE color_dlgboton;
	BYTE color_dlginput;
	BYTE color_dlgsel;
	BYTE color_dlgtec;
	BYTE color_dlgtecboton;
	BYTE color_ayd;                 /* colores de ventana de ayuda */
	BYTE color_ayds1;
	BYTE color_ayds2;
	BYTE color_aydboton;
	BYTE color_aydinput;
	BYTE color_aydsel;
	BYTE color_aydtec;
	BYTE color_aydtecboton;
	BYTE color_err;                 /* colores de ventana de errores */
	BYTE color_errs1;
	BYTE color_errs2;
	BYTE color_errboton;
	BYTE color_errinput;
	BYTE color_errsel;
	BYTE color_errtec;
	BYTE color_errtecboton;
	BYTE color_cs;                  /* colores de ventana compilador */
	BYTE color_css1;   		/* y de linkador */
	BYTE color_css2;
	BYTE color_csboton;
	BYTE color_csinput;
	BYTE color_cssel;
	BYTE color_cstec;
	BYTE color_cstecboton;
} STC_CFG;

#endif  /* SINTAC_H */
