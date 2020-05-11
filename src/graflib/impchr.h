/**************************************
	Fichero de cabecera de
	las funciones de impresi¢n
	de caracteres de IMPCHR.C
**************************************/

#if !defined (IMPCHR_H)
#define IMPCHR_H

/*** Constantes ***/
/* modos de impresi¢n de caracteres con la funci¢n imp_chr() */
#define CHR_NORM  0x00
#define CHR_AND   0x08
#define CHR_OR    0x10
#define CHR_XOR   0x18

/*** Prototipos ***/
int chr_altura(void);
int chr_anchura(unsigned char chr);
int chr_maxfil(void);
int chr_maxcol(void);
void imp_chr_pos(int x, int y);
int imp_chr(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo);
void def_chr_set(unsigned char *ptr_set1, unsigned char *ptr_set2,
  unsigned char *ptr_anch1, unsigned char *ptr_anch2);

#endif  /* IMPCHR_H */

