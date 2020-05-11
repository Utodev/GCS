/**************************************
	Fichero de cabecera de
	GRFPCX.C
**************************************/

#if !defined (GRFPCX_H)
#define GRFPCX_H

/*** Macros ***/
#define RGB(r,g,b) (((long)((b) << 8 | (g)) << 8) | (r))

/*** Constantes ***/
#define GRF_ID	"JSJ GRF1"	/* cadena de identificaci¢n */
#define LNG_GRF_ID	9   	/* longitud cadena identificaci¢n */
#define TAMLINGRF16	80      /* tama¤o de 1 plano de l¡nea 16 colores */
#define TAMLINGRF256	320     /* tama¤o de l¡nea 256 colores */
#define TAMLINGRF256X	90      /* tama¤o de l¡nea modo extendido 256 colores */

/* c¢digos de error de las funciones de decodificaci¢n de ficheros GRF */
#define E_GRF_APER	1       /* error de apertura de fichero */
#define E_GRF_LECT	2       /* error de lectura de fichero */
#define E_GRF_FORM	3       /* formato de fichero no v lido */
#define E_GRF_IMG	4	/* formato de imagen no v lido */
#define E_GRF_MODO  	5       /* modo de pantalla incorrecto */
#define E_GRF_MEM   	6       /* no hay memoria suficiente para imagen */
#define E_GRF_NOEX  	7	/* no existe esa imagen en fichero */

/*** Tipos de datos y estructuras ***/
#if !defined (BYTE)
#define BYTE unsigned char
#endif

/* cabecera de fichero GRF */
typedef struct {
	char id[LNG_GRF_ID];	/* cadena identificativa */
	int n_img;		/* n£mero de im genes en fichero */
} STC_CABGRF;

/* cabecera de imagen en fichero GRF */
typedef struct {
	int num;		/* n£mero de imagen */
	long tam_img;		/* tama¤o imagen (bytes) sin decodificar */
	BYTE bits_pixel;        /* n£mero de bits por pixel */
	int ventana[4];         /* tama¤o dibujo (izq., arr., der., abajo) */
	BYTE paleta[256][3];    /* informaci¢n de paleta para la imagen */
	BYTE num_planos;        /* n£mero de planos de color */
	int bytes_scan;         /* n£mero de bytes por plano de l¡nea scan */
} STC_CABIMG;

/* imagen decodificada */
typedef struct {
	int ancho, alto;        /* dimensiones en pixels */
	int num_colores;        /* n£mero de colores (m x. 256) */
	BYTE paleta[256][3];    /* paleta RGB de colores */
	BYTE far *(*lineas);	/* punteros a l¡neas de imagen */
} STC_IMGRF;

/*** Prototipos ***/
int grf_decodifica(char *nombre_grf, int n_img, STC_IMGRF *imagen);
void grf_libera_mem(STC_IMGRF *imagen);
int grf_dibuja(STC_IMGRF *imagen, int x, int y, int anchura, int altura,
  int paleta);
int grf_visualiza(char *nombre_grf, int n_img, int x, int y, int anchura,
  int altura, int paleta);

#endif  /* GRFPCX_H */
