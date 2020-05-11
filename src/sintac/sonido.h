/**************************************
	Fichero de cabecera de
	SONIDO.C
**************************************/

#if !defined (SONIDO_H)
#define SONIDO_H

/*** Constantes ***/
#define NUM_NOTAS	8*12	/* n£mero de notas en tabla de frecuencias */

#define SND_SOLO 	0x00	/* SND se reproduce una sola vez */
#define SND_MULT        0x01	/*  "  "      "     c¡clicamente */

#define SND_SILENCIO	9999	/* silencio */
#define SND_ID		"JSJ SND1"	/* cadena identificaci¢n */
#define LNG_SND_ID      9	/* longitud cadena de identificaci¢n */

/* c¢digos de error */
#define E_SND_APER 	1	/* error de apertura fichero SND */
#define E_SND_LECT 	2	/* error de lectura fichero SND */
#define E_SND_FORM 	3	/* formato de fichero no v lido */
#define E_SND_MEM  	4	/* no hay suficiente memoria */

/*** Tipos de datos y estructuras ***/
/* cabecera de fichero SND */
typedef struct {
	char id[LNG_SND_ID];	/* cadena identificativa */
	int n_snd;		/* n£mero de notas en fichero */
} STC_CABSND;

/* nota en fichero SND */
typedef struct {
	int nota;		/* n§ de nota; silencio=SND_SILENCIO */
	unsigned duracion;	/* duraci¢n en d‚cimas de segundo */
} STC_SND;

/*** Prototipos ***/
void bpr_sonido(unsigned frec, unsigned duracion);
void bpr_nota(int nota, unsigned duracion);
int bpr_musica(char *nf_snd, int modo);

#endif  /* SONIDO_H */