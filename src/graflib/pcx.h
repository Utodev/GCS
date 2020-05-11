/**************************************
	Fichero de cabecera de
	las funciones de manejo
	de ficheros PCX de PCX.C
**************************************/

#if !defined (PCX_H)
#define PCX_H

/*** Macros ***/
#define RGB(r,g,b) (((long)((b) << 8 | (g)) << 8) | (r))

/*** Constantes ***/
#define TAMLINPCX16	80	/* tama¤o de 1 plano de l¡nea 16 colores */
#define TAMLINPCX256	320     /* tama¤o de l¡nea 256 colores */
#define TAMLINPCX256X	90      /* tama¤o de l¡nea modo extendido 256 colores */

/* c¢digos de error de las funciones de decodificaci¢n de ficheros PCX */
#define E_PCX_APER  1           /* error de apertura de fichero */
#define E_PCX_LECT  2           /* error de lectura de fichero */
#define E_PCX_FORM  3           /* formato de fichero no v lido */
#define E_PCX_MODO  4           /* modo de pantalla incorrecto */
#define E_PCX_MEM   5           /* no hay memoria suficiente para imagen */

/*** Tipos de datos y estructuras ***/
#if !defined (BYTE)
#define BYTE unsigned char
#endif

/* cabecera de fichero PCX */
typedef struct {
	BYTE propietario;       /* deber¡a ser siempre 0x0A */
	BYTE version;           /* version del fichero PCX */
	BYTE metodo_codif;      /* 1=codificaci¢n PCX 'run-lenght' */
	BYTE bits_pixel;        /* n£mero de bits por pixel */
	int ventana[4];         /* tama¤o dibujo (izq., arr., der., abajo) */
	int resh, resv;         /* resoluci¢n horizontal y vertical */
	BYTE paleta[16][3];     /* informaci¢n de paleta para la imagen */
	BYTE reserv1;
	BYTE num_planos;        /* n£mero de planos de color */
	int bytes_scan;         /* n£mero de bytes por plano de l¡nea scan */
	BYTE reserv2[60];
} STC_CABPCX;

/* imagen decodificada */
typedef struct {
	int ancho, alto;        /* dimensiones en pixels */
	int num_colores;        /* n£mero de colores (m x. 256) */
	BYTE paleta[256][3];    /* paleta RGB de colores */
	BYTE far *(*lineas);	/* punteros a l¡neas de imagen */
} STC_IMAGEN;

/*** Prototipos ***/
int pcx_decodifica(char *nombre_pcx, STC_IMAGEN *imagen);
void pcx_libera_mem(STC_IMAGEN *imagen);
int pcx_dibuja(STC_IMAGEN *imagen, int x, int y, int anchura, int altura,
  int paleta);
int pcx_visualiza(char *nombre_pcx, int x, int y, int anchura, int altura,
  int paleta);

#endif  /* PCX_H */
