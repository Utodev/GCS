/**************************************
      Fichero de cabecera principal
      del sistema SINTAC
**************************************/

#if !defined (SINTAC_H)
#define SINTAC_H

#define WORD unsigned int


/*** Constantes ***/
#define MODO0_COL  80           /* columnas en el modo 0 (640x480x16) */
#define MODO0_FIL  30           /* filas en el modo 0 */
#define MODO1_COL  40           /* columnas en el modo 1 (320x200x256) */
#define MODO1_FIL  25           /* filas en el modo 1 */
#define END_OF_PROCESS_MARK 0	/* marca de fin de proceso en la tabla de entradas */
#define END_OF_CONDACTS_MARK 0XFF	/* marca de fin de condactos en una entrada */


#define N_SECCS    6            /* número de secciones en base de datos */
#define LONG_LIN   129          /* longitud máxima de línea de entrada */
#define MARCA_S    (char)'\\'   /* marca de inicio de sección */
#define CHR_COMENT (char)';'    /* indicador de comentario */
#define CHR_DELIM  (char)'@'    /* delimitador de texto */
#define MARCA_CNX  (char)'#'    /* marca de inicio de conexión */
#define CHR_NOPAL  (char)'_'    /* carácter indicador 'cualquier palabra' */
#define MARCA_ETQ  (char)'$'    /* marca de etiqueta */
#define BAND_0     (char)'O'    /* carácter para band. usuario desactivada */
#define BAND_1     (char)'X'    /* carácter para band. usuario activada */
#define LNGCOND    7            /* longitud de un condacto */
#define LABELS     100          /* máximo número de etiquetas */
#define LONGETQ    14           /* longitud máxima de etiqueta */
#define FSKIP      100          /* máximo número de saltos (SKIP) 'forward' */
#define FPROCESS   2000         /* máximo número de llamadas 'forward' */
#define NCONST     1000         /* máximo número de constantes */
#define LNGCONST   14           /* longitud máxima de constantes */
#define FLAGS      256          /* número de variables */
#define BANDS      32           /* número de banderas/8 */
#define MAXLONG    128          /* máxima longitud de la línea de entrada */
				/* en el intérprete */

#define V_MOV      14           /* máximo núm. para verbos de movimiento */
#define N_CONV     20           /*    "    "     "  nombres convertibles */
#define N_PROP     50           /*    "    "     "     "    propios */
#define LONGPAL    5            /* longitud de palabra de vocabulario */
#define MAX_VAL	   255			/* máximo valor para las variables (flags) */

#define NUM_PAL    768          /* máximo número de palabras en vocabulario */
#define MAX_MSY    256          /*   "      "    "  Mensajes del Sistema */
#define MAX_MSG    255           /*   "      "    "  Mensajes */
#define MAX_LOC    255           /*   "      "    "  Localidades */
#define MAX_OBJ    255           /*   "      "    "  Objetos */
#define MAX_PRO    255           /*   "      "    "  Procesos */
#define MAX_TMSG   255            /*   "      "    "  Tablas de mensajes */


#define NUM_MSY    32           /* número mínimo de Mensajes del Sistema */
#define TAM_MEM    0xffdc     	/* tamaño (bytes) de buffers de compilador */

#define BANCOS_RAM 2    /* número de bancos de memoria para RAMSAVE/RAMLOAD */
#define N_VENT     10   /* máximo número de ventanas */
#define N_BORD     9    /* máximo número de tipos de borde */

/* FLAGS DEL SISTEMA */

#define FDARK 		0   /* Si no es cero, está  oscuro */
#define FCARRIEDOBJ	1   /* Objetos llevados, que no puestos */ 
#define FMOUSEX		2	/* Extra flag modo SINTAC: MOUSE X */
#define FMOUSEY		3	/* Extra flag modo SINTAC: MOUSE Y */
#define FMOUSEB1	4	/* Extra flag modo SINTAC: MOUSE BUTTON 1*/
#define FMOUSEB2	5	/* Extra flag modo SINTAC: MOUSE BUTTON 2*/
#define FASK 		11  /* Extra flag modo SINTAC: Control de ASK */
#define FINPUT 		12  /* Extra flag modo SINTAC: Control de INPUT */
#define FACTIVEMSG  17	/* Extra flag modo SINTAC: Tabla de mensajes activa */ 
#define FSCORE		30  /* Contador de puntuación */
#define FTURNSL		31	/* Contador de turnos LSB */
#define FTURNSH		32	/* Contador de turnos MSB */
#define FVERB		33	/* Verbo de la SL actual */
#define FNOUN		34	/* Nombre de la SL actual */
#define FADJECT		35  /* Adjetivo de la SL actual */
#define FADVERB		36  /* Adverbio de la sentencia lógica actual */
#define FABILITY	37	/* Máximo número de objetos que un jugador puede llevar */
#define FPLAYER 	38  /* Localidad actual del jugador */
#define FSTREAM		41  /* Stream actual de entrada */
#define FPROMPT		42  /* Número del mensaje del prompt, 0 para random */
#define FPREP		43  /* Preposición de la sentencia actual */
#define FNOUN2		44  /* NOUN2 y ADJECT2 */
#define FADJECT2	45
#define FPRONUNNOUN	46  /* Nombre y adjetivo del ultimo pronombre */
#define FPRONUNADJ	47
#define FTIMEOUT	48  /* Duración del timeout  */

#define FTIMEOUTFLG 49	/* Flags de control del timeout*/
#define FTTIMEOUT 	7	/* Is set if timeout happened */  /* PENDIENTE*/
#define FTRECALL 	6	/* If data available for recall */  /* PENDIENTE*/
#define FTAUTRECALL	5	/* Set this to cause auto recall of input buffer after  timeout */  /* PENDIENTE*/
#define FTPRINTINP 	4	/* Set this to print input in current stream after edit */  /* PENDIENTE*/
#define FTCLRWIN 	3	/* Set this to clear input window */  /* PENDIENTE*/
#define FTANYKEY 	2	/* Set this so timeout can occur on ANYKEY */  /* PENDIENTE*/
#define FTMORE	 	1	/* Set this so timeout can occur on "More..." */  /* PENDIENTE*/
#define FTSTART 	0	/* Set this so timeout can occur at start of input only */  /* PENDIENTE*/

#define FDOALLOBJ	50  /* Objeto para el bucle DOALL */
#define FRFOBJ		51  /* ?ltimo objeto referenciado */
#define FABILITY2	52  /* Máximo peso que el jugador puede llevar */

#define FFLAGS		53  /* Print flags para LISTOBJ/LISTAT. and other new flags */
#define FFOBJLISTED 7	/* Bit 7 is set to 1 if any object listed during LISTOBJ/LISTAT */
#define FFLISTCOL	6	/* Bit 6, if set to 1 listing is made in column */
#define FFMOUSEON 	5	/* Bit 5, if set, mouse is active */
#define FFSINTAC	0	/* Bit 0, if set SINTAC mode is activated and some things start to work */	

									 
#define FRFOBJLOC	54	/* Localidad del último objeto referenciado */
#define FRFOBJWEIG	55	/* Peso del último objeto referenciado */
#define FRFOBJCONT	56	/* 128 si el último objeto referenciado es contenedor */
#define FRFOBJWEAR	57	/* 128 si el último objeto referenciado es prenda */
#define FRFOBJATTRL	58	/* Atributos bajos del último objeto referenciado */
#define FRFOBJATTRH	59	/* Atributos altos del último objeto referenciado */
#define FKEY1		60  /* Primer flag para retornar un valor de tecla pulsada */
#define FKEY2		61  /* Segundo flag para retornar un valor de tecla pulsada */
#define FVIDEOMODE	62	/* Video mode */
#define FACTWINDOW	63	/* Active window */



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
#define _CONJ  3        /*   "     "  conjunción */
#define NO_PAL 255	/* valor de palabra nula */

#define NO_CREADO  252  /* número de localidad para objs. no creados */
#define PUESTO     253  /*   "    "      "      "     "   puestos */
#define COGIDO     254  /*   "    "      "      "     "   cogidos */
#define LOC_ACTUAL 255  /*   "    "      "     que equivale a loc. actual */

/* cadena de reconocimiento de fichero de base de datos */
/* los 2 caracteres finales indican la versión de la base de datos */
/* las versiones disponibles son: */
/*      'T1' primera versión, modo texto */
/*      'T2' segunda versión, modo texto */
/*      'G1' primera versión, modo gráfico */
/*      'G2' segunda versión, modo gráfico */
/*		'G3' tercera versión, modo gráfico */
/*   	'D1' primera versión, modo DAAD */
#define SRECON  "JSJ SINTAC D1"
#define L_RECON 13              /* longitud de la cadena de reconocimiento */

/* cadena de reconocimiento de fichero de fuente */
/* el último carácter indica la versión de fichero */
/* versiones disponibles: */
/*      "1"     fuente de 8x14 */
/*      "2"     fuentes de 8x16 y 8x8 */
/*	"3"	fuentes de 8x16 y 8x8 proporcionales */
#define RECON_FUENTE  "JSJ SINTAC FNT3"
#define LONG_RECON_F  15        /* longitud de la cadena de reconocimiento */

/* prefijo de indirección */
#define INDIR   255

/* nombre de fichero donde se guardarán los errores del compilador */
#define NF_ERR  "$SINT$.ERR"

/* nombre de fichero de configuración */
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

/* estructura de cabecera de fichero de base de datos DAAD */
typedef struct {
	BYTE version;
	BYTE target_machine_language;
	BYTE the95;
	BYTE num_obj;
	BYTE num_loc;
	BYTE num_msg;
	BYTE num_msy;
	BYTE num_pro;
	WORD tok_pos;
	WORD pro_pos;
	WORD obj_pos;
	WORD loc_pos;
	WORD msg_pos;
	WORD msy_pos;
	WORD con_pos;
	WORD voc_pos;
	WORD obj_initially_pos;
	WORD obj_names_pos;
	WORD obj_weight_cont_wear_pos;
	WORD obj_att_pos;
	WORD file_length;
} CAB_DAAD;


/* estructura de código de retorno de error */
typedef struct {
	int codigo;
	unsigned long linea;
} STC_ERR;

/* estructura de etiqueta */
typedef struct {
	char etq[LONGETQ+1];    /* para guardar nombre etiqueta */
	BYTE *petq;             /*   "     "    dirección etiqueta */
} STC_ETIQUETA;

/* estructura para saltos (SKIP) 'forward' */
typedef struct {
	char etq[LONGETQ+1];    /* nombre etiqueta a sustituir */
	BYTE *fsk;              /* dónde sustituir etiqueta */
	unsigned long nl;       /* número de línea en archivo de entrada */
} STC_SKPFORWARD;

/* estructura para llamadas a procesos 'forward' */
typedef struct {
	BYTE numpro;            /* número de proceso al que se llama */
	unsigned long nl;       /* línea de archivo de entrada dónde llama */
} STC_PRCFORWARD;

/* estructura para constantes */
typedef struct {
	char cnst[LNGCONST+1];  /* nombre de la constante */
	BYTE valor;             /* valor de la constante */
} STC_CONSTANTE;

/* estructura del fichero de configuración */
typedef struct {
	char dir_sintac[MAXPATH];       /* dir. del sistema SINTAC */
	char dir_bd[MAXPATH];           /* dir. bases de datos */
	char dir_util[MAXPATH];         /* dir. de utilidades */
	BYTE color_men;                 /* colores de menús de opciones */
	BYTE color_mens1;
	BYTE color_mens2;
	BYTE color_mentec;
	BYTE color_mensel;
	BYTE color_ved;                 /* colores de ventana de edición */
	BYTE color_veds1;
	BYTE color_veds2;
	BYTE color_vedblq;
	BYTE color_vedcoment;
	BYTE color_vedesp;
	BYTE color_vedpalclv;
	BYTE color_dlg;                 /* colores de cuadros de diálogo */
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
