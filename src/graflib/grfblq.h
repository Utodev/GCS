/**************************************
	Fichero de cabecera de
	las funciones de manejo de
	bloques gr ficos de GRFBLQ.C
**************************************/

#if !defined (GRFBLQ_H)
#define GRFBLQ_H

/*** Prototipos ***/
unsigned long blq_tam(int x0, int y0, int x1, int y1);
void blq_coge(int x0, int y0, int x1, int y1, unsigned char far *blq);
void blq_pon(int x, int y, unsigned char far *blq);

#endif  /* GRFBLQ_H */
