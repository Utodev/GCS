/****************************************************************************
				    GRF.C

	Biblioteca de funciones gr ficas para los modos de 16 y 256 colores.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- g_coge_modovideo: devuelve modo de v¡deo actual
		- g_modovideo: establece un modo de v¡deo
		- g_maxx: devuelve n£mero de pixels horizontales de pantalla
		- g_maxy: devuelve n£mero de pixels verticales de pantalla
		- g_borra_pantalla: borra la pantalla
		- g_coge_pixel: devuelve color de un pixel
		- g_punto: dibuja un punto
		- g_linea: dibuja una l¡nea
		- g_rectangulo: dibuja un rect ngulo
		- g_scroll_arr: scroll hacia arriba de una ventana
		- g_scroll_abj: scroll hacia abajo de una ventana

	Las siguientes variables son p£blicas:
		- _g_modovideo: modo de v¡deo actual
****************************************************************************/

#include "alloc.h"
#include "grf.h"

/** Variables globales ***/
/* guarda modo de v¡deo actual */
int _g_modovideo=0;

/*** Variables globales internas ***/
/* paleta y registros de color para modos 16 colores */
static unsigned char paleta16[17]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
  11, 12, 13, 14, 15, 16};
static unsigned char regcol16[3*17]={0, 0, 0, 0, 0, 42, 0, 42, 0, 0, 42, 42,
  42, 0, 0, 42, 0, 42, 42, 21, 0, 42, 42, 42, 21, 21, 21, 21, 21, 63, 21, 63,
  21, 21, 63, 63, 63, 21, 21, 63, 21, 63, 63, 63, 21, 63, 63, 63, 0, 0, 0};
/* indicador de modo de v¡deo extendido activado, 0=no es modo extendido */
static int ext_mvid=0;

/*** Prototipos de funciones internas ***/
static void swapint(int *i1, int *i2);
static void g_modo256x(void);
static void g_punto16(int x, int y, unsigned char color, unsigned char modo);
static void g_punto256(int x, int y, unsigned char color, unsigned char modo);
static void g_punto256x(int x, int y, unsigned char color, unsigned char modo);
static void g_linea16(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo);
static void g_linea256(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo);
static void g_linea256x(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo);
static void g_rectangulo16(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo);
static void g_rectangulo256(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo);
static void g_rectangulo256x(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo);

/****************************************************************************
	SWAPINT: intercambia dos variables enteras.
	  Entrada:	'i1', 'i2' punteros a variables a intercambiar
****************************************************************************/
void swapint(int *i1, int *i2)
{
int i;

i=*i2;
*i2=*i1;
*i1=i;

}

/****************************************************************************
	G_MODO256X: activa modo VGA extendido de 360x480x256.
****************************************************************************/
void g_modo256x(void)
{

asm {
	push di

	mov ax,0013h	// modo 320x200x256
	int 10h
	mov dx,03c4h	// puerto del secuenciador
	mov ax,0604h	// desactiva 'Chain 4'
	out dx,ax
	mov ax,0f02h	// permite escribir en todos los planos
	out dx,ax

	mov ax,0a000h
	mov es,ax
	xor di,di	// ES:DI=inicio buffer de v¡deo
	mov cx,21600	// bytes en buffer de v¡deo, 360*480/4=43200/2=21600
	xor ax,ax
	rep stosw	// limpia la pantalla

	mov dx,03c4h	// puerto del secuenciador
	mov ax,0100h	// reset s¡ncrono
	out dx,ax
	mov dx,03c2h	// puerto miscel neo
	mov al,0e7h	// usar reloj de puntos de 28 MHz
	out dx,al
	mov dx,03c4h	// puerto del secuenciador
	mov ax,0300h	// reinicializar secuenciador
	out dx,ax
	mov dx,03d4h	// puerto del controlador del CRT
	mov al,11h	// registro 11 del CRT
	out dx,al
	inc dx
	in al,dx
	and al,7fh	// desproteger registros CR0-CR7
	out dx,al

	mov dx,03d4h	// puerto del controlador del CRT
	mov ax,6b00h	// total horizontal
	out dx,ax
	mov ax,5901h	// horizontal visualizado
	out dx,ax
	mov ax,5a02h	// inicio blanqueo horizontal
	out dx,ax
	mov ax,8e03h	// fin blanqueo horizontal
	out dx,ax
	mov ax,5e04h	// inicio sincronismo h
	out dx,ax
	mov ax,8a05h	// fin sincronismo h
	out dx,ax
	mov ax,0d06h	// total vertical
	out dx,ax
	mov ax,3e07h	// rebosamiento
	out dx,ax
	mov ax,4009h	// altura de celdilla
	out dx,ax
	mov ax,0ea10h	// inicio sincronismo v
	out dx,ax
	mov ax,0ac11h	// fin sincronismo v y protecci¢n registros CR0-CR7
	out dx,ax
	mov ax,0df12h	// vertical visualizado
	out dx,ax
	mov ax,2d13h	// desplazamiento
	out dx,ax
	mov ax,0014h	// desactiva modo DWORD
	out dx,ax
	mov ax,0e715h	// inicio blanqueo vertical
	out dx,ax
	mov ax,0616h	// fin blanqueo vertical
	out dx,ax
	mov ax,0e317h	// activa modo BYTE
	out dx,ax

	pop di
}

}

/****************************************************************************
	G_COGE_MODOVIDEO: devuelve el modo de v¡deo actual.
	  Salida:	modo de v¡deo
****************************************************************************/
int g_coge_modovideo(void)
{
int mvid;

if(ext_mvid) mvid=ext_mvid;
else {
	asm {
		mov ah,0fh
		int 10h         // AH=n§ de columnas, AL=modo de v¡deo
		mov ah,0
		mov mvid,ax
	}
}

return(mvid);
}

/****************************************************************************
	G_MODOVIDEO: establece un modo de v¡deo.
	  Entrada:	'mvid' modo de v¡deo (G_MV_XXXXXX)
	  Salida:	0 si no pudo establecer modo de v¡deo, 1 si pudo
****************************************************************************/
int g_modovideo(int mvid)
{
unsigned char far *pal, far *rgcol;
int tam;

if(mvid==G_MV_G2C256) {
	g_modo256x();
	ext_mvid=G_MV_G2C256;
	_g_modovideo=G_MV_G2C256;
}
else {
	ext_mvid=0;
	asm {
		mov ah,0
		mov al,byte ptr mvid
		int 10h
	}

	if(g_coge_modovideo()!=mvid) return(0);

	if((mvid==G_MV_G1C16) || (mvid==G_MV_G2C16) || (mvid==G_MV_G3C16)) {
		pal=paleta16;
		rgcol=regcol16;
		asm {
			mov ax,1002h    // definir paleta y borde
			les dx,pal
			int 10h
			mov ax,1012h    // definir registros de color
			mov bx,0
			mov cx,17
			les dx,rgcol
			int 10h
		}
	}
	_g_modovideo=mvid;
}

return(1);
}

/****************************************************************************
	G_MAXX: devuelve n£mero de pixels horizontales de pantalla.
	  Salida:	n£mero de pixels horizontales, 0 si modo de pantalla
			no v lido
****************************************************************************/
int g_maxx(void)
{

switch(_g_modovideo) {
	case G_MV_G1C16 :
		return(640);
	case G_MV_G2C16 :
		return(640);
	case G_MV_G3C16 :
		return(640);
	case G_MV_G1C256 :
		return(320);
	case G_MV_G2C256 :
		return(360);
}

return(0);
}

/****************************************************************************
	G_MAXY: devuelve n£mero de pixels verticales de pantalla.
	  Salida:	n£mero de pixels verticales, 0 si modo de pantalla
 			no v lido
****************************************************************************/
int g_maxy(void)
{

switch(_g_modovideo) {
	case G_MV_G1C16 :
		return(200);
	case G_MV_G2C16 :
		return(350);
	case G_MV_G3C16 :
		return(480);
	case G_MV_G1C256 :
		return(200);
	case G_MV_G2C256 :
		return(480);
}

return(0);
}

/****************************************************************************
	G_BORRA_PANTALLA: borra la pantalla.
****************************************************************************/
void g_borra_pantalla(void)
{
int filas_pixels;

if(_g_modovideo==G_MV_G1C256) {
	asm {
		push di

		mov di,0a000h
		mov es,di
		xor di,di	// ES:DI=direcci¢n inicio buffer de v¡deo

		xor ax,ax	// AX=0 (borrar buffer)

		mov cx,32000	// bytes de buffer de v¡deo/2
		rep stosw

		pop di
	}
}
else if(_g_modovideo==G_MV_G2C256) {
	asm {
		push di

		mov dx,03c4h	// puerto del secuenciador
		mov ax,0f02h	// permite escribir en todos los planos
		out dx,ax

		mov di,0a000h
		mov es,di
		xor di,di	// ES:DI=direcci¢n inicio buffer de v¡deo

		xor ax,ax	// AX=0 (borrar buffer)

		mov cx,21600
		rep stosw	// limpia la pantalla

		pop di
	}
}
else {
	/* calcula filas de pixels */
	filas_pixels=g_maxy()*40;

	asm {
		push di

		mov di,0a000h
		mov es,di
		xor di,di	// ES:DI=direcci¢n inicio buffer de v¡deo

		mov dx,03ceh	// puerto del controlador de gr ficos
		mov ax,0	// AL=n£mero registro set/reset, AH=0 (borrar)
		out dx,ax
		mov ax,0f01h	// AH=m scara activaci¢n set/reset
				// AL=n£mero registro activaci¢n set/reset
		out dx,ax

		mov cx,filas_pixels
		rep stosw	// rellena buffer de v¡deo

		mov ax,0001h	// inicializar registro activaci¢n set/reset
		out dx,ax

		pop di
	}
}

}

/****************************************************************************
	G_COGE_PIXEL: devuelve el color de un pixel.
	  Entrada:	'x', 'y' coordenadas del pixel
	  Salida:	color del pixel
****************************************************************************/
unsigned char g_coge_pixel(int x, int y)
{
unsigned char color;


if(_g_modovideo==G_MV_G1C256) {
	asm {
		mov ax,y
		mov bx,x
	}
		DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
	asm {
		mov al,es:[bx]
		mov color,al
	}
}
else if(_g_modovideo==G_MV_G2C256) {
	asm {
		mov ax,x
		and ax,3
		mov ah,al       // AH=n§ de plano de bits
		mov al,4
		mov dx,03ceh	// puerto del controlador gr fico
		out dx,ax	// selecciona plano

		mov ax,y
		mov bx,x
	}
		DIR_PIXEL256X;	// ES:BX=direcci¢n en buffer de v¡deo
	asm {
		mov al,es:[bx]	// guarda un byte desde cada plano de bits
		mov color,al
	}
}
else {
	asm {
		push si

		mov ax,y
		mov bx,x
	}
		DIR_PIXEL16;	// AH=m scara de bit
				// ES:BX=direcci¢n en buffer v¡deo
				// CL=cantidad de bits a desplazar a izquierda
	asm {
		mov ch,ah
		shl ch,cl	// CH=m scara de bit en posici¢n adecuada

		mov si,bx	// ES:SI=direcci¢n en buffer de v¡deo
		xor bl,bl	// BL se utiliza para acumular valor pixel

		mov dx,03ceh	// puerto del controlador de gr ficos
		mov ax,0304h	// AH=n£mero de plano de bits inicial
				// AL=n£mero registro selecci¢n de mapa lectura
	}
	cogepixel16:
	asm {
		out dx,ax     	// selecciona plano de bits
		mov bh,es:[si]	// BH=bytes desde plano actual
		and bh,ch	// enmascara bit
		neg bh		// bit 7 de BH=1 si bit enmascarado es 1
				// bit 7 de BH=0 si bit enmascarado es 0
		rol bx,1	// bit 0 de BL=siguiente bit de valor pixel
		dec ah		// AH=siguiente plano de bits
		jge cogepixel16

		mov color,bl

		pop si
	}
}

return(color);
}

/****************************************************************************
	G_PUNTO16: dibuja un punto en los modos de 16 colores.
	  Entrada:	'x', 'y' coordenadas
			'color' color del punto
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_punto16(int x, int y, unsigned char color, unsigned char modo)
{

asm {
	mov ax,y
	mov bx,x
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	shl ah,cl	// AH=m scara de bit en posici¢n correcta

	mov dx,03ceh	// puerto del controlador gr fico
	mov al,8	// AL=n£mero registro de m scara de bit
	out dx,ax

	mov ax,0205h	// AL=n£mero de registro de modo
			// AH=modo escritura 2, lectura 0
	out dx,ax

	mov ah,modo
	mov al,3	// AL=registro rotar dato/seleccionar funci¢n
	out dx,ax

	mov al,es:[bx]	// guarda un byte desde cada plano de bits
	mov al,color
	mov es:[bx],al	// actualiza planos de bits

	mov ax,0ff08h	// inicializa m scara de bit
	out dx,ax
	mov ax,0005h	// inicializa registro de modo
	out dx,ax
	mov ax,0003h	// inicializa registro seleccionar funci¢n
	out dx,ax
}

}

/****************************************************************************
	G_PUNTO256: dibuja un punto en el modo de 256 colores.
	  Entrada:	'x', 'y' coordenadas
			'color' color del punto
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_punto256(int x, int y, unsigned char color, unsigned char modo)
{

asm {
	mov ax,y
	mov bx,x
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov al,color

	cmp modo,G_NORM
	jne punto256_and

	mov es:[bx],al
}
	return;
punto256_and:
asm {
	cmp modo,G_AND
	jne punto256_or

	and es:[bx],al
}
	return;
punto256_or:
asm {
	cmp modo,G_OR
	jne punto256_xor

	or es:[bx],al
}
	return;
punto256_xor:
asm {
	xor es:[bx],al
}

}

/****************************************************************************
	G_PUNTO256X: dibuja un punto en el modo 360x480x256.
	  Entrada:	'x', 'y' coordenadas del punto
			'color' color del punto
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_punto256x(int x, int y, unsigned char color, unsigned char modo)
{

asm {
	mov cx,x
	and cx,3
	mov ax,1
	shl ax,cl	// m scara planos de bit
	mov ah,al
	mov al,2
	mov dx,03c4h	// puerto del secuenciador
	out dx,ax	// selecciona plano

	mov ax,y
	mov bx,x
}
	DIR_PIXEL256X;
asm {
	mov dx,03ceh	// puerto del controlador gr fico
	mov ah,modo
	mov al,3	// AL=registro rotar dato/seleccionar funci¢n
	out dx,ax

	mov al,es:[bx]	// guarda un byte desde cada plano de bits
	mov al,color
	mov es:[bx],al  // actualiza planos de bits

	mov ax,0003h	// inicializa registro seleccionar funci¢n
	out dx,ax
}

}

/****************************************************************************
	G_LINEA16: dibuja una l¡nea en los modos de 16 colores.
	  Entrada:	'x0', 'y0' punto de origen
			'x1', 'y1' punto final
			'color' color de la l¡nea
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_linea16(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int incvert, incr1, incr2, rutina;

asm {
	push si
	push di
	push ds

	mov dx,03ceh	// DX=puerto del controlador de gr ficos
	mov ah,color
	xor al,al	// AL=n£mero de registro set/reset
	out dx,ax

	mov ax,0f01h	// AH=m scara de plano de bit
			// AL=n£mero registro activaci¢n set/reset
	out dx,ax

	mov ah,modo	// modo de dibujo: 18h=XOR, 10h=OR, 08h=AND, 00h=NORM
	mov al,3	// AL=n£mero registro rotar dato/selecc. funci¢n
	out dx,ax

	mov si,80	// incremento para buffer de v¡deo
	mov cx,x1
	sub cx,x0	// CX=x1-x0
	jnz lin256_00
	jmp linea_vert16 	// salta si l¡nea vertical
}
lin256_00:
asm {
	jns lin16_01	// salta si x1>x0

	neg cx		// CX=x0-x1
	mov bx,x1
	xchg bx,x0
	mov x1,bx       // intercambia x0 y x1
	mov bx,y1
	xchg bx,y0
	mov y1,bx	// intercambia y0 e y1
}
lin16_01:
asm {
	mov bx,y1
	sub bx,y0	// BX=y1-y0
	jnz lin256_02
	jmp linea_horz16	// salta si l¡nea horizontal
}
lin256_02:
asm {
	jns lin16_03	// salta si pendiente positiva

	neg bx		// BX=y0-y1
	neg si		// incremento negativo para buffer de v¡deo
}
lin16_03:
asm {
	mov incvert,si	// guarda incremento vertical
	mov rutina,0
	cmp bx,cx
	jle lin16_04	// salta si dy<=dx (pendiente<=1)
	mov rutina,1
	xchg bx,cx	// intercambia dy y dx
}
lin16_04:
asm {
	shl bx,1
	mov incr1,bx    // incr1=2*dy
	sub bx,cx
	mov si,bx	// SI=d=2*dy-dx
	sub bx,cx
	mov incr2,bx	// incr2=2*(dy-dx)

	push cx
	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	shl ah,cl	// AH=m scara de bit desplazada
	mov bl,ah
	mov al,8	// AL=n£mero registro de m scara de bit
	pop cx
	inc cx		// CX=cantidad de pixels a dibujar

	cmp rutina,1
	je pendiente1
	jmp lin16_10
}
pendiente1:
asm {
	jmp lin16_15
}
linea_vert16:
asm {
	mov ax,y0
	mov bx,y1
	mov cx,bx
	sub cx,ax	// CX=dy
	jge lin16_05	// salta si dy>=0

	neg cx		// fuerza dy>=0
	mov ax,bx
}
lin16_05:
asm {
	inc cx		// CX=cantidad de pixels a dibujar
	mov bx,x0
	push cx
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	shl ah,cl	// AH=m scara desplazada
	mov al,8	// AL=n£mero registro m scara de bit
	out dx,ax
	pop cx
}
lin16_06:
asm {
	or es:[bx],al	// activa pixel
	add bx,si	// siguiente pixel
	loop lin16_06
	jmp fin16
}
linea_horz16:
asm {
	push ds
	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	mov dh,ah
	not dh
	shl dh,cl
	not dh          // DH=m scara para primer byte
	mov cx,x1
	and cl,7
	xor cl,7
	mov dl,0ffh
	shl dl,cl	// DL=m scara para £ltimo byte

	mov ax,x1
	mov bx,x0
	mov cl,3
	shr ax,cl
	shr bx,cl
	mov cx,ax
	sub cx,bx	// CX=n£mero de bytes por l¡nea-1

	mov bx,dx
	mov dx,03ceh	// DX=puerto del controlador de gr ficos
	mov al,8	// AL=n£mero registro de m scara de bit

	push es
	pop ds
	mov si,di	// DS:SI=direcci¢n en buffer de v¡deo

	or bh,bh
	js lin16_08	// salta si alineado por byte

	or cx,cx
	jnz lin16_07	// salta si hay m s de un byte en la l¡nea

	and bl,bh 	// BL=m scara de bit para la l¡nea
	jmp lin16_09
}
lin16_07:
asm {
	mov ah,bh	// AH=m scara para primer byte
	out dx,ax
	movsb		// dibuja primer byte
	dec cx
}
lin16_08:
asm {
	mov ah,0ffh	// AH=m scara de bit
	out dx,ax
	rep movsb	// dibuja resto de la l¡nea
}
lin16_09:
asm {
	mov ah,bl	// AH=m scara para £ltimo byte
	out dx,ax
	movsb		// dibuja £ltimo byte
	pop ds
	jmp fin16
}
lin16_10:
asm {
	mov ah,bl	// AH=m scara de bit para siguiente pixel
}
lin16_11:
asm {
	or ah,bl 	// posici¢n de pixel actual en m scara
	ror bl,1
	jc lin16_13     // salta si m scara est  girada a la posici¢n de
			// pixel m s a la izquierda
	or si,si
	jns lin16_12	// salta si d>=0

	add si,incr1	// d=d+incr1
	loop lin16_11

	out dx,ax
	or es:[di],al
	jmp fin16
}
lin16_12:
asm {
	add si,incr2	// d=d+incr2
	out dx,ax
	or es:[di],al
	add di,incvert	// incrementa y
	loop lin16_10
	jmp fin16
}
lin16_13:
asm {
	out dx,ax
	or es:[di],al
	inc di		// incrementa x

	or si,si
	jns lin16_14	// salta si d>=0

	add si,incr1
	loop lin16_10
	jmp fin16
}
lin16_14:
asm {
	add si,incr2
	add di,incvert
	loop lin16_10
	jmp fin16
}
lin16_15:
asm {
	mov bx,incvert
}
lin16_16:
asm {
	out dx,ax
	or es:[di],al
	add di,bx	// incrementa y
	or si,si
	jns lin16_17	// salta si d>=0

	add si,incr1	// d=d+incr1
	loop lin16_16
	jmp fin16
}
lin16_17:
asm {
	add si,incr2	// d=d+incr2
	ror ah,1
	adc di,0	// incrementa DI si la m scara est  girada a la
			// posici¢n de pixel m s a la izquieda
	loop lin16_16
}
fin16:
asm {
	xor ax,ax
	out dx,ax	// inicializa registro set/reset
	inc ax
	out dx,ax	// inicializa registro activaci¢n set/reset
	mov al,3
	out dx,ax	// inicializa registro rotar dato/seleccionar funci¢n
	mov ax,0ff08h
	out dx,ax	// inicializa registro m scara de bit

	pop ds
	pop di
	pop si
}

}

/****************************************************************************
	G_LINEA256: dibuja una l¡nea en el modo de 256 colores.
	  Entrada:	'x0', 'y0' punto de origen
			'x1', 'y1' punto final
			'color' color de la l¡nea
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_linea256(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int incr1, incr2, rutina;

asm {
	push si
	push di
	push ds

	mov si,320	// incremento inicial de y
	mov cx,x1
	sub cx,x0      	// CX=x1-x0
	jnz lin256_00
	jmp linea_vert256	// salta si es una l¡nea vertical
}
lin256_00:
asm {
	jns lin256_01  	// salta si x1>x0

	neg cx          // CX=x0-x1
	mov bx,x1
	xchg bx,x0
	mov x1,bx	// intercambia x0 y x1
	mov bx,y1
	xchg bx,y0
	mov y1,bx	// intercambia y0 e y1
}
lin256_01:
asm {
	mov bx,y1
	sub bx,y0      	// BX=y1-y0
	jnz lin256_02
	jmp linea_horz256	// salta si es una l¡nea horizontal
}
lin256_02:
asm {
	jns lin256_03   // salta si la pendiente es positiva

	neg bx          // BX=y0-y1
	neg si          // niega el incremento de y
}
lin256_03:
asm {
	push si
	mov rutina,0
	cmp bx,cx
	jle lin256_04  	// salta si dy<=dx (pendiente<=1)
	mov rutina,1
	xchg bx,cx      // intercambia dy y dx
}
lin256_04:
asm {
	shl bx,1        // BX=2*dy
	mov incr1,bx    // incr1=2*dy
	sub bx,cx
	mov si,bx       // SI=d=2*dy-dx
	sub bx,cx
	mov incr2,bx    // incr2=2*(dy-dx)

	push cx
	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo
	pop cx
	inc cx          // CX=cantidad de pixels a dibujar

	pop bx          // BX=incremento de y
	cmp rutina,1
	je pendiente1
	jmp lin256_07
}
pendiente1:
asm {
	jmp lin256_10
}
linea_vert256:
asm {
	mov ax,y0
	mov bx,y1
	mov cx,bx
	sub cx,ax    	// CX=dy
	jge lin256_05   // salta si dy>=0

	neg cx          // fuerza dy>=0
	mov ax,bx
}
lin256_05:
asm {
	inc cx          // CX=cantidad de pixels a dibujar
	mov bx,x0
	push cx
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	pop cx
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	dec si          // SI=bytes/l¡nea-1
	mov al,color    // AL=valor de pixel
}
lin256_06:
asm {
	cmp modo,G_NORM
	jne lin256_06and

	stosb

	add di,si       // siguiente l¡nea
	loop lin256_06
	jmp fin256
}
lin256_06and:
asm {
	cmp modo,G_AND
	jne lin256_06or
	and es:[di],al
	inc di
	add di,si       // siguiente l¡nea
	loop lin256_06and
	jmp fin256
}
lin256_06or:
asm {
	cmp modo,G_OR
	jne lin256_06xor
	or es:[di],al
	inc di
	add di,si       // siguiente l¡nea
	loop lin256_06or
	jmp fin256
}
lin256_06xor:
asm {
	xor es:[di],al
	inc di
	add di,si       // siguiente l¡nea
	loop lin256_06xor
	jmp fin256
}
linea_horz256:
asm {
	push cx
	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo
	pop cx
	inc cx          // CX=cantidad de pixels a dibujar
	mov al,color    // AL=valor de pixel

	cmp modo,G_NORM
	jne linea_horz256and

	rep stosb

	jmp fin256
}
linea_horz256and:
asm {
	cmp modo,G_AND
	jne linea_horz256or
}
linea_horz256andr:
asm {
	and es:[di],al
	inc di
	loop linea_horz256andr
	jmp fin256
}
linea_horz256or:
asm {
	cmp modo,G_OR
	jne linea_horz256xor
}
linea_horz256orr:
asm {
	or es:[di],al
	inc di
	loop linea_horz256orr
	jmp fin256
}
linea_horz256xor:
asm {
	xor es:[di],al
	inc di
	loop linea_horz256xor
	jmp fin256
}
lin256_07:
asm {
	mov al,color 	// AL=valor de pixel

	cmp modo,G_NORM
	je lin256_08
	jmp lin256_07and
}
lin256_08:
asm {
	stosb

	or si,si        // comprueba el signo de d
	jns lin256_09   // salta si d>=0

	add si,incr1    // d=d+incr1
	loop lin256_08
	jmp fin256
}
lin256_09:
asm {
	add si,incr2    // d=d+incr2
	add di,bx       // incrementa y
	loop lin256_08
	jmp fin256
}
lin256_07and:
asm {
	cmp modo,G_AND
	je lin256_07and1
	jmp lin256_07or
}
lin256_07and1:
asm {
	and es:[di],al
	inc di

	or si,si        // comprueba el signo de d
	jns lin256_07and2	// salta si d>=0

	add si,incr1    // d=d+incr1
	loop lin256_07and1
	jmp fin256
}
lin256_07and2:
asm {
	add si,incr2    // d=d+incr2
	add di,bx       // incrementa y
	loop lin256_07and1
	jmp fin256
}
lin256_07or:
asm {
	cmp modo,G_OR
	je lin256_07or1
	jmp lin256_07xor
}
lin256_07or1:
asm {
	or es:[di],al
	inc di

	or si,si        // comprueba el signo de d
	jns lin256_07or2	// salta si d>=0

	add si,incr1    // d=d+incr1
	loop lin256_07or1
	jmp fin256
}
lin256_07or2:
asm {
	add si,incr2    // d=d+incr2
	add di,bx       // incrementa y
	loop lin256_07or1
	jmp fin256
}
lin256_07xor:
asm {
	xor es:[di],al
	inc di

	or si,si        // comprueba el signo de d
	jns lin256_07xor2	// salta si d>=0

	add si,incr1    // d=d+incr1
	loop lin256_07xor
	jmp fin256
}
lin256_07xor2:
asm {
	add si,incr2    // d=d+incr2
	add di,bx       // incrementa y
	loop lin256_07xor
	jmp fin256
}
lin256_10:
asm {
	mov al,color    // AL=valor de pixel

	cmp modo,G_NORM
	je lin256_11
	jmp lin256_10and
}
lin256_11:
asm {
	stosb

	add di,bx       // incrementa y
	or si,si        // comprueba el signo de d
	jns lin256_12   // salta si d>=0
	add si,incr1    // d=d+incr1
	dec di          // decrementa x (ya incrementado)
	loop lin256_11
	jmp fin256
}
lin256_12:
asm {
	add si,incr2    // d=d+incr2
	loop lin256_11
}
lin256_10and:
asm {
	cmp modo,G_AND
	je lin256_10and1
	jmp lin256_10or
}
lin256_10and1:
asm {
	and es:[di],al
	inc di

	add di,bx       // incrementa y
	or si,si        // comprueba el signo de d
	jns lin256_10and2	// salta si d>=0
	add si,incr1    // d=d+incr1
	dec di          // decrementa x (ya incrementado)
	loop lin256_10and1
	jmp fin256
}
lin256_10and2:
asm {
	add si,incr2    // d=d+incr2
	loop lin256_10and1
}
lin256_10or:
asm {
	cmp modo,G_OR
	je lin256_10or1
	jmp lin256_10xor
}
lin256_10or1:
asm {
	or es:[di],al
	inc di

	add di,bx       // incrementa y
	or si,si        // comprueba el signo de d
	jns lin256_10or2	// salta si d>=0
	add si,incr1    // d=d+incr1
	dec di          // decrementa x (ya incrementado)
	loop lin256_10or1
	jmp fin256
}
lin256_10or2:
asm {
	add si,incr2    // d=d+incr2
	loop lin256_10or1
}
lin256_10xor:
asm {
	xor es:[di],al
	inc di

	add di,bx       // incrementa y
	or si,si        // comprueba el signo de d
	jns lin256_10xor2	// salta si d>=0
	add si,incr1    // d=d+incr1
	dec di          // decrementa x (ya incrementado)
	loop lin256_10xor
	jmp fin256
}
lin256_10xor2:
asm {
	add si,incr2    // d=d+incr2
	loop lin256_10xor
}
fin256:
asm {
	pop ds
	pop di
	pop si
}

}

/****************************************************************************
	G_LINEA256X: dibuja un l¡nea en el modo 360x480x256.
	  Entrada:	'x0', 'y0' punto de origen
			'x1', 'y1' punto final
			'color' color de la l¡nea
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_linea256x(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
register int x, y;
int d, dx, dy, aincr, bincr, xincr, yincr;

/* ir por el eje X o Y */
if(abs(x1-x0)<abs(y1-y0)) {	/* por el eje Y */
	if(y0>y1) {
		swapint(&x0,&x1);
		swapint(&y0,&y1);
	}
	xincr=(x1>x0) ? 1 : -1;

	dy=y1-y0;
	dx=abs(x1-x0);
	d=2*dx-dy;
	aincr=2*(dx-dy);
	bincr=2*dx;
	x=x0;
	y=y0;

	g_punto256x(x,y,color,modo);
	for(y=y0+1; y<=y1; y++) {
		if(d>=0) {
			x+=xincr;
			d+=aincr;
		}
		else d+=bincr;
		g_punto256x(x,y,color,modo);
	}
}
else {				/* por el eje X */
	if(x0>x1) {
		swapint(&x0,&x1);
		swapint(&y0,&y1);
	}

	yincr=(y1>y0) ? 1 : -1;

	dx=x1-x0;
	dy=abs(y1-y0);
	d=2*dy-dx;
	aincr=2*(dy-dx);
	bincr=2*dy;
	x=x0;
	y=y0;

	g_punto256x(x,y,color,modo);
	for(x=x0+1; x<=x1; x++) {
		if(d>=0) {
			y+=yincr;
			d+=aincr;
		}
		else d+=bincr;
		g_punto256x(x,y,color,modo);
	}
}

}

/****************************************************************************
	G_RECTANGULO16: dibuja un rect ngulo s¢lido en los modos de 16
	  colores.
	  Entrada:	'x0', 'y0' esquina superior izquierda
			'x1', 'y1' esquina inferior derecha
			'color' color del rect ngulo
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_rectangulo16(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int lineas;

asm {
	push si
	push di
	push ds

	mov dx,03ceh	// DX=puerto del controlador de gr ficos
	mov ah,color
	xor al,al	// AL=n£mero de registro set/reset
	out dx,ax

	mov ax,0f01h	// AH=m scara de plano de bit
			// AL=n£mero registro activaci¢n set/reset
	out dx,ax

	mov ah,modo	// modo de dibujo: 18h=XOR, 10h=OR, 08h=AND, 00h=NORM
	mov al,3	// AL=n£mero registro rotar dato/selecc. funci¢n
	out dx,ax

	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	mov dh,ah
	not dh
	shl dh,cl
	not dh          // DH=m scara para primer byte
	mov cx,x1
	and cl,7
	xor cl,7
	mov dl,0ffh
	shl dl,cl	// DL=m scara para £ltimo byte

	mov ax,x1
	mov bx,x0
	mov cl,3
	shr ax,cl
	shr bx,cl
	mov cx,ax
	sub cx,bx	// CX=n£mero de bytes por l¡nea-1

	mov ax,y1
	sub ax,y0
	inc ax		// BX=n£mero de l¡neas
	mov lineas,ax

	mov bx,dx
	mov dx,03ceh	// DX=puerto del controlador de gr ficos
	mov al,8	// AL=n£mero registro de m scara de bit

	push es
	pop ds
	mov si,di	// DS:SI=direcci¢n en buffer de v¡deo
}
rect16_00:
asm {
	push si
	push di
	push cx

	or bh,bh
	js rect16_02	// salta si alineado por byte

	or cx,cx
	jnz rect16_01	// salta si hay m s de un byte en la l¡nea

	and bl,bh 	// BL=m scara de bit para la l¡nea
	jmp rect16_03
}
rect16_01:
asm {
	mov ah,bh	// AH=m scara para primer byte
	out dx,ax
	movsb		// dibuja primer byte
	dec cx
}
rect16_02:
asm {
	mov ah,0ffh	// AH=m scara de bit
	out dx,ax
	rep movsb	// dibuja resto de la l¡nea
}
rect16_03:
asm {
	mov ah,bl	// AH=m scara para £ltimo byte
	out dx,ax
	movsb		// dibuja £ltimo byte

	pop cx
	pop di
	pop si

	add si,80	// siguiente l¡nea
	add di,80

	dec lineas
	jnz rect16_00

	xor ax,ax
	out dx,ax	// inicializa registro set/reset
	inc ax
	out dx,ax	// inicializa registro activaci¢n set/reset
	mov al,3
	out dx,ax	// inicializa registro rotar dato/seleccionar funci¢n
	mov ax,0ff08h
	out dx,ax	// inicializa registro m scara de bit

	pop ds
	pop di
	pop si
}

}

/****************************************************************************
	G_RECTANGULO256: dibuja un rect ngulo s¢lido en el modo de 256
	  colores.
	  Entrada:	'x0', 'y0' esquina superior izquierda
			'x1', 'y1' esquina inferior derecha
			'color' color del rect ngulo
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_rectangulo256(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{

asm {
	push si
	push di
	push ds

	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo

	mov cx,x1
	sub cx,x0
	inc cx          // CX=cantidad de pixels a dibujar

	mov bx,y1
	sub bx,y0
	inc bx		// BX=n£mero de l¡neas

	mov al,color    // AL=valor de pixel

	cmp modo,G_NORM
	jne rect256and
}
rect256norm:
asm {
	push di
	push cx

	rep stosb

	pop cx
	pop di
	add di,320	// siguiente l¡nea

	dec bx
	jnz rect256norm

	jmp finrect256
}
rect256and:
asm {
	cmp modo,G_AND
	jne rect256or
}
rect256andr0:
asm {
	push di
	push cx
}
rect256andr1:
asm {
	and es:[di],al
	inc di
	loop rect256andr1

	pop cx
	pop di
	add di,320	// siguiente l¡nea

	dec bx
	jnz rect256andr0

	jmp finrect256
}
rect256or:
asm {
	cmp modo,G_OR
	jne rect256xorr0
}
rect256orr0:
asm {
	push di
	push cx
}
rect256orr1:
asm {
	or es:[di],al
	inc di
	loop rect256orr1

	pop cx
	pop di
	add di,320	// siguiente l¡nea

	dec bx
	jnz rect256orr0

	jmp finrect256
}
rect256xorr0:
asm {
	push di
	push cx
}
rect256xorr1:
asm {
	xor es:[di],al
	inc di
	loop rect256xorr1

	pop cx
	pop di
	add di,320	// siguiente l¡nea

	dec bx
	jnz rect256xorr0
}
finrect256:
asm {
	pop ds
	pop di
	pop si
}

}

/****************************************************************************
	G_RECTANGULO256X: dibuja un rect ngulo s¢lido en el modo 360x480x256.
	  Entrada:	'x0', 'y0' esquina superior izquierda
			'x1', 'y1' esquina inferior derecha
			'color' color del rect ngulo
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_rectangulo256x(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int a, b, ancho, alto;

/* calcula dimensiones */
alto=y1-y0+1;
ancho=x1-x0+1;

if(ancho<=4) {
	asm {
		mov dx,03ceh	// puerto del controlador gr fico
		mov ah,modo
		mov al,3	// AL=registro rotar dato/seleccionar funci¢n
		out dx,ax

		mov ax,y0
		mov bx,x0
	}
		DIR_PIXEL256X;
	rect256x1_1:
	asm {
		push bx
		mov cx,x0
		and cx,3
		mov ax,1
		shl ax,cl
		mov ah,al	// AH=m scara planos de bit
		mov dx,03c4h	// puerto del secuenciador
		mov cx,ancho
	}
	rect256x1_2:
	asm {
		mov al,2
		out dx,ax	// selecciona plano

		mov al,es:[bx]	// guarda un byte desde cada plano de bits
		mov al,color
		mov es:[bx],al	// actualiza planos de bits

		cmp ah,8
		je rect256x1_3	// comprueba si hay que cambiar de byte

		shl ah,1	// siguiente plano de bits

		loop rect256x1_2
		jmp rect256x1_4
	}
	rect256x1_3:
	asm {
		inc bx
		mov ah,1	// nueva m scara de planos de bits
		loop rect256x1_2
	}
	rect256x1_4:
	asm {
		pop bx
		mov cx,90
		add bx,cx	// siguiente l¡nea

		dec alto
		jnz rect256x1_1	// salta si no se han dibujado todas las l¡neas

		mov dx,03ceh	// puerto del controlador gr fico
		mov ax,0003h	// inicializa registro seleccionar funci¢n
		out dx,ax
	}
}
else {
	a=4-(x0%4);
	ancho-=a;

	b=(x1%4)+1;
	if((b!=4) && ancho) ancho-=b;

	ancho>>=2;

	asm {
		mov dx,03ceh	// puerto del controlador gr fico
		mov ah,modo
		mov al,3	// AL=registro rotar dato/seleccionar funci¢n
		out dx,ax

		mov ax,y0
		mov bx,x0
	}
		DIR_PIXEL256X;
	rect256x2_1:
	asm {
		push bx
		mov dx,03c4h	// puerto del secuenciador

		cmp a,4
		je rect256x2_3

		mov cx,x0
		and cx,3
		mov ax,1
		shl ax,cl
		mov ah,al	// AH=m scara planos de bit
	}
	rect256x2_2:
	asm {
		mov al,2
		out dx,ax	// selecciona plano

		mov al,es:[bx]	// guarda un byte desde cada plano de bits
		mov al,color
		mov es:[bx],al	// actualiza planos de bits

		shl ah,1	// siguiente plano de bits
		cmp ah,10h
		jne rect256x2_2

		inc bx		// siguiente byte
	}
	rect256x2_3:
	asm {
		cmp ancho,0
		je rect256x2_6
		mov ah,1	// selecciona 1er plano
	}
	rect256x2_4:
	asm {
		push bx
		mov al,2
		out dx,ax

		mov cx,ancho
	}
	rect256x2_5:
	asm {
		mov al,es:[bx]	// guarda un byte desde cada plano de bits
		mov al,color
		mov es:[bx],al	// actualiza planos de bits
		inc bx
		loop rect256x2_5

		pop bx
		shl ah,1	// siguiente plano de bits
		cmp ah,10h
		jne rect256x2_4
	}
	rect256x2_6:
	asm {
		cmp b,4
		je rect256x2_8
		add bx,ancho

		mov cx,b
		mov ah,1
	}
	rect256x2_7:
	asm {
		mov al,2
		out dx,ax

		mov al,es:[bx]	// guarda un byte desde cada plano de bits
		mov al,color
		mov es:[bx],al	// actualiza planos de bits

		shl ah,1	// siguiente plano de bits
		loop rect256x2_7
	}
	rect256x2_8:
	asm {
		pop bx
		mov cx,90
		add bx,cx	// siguiente l¡nea

		dec alto
		jnz rect256x2_1	// salta si no se han dibujado todas las l¡neas

		mov dx,03ceh	// puerto del controlador gr fico
		mov ax,0003h	// inicializa registro seleccionar funci¢n
		out dx,ax
	}
}

}

/****************************************************************************
	G_PUNTO: dibuja un punto.
	  Entrada:	'x', 'y' coordenadas
			'color' color del punto
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_punto(int x, int y, unsigned char color, unsigned char modo)
{

if(_g_modovideo==G_MV_G2C256) g_punto256x(x,y,color,modo);
else if(_g_modovideo==G_MV_G1C256) g_punto256(x,y,color,modo);
else g_punto16(x,y,color,modo);

}

/****************************************************************************
	G_LINEA: dibuja una l¡nea.
	  Entrada:	'x0', 'y0' punto de origen
			'x1', 'y1' punto final
			'color' color de la l¡nea
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_linea(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{

if(_g_modovideo==G_MV_G2C256) g_linea256x(x0,y0,x1,y1,color,modo);
else if(_g_modovideo==G_MV_G1C256) g_linea256(x0,y0,x1,y1,color,modo);
else g_linea16(x0,y0,x1,y1,color,modo);

}

/****************************************************************************
	G_RECTANGULO: dibuja un rect ngulo.
	  Entrada:	'x0', 'y0' esquina superior izquierda
			'x1', 'y1' esquina inferior derecha
			'color' color del rect ngulo
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
			'relleno' 0 dibuja s¢lo borde, 1 dibuja rect ngulo
			s¢lido
****************************************************************************/
void g_rectangulo(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo, int relleno)
{

if(!relleno) {
	g_linea(x0,y0,x1,y0,color,modo);
	g_linea(x1,y0+1,x1,y1,color,modo);
	g_linea(x1-1,y1,x0,y1,color,modo);
	g_linea(x0,y1-1,x0,y0+1,color,modo);
}
else {
	if(_g_modovideo==G_MV_G2C256) g_rectangulo256x(x0,y0,x1,y1,color,modo);
	else if(_g_modovideo==G_MV_G1C256) g_rectangulo256(x0,y0,x1,y1,color,modo);
	else g_rectangulo16(x0,y0,x1,y1,color,modo);
}

}

/****************************************************************************
	G_SCROLL_ARR: realiza scroll hacia arriba de una ventana.
	  Entrada:      'fila', 'columna' inicio de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'color' color de relleno
****************************************************************************/
void g_scroll_arr(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
unsigned char far *pvideo;
unsigned char far *pvid2;

if(_g_modovideo==G_MV_G1C256) {
	/* direcci¢n en buffer de v¡deo de zona a desplazar */
	pvideo=(unsigned char far *)0xa0000000L+((fila*320*8)+(columna*8));
	/* puntero auxiliar a l¡nea siguiente */
	pvid2=pvideo+(320*8);
	ancho*=8;
	alto=(alto-1)*8;

	asm {
		push di
		push si
		push ds

		les di,pvideo           // direcci¢n de inicio
		lds si,pvid2            // puntero a l¡nea siguiente

		mov cx,alto             // n£mero de l¡neas a desplazar
	}
	scr_arr256:
	asm {
		push cx
		push di
		push si
		mov cx,ancho            // transfiere l¡nea siguiente a actual
		rep movsb
		pop si
		pop di
		pop cx

		add di,320              // siguiente l¡nea
		add si,320

		loop scr_arr256         // repite hasta desplazar todo bloque

		mov al,color   		// AL=color de relleno
		mov cx,8          	// CX=n£mero de l¡neas a rellenar
	}
	scr_arr_rell256:
	asm {
		push cx
		push di
		mov cx,ancho
		rep stosb               // rellena l¡nea con 'color'
		pop di
		pop cx

		add di,320              // siguiente l¡nea
		loop scr_arr_rell256

		pop ds
		pop si
		pop di
	}
}
else if(_g_modovideo==G_MV_G2C256) {
	/* direcci¢n en buffer de v¡deo de zona a desplazar */
	pvideo=(unsigned char far *)0xa0000000L+((fila*90*16)+(columna*2));
	/* puntero auxiliar a l¡nea siguiente */
	pvid2=pvideo+(90*16);
	ancho*=2;
	alto=(alto-1)*16;

	asm {
		push di
		push si
		push ds

		mov dx,03c4h		// DX=puerto del secuenciador
		mov ax,0f02h		// seleccionar todos los planos
		out dx,ax

		mov dx,03ceh            // DX=puerto del controlador gr fico
		mov ax,0008h            // reg. m scara de bit=00h
		out dx,ax

		les di,pvideo           // direcci¢n de inicio
		lds si,pvid2            // puntero a l¡nea siguiente

		mov cx,alto             // n£mero de l¡neas a desplazar
	}
	scr_arr256x:
	asm {
		push cx
		push di
		push si
		mov cx,ancho            // transfiere l¡nea siguiente a actual
		rep movsb
		pop si
		pop di
		pop cx

		add di,90               // siguiente l¡nea
		add si,90

		loop scr_arr256x        // repite hasta desplazar todo bloque

		mov ax,0ff08h           // reg. m scara de bit=ffh
		out dx,ax

		mov al,color		// AL=color de relleno
		mov cx,16          	// CX=n£mero de l¡neas a rellenar
	}
	scr_arr_rell256x:
	asm {
		push cx
		push di
		mov cx,ancho
		rep stosb               // rellena l¡nea con 'color'
		pop di
		pop cx

		add di,90               // siguiente l¡nea
		loop scr_arr_rell256x

		pop ds
		pop si
		pop di
	}
}
else {
	/* direcci¢n en buffer de v¡deo de zona a desplazar */
	pvideo=(unsigned char far *)0xa0000000L+((fila*80*16)+columna);
	/* puntero auxiliar a l¡nea siguiente */
	pvid2=pvideo+(80*16);
	alto=(alto-1)*16;

	asm {
		push di
		push si
		push ds

		mov dx,03ceh            // DX=puerto del controlador gr fico
		mov ax,0008h            // reg. m scara de bit=00h
		out dx,ax

		les di,pvideo           // direcci¢n de inicio
		lds si,pvid2            // puntero a l¡nea siguiente

		mov cx,alto             // n£mero de l¡neas a desplazar
	}
	scr_arr16:
	asm {
		push cx
		push di
		push si
		mov cx,ancho            // transfiere l¡nea siguiente a actual
		rep movsb
		pop si
		pop di
		pop cx

		add di,80               // siguiente l¡nea
		add si,80

		loop scr_arr16          // repite hasta desplazar todo bloque

		mov ah,color   		// AH=color de relleno
		mov al,0                // AL=registro set/reset
		out dx,ax
		mov ax,0f01h            // reg. activaci¢n set/reset=0fh
		out dx,ax
		mov ax,0ff08h           // reg. m scara de bit=ffh
		out dx,ax

		mov cx,16          	// CX=n£mero de l¡neas a rellenar
	}
	scr_arr_rell16:
	asm {
		push cx
		push di
		mov cx,ancho
		rep stosb               // rellena l¡nea con 'color'
		pop di
		pop cx

		add di,80               // siguiente l¡nea
		loop scr_arr_rell16

		mov ax,0001h            // restaura reg. activaci¢n set/reset
		out dx,ax

		pop ds
		pop si
		pop di
	}
}

}

/****************************************************************************
	G_SCROLL_ABJ: realiza scroll hacia abajo de una ventana.
	  Entrada:      'fila', 'columna' inicio de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'color' color de relleno
****************************************************************************/
void g_scroll_abj(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
unsigned char far *pvideo;
unsigned char far *pvid2;

if(_g_modovideo==G_MV_G1C256) {
	/* direcci¢n en buffer de v¡deo de final de zona a desplazar */
	pvideo=(unsigned char far *)0xa0000000L+(((((fila+alto)*8)-1)*320)+
	  (columna*8));
	/* puntero auxiliar a l¡nea anterior */
	pvid2=pvideo-(320*8);
	ancho*=8;
	alto=(alto-1)*8;

	asm {
		push di
		push si
		push ds

		les di,pvideo           // direcci¢n de inicio
		lds si,pvid2            // puntero a l¡nea anterior

		mov cx,alto		// n£mero de l¡neas a desplazar
	}
	scr_abj256:
	asm {
		push cx
		push di
		push si
		mov cx,ancho            // transfiere l¡nea anterior a actual
		rep movsb
		pop si
		pop di
		pop cx

		sub di,320              // l¡nea anterior
		sub si,320

		loop scr_abj256         // repite hasta desplazar todo bloque

		sub di,2240		// ES:DI=dir. origen primera l¡nea
					// 2240=320*(8-1)

		mov al,color   		// AL=color de relleno
		mov cx,8          	// CX=n£mero de l¡neas a rellenar
	}
	scr_abj_rell256:
	asm {
		push cx
		push di
		mov cx,ancho
		rep stosb               // rellena l¡nea con 'color'
		pop di
		pop cx

		add di,320              // siguiente l¡nea
		loop scr_abj_rell256

		pop ds
		pop si
		pop di
	}
}
else if(_g_modovideo==G_MV_G2C256) {
	/* direcci¢n en buffer de v¡deo de final de zona a desplazar */
	pvideo=(unsigned char far *)0xa0000000L+(((((fila+alto)*16)-1)*90)+
	  (columna*2));
	/* puntero auxiliar a l¡nea anterior */
	pvid2=pvideo-(90*16);
	ancho*=2;
	alto=(alto-1)*16;

	asm {
		push di
		push si
		push ds

		mov dx,03c4h		// DX=puerto del secuenciador
		mov ax,0f02h		// activa todos los planos
		out dx,ax

		mov dx,03ceh            // DX=puerto del controlador gr fico
		mov ax,0008h            // reg. m scara de bit=00h
		out dx,ax

		les di,pvideo           // direcci¢n de inicio
		lds si,pvid2            // puntero a l¡nea anterior

		mov cx,alto		// n£mero de l¡neas a desplazar
	}
	scr_abj256x:
	asm {
		push cx
		push di
		push si
		mov cx,ancho            // transfiere l¡nea anterior a actual
		rep movsb
		pop si
		pop di
		pop cx

		sub di,90              	// l¡nea anterior
		sub si,90

		loop scr_abj256x        // repite hasta desplazar todo bloque

		sub di,1350		// ES:DI=dir. origen primera l¡nea
					// 1350=90*(16-1)

		mov ax,0ff08h           // reg. m scara de bit=ffh
		out dx,ax

		mov al,color		// AL=color de relleno
		mov cx,16          	// CX=n£mero de l¡neas a rellenar
	}
	scr_abj_rell256x:
	asm {
		push cx
		push di
		mov cx,ancho
		rep stosb               // rellena l¡nea con 'color'
		pop di
		pop cx

		add di,90              // siguiente l¡nea
		loop scr_abj_rell256x

		pop ds
		pop si
		pop di
	}
}
else {
	/* direcci¢n en buffer de v¡deo de final de zona a desplazar */
	pvideo=(unsigned char far *)0xa0000000L+(((((fila+alto)*16)-1)*80)+
	  columna);
	/* puntero auxiliar a l¡nea anterior */
	pvid2=pvideo-(80*16);
	alto=(alto-1)*16;

	asm {
		push di
		push si
		push ds

		mov dx,03ceh            // DX=puerto del controlador gr fico
		mov ax,0008h            // reg. m scara de bit=00h
		out dx,ax

		les di,pvideo           // direcci¢n de inicio
		lds si,pvid2            // puntero a l¡nea anterior

		mov cx,alto             // n£mero de l¡neas a desplazar
	}
	scr_abj16:
	asm {
		push cx
		push di
		push si
		mov cx,ancho            // transfiere l¡nea anterior a actual
		rep movsb
		pop si
		pop di
		pop cx

		sub di,80               // l¡nea anterior
		sub si,80

		loop scr_abj16          // repite hasta desplazar todo bloque

		sub di,1200		// ES:DI=dir. origen primera l¡nea
					// 1200=80*(16-1)

		mov ah,color   		// AH=color de relleno
		mov al,0                // AL=registro set/reset
		out dx,ax
		mov ax,0f01h            // reg. activaci¢n set/reset=0fh
		out dx,ax
		mov ax,0ff08h           // reg. m scara de bit=ffh
		out dx,ax

		mov cx,16          	// CX=n£mero de l¡neas a rellenar
	}
	scr_abj_rell16:
	asm {
		push cx
		push di
		mov cx,ancho
		rep stosb               // rellena l¡nea con 'color'
		pop di
		pop cx

		add di,80               // siguiente l¡nea
		loop scr_abj_rell16

		mov ax,0001h            // restaura reg. activaci¢n set/reset
		out dx,ax

		pop ds
		pop si
		pop di
	}
}

}
