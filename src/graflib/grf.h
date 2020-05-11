/**************************************
	Fichero de cabecera de
	las funciones gr ficas
	de GRF.C
**************************************/

#if !defined (GRF_H)
#define GRF_H

/*** Constantes ***/
/* modos de v¡deo */
#define G_MV_T40	0x00	/* texto 40x25, 16 colores sin r faga color */
#define G_MV_T40C	0x01	/* texto 40x25, 16 colores */
#define G_MV_T80      	0x02    /* texto 80x25, 16 colores sin r faga color */
#define G_MV_T80C      	0x03	/* texto 80x25, 16 colores */
#define G_MV_G1C16	0x0e	/* gr ficos 640x200, 16 colores */
#define G_MV_G2C16	0x10	/* gr ficos 640x350, 16 colores */
#define G_MV_G3C16	0x12	/* gr ficos 640x480, 16 colores */
#define G_MV_G1C256	0x13	/* gr ficos 320x200, 256 colores */
#define G_MV_G2C256	0xff	/* gr ficos 360x480, 256 colores */

/* modos de dibujo */
#define G_NORM	0x00
#define G_AND   0x08
#define G_OR    0x10
#define G_XOR   0x18

/*** Macros ***/
/* calcula direcci¢n de pixel en buffer de v¡deo en los modos de 16 colores */
/* a la entrada: AX=coordenada Y, BX=coordenada X */
/* a la salida: AH=m scara de bit, CL=n§ de bits a desplazar a izquierda */
/*              ES:BX=direcci¢n en buffer de v¡deo */
#define DIR_PIXEL16	asm { 			 \
				mov cl,bl	;\
				push dx		;\
				mov dx,80	;\
				mul dx          ;\
				pop dx		;\
				shr bx,1	;\
				shr bx,1	;\
				shr bx,1	;\
				add bx,ax	;\
				mov ax,0a000h	;\
				mov es,ax	;\
				and cl,7	;\
				xor cl,7        ;\
				mov ah,1	;\
			}
/* calcula direcci¢n de pixel en buffer de v¡deo en el modo 320x200x256 */
/* a la entrada: AX=coordenada Y, BX=coordenada X */
/* a la salida: ES:BX=direcci¢n en buffer de v¡deo */
#define DIR_PIXEL256	asm {			 \
				xchg ah,al	;\
				add bx,ax	;\
				shr ax,1	;\
				shr ax,1	;\
				add bx,ax       ;\
				mov ax,0a000h	;\
				mov es,ax	;\
			}
/* calcula direcci¢n de pixel en buffer de v¡deo en el modo 360x480x256 */
/* a la entrada: AX=coordenada Y, BX=coordenada X */
/* a la salida: ES:BX=direcci¢n en buffer de v¡deo */
#define DIR_PIXEL256X	asm {			 \
				mov dx,90	;\
				mul dx          ;\
				shr bx,1        ;\
				shr bx,1        ;\
				add bx,ax       ;\
				mov ax,0a000h   ;\
				mov es,ax       ;\
			}

/*** Prototipos ***/
int g_coge_modovideo(void);
int g_modovideo(int mvid);
int g_maxx(void);
int g_maxy(void);
void g_borra_pantalla(void);
unsigned char g_coge_pixel(int x, int y);
void g_punto(int x, int y, unsigned char color, unsigned char modo);
void g_linea(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo);
void g_rectangulo(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo, int relleno);
void g_scroll_arr(int fila, int columna, int ancho, int alto,
  unsigned char color);
void g_scroll_abj(int fila, int columna, int ancho, int alto,
  unsigned char color);

#endif  /* GRF_H */
