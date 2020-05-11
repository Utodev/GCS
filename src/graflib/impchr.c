/****************************************************************************
				   IMPCHR.C

	Biblioteca de funciones para imprimir textos en los modos gr ficos de
	16 y 256 colores.

			    (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- chr_altura: devuelve la altura de los caracteres
		- chr_anchura: devuelve la anchura de un car cter
		- chr_maxfil: devuelve n£mero de filas de pantalla
		- chr_maxcol: devuelbe n£mero de columnas de pantalla
		- imp_chr_pos: coloca posici¢n de impresi¢n
		- imp_chr: imprime un car cter en una posici¢n de pantalla
		  dada
		- def_chr_set: actualiza punteros a un nuevo 'set' de
		  caracteres
****************************************************************************/

#include <stddef.h>
#include "grf.h"
#include "impchr.h"
#include "defchr.h"

/*** Variables externas **/
extern int _g_modovideo;

/*** Variables globales internas ***/
/* punteros a tablas con las definiciones de caracteres de 8xCHR_ALT y 8x8 */
static unsigned char *_defchr=&_defchr_sys[0][0];
static unsigned char *_defchr8x8=&_defchr8x8_sys[0][0];

/* punteros a tablas con las anchuras de caracteres de 8xCHR_ALT y 8x8 */
static unsigned char *_anchchr=&_anchchr_sys[0];
static unsigned char *_anchchr8x8=&_anchchr8x8_sys[0];

/* £ltimas coordenadas de impresi¢n */
static int ult_x=0;
static int ult_y=0;

/****************************************************************************
	CHR_ALTURA: devuelve la altura de los caracteres.
	  Salida:	altura (en pixels) de los caracteres
****************************************************************************/
int chr_altura(void)
{

if(_g_modovideo==G_MV_G1C256) return(8);
else return(CHR_ALT);

}

/****************************************************************************
	CHR_ANCHURA: devuelve la anchura de un car cter.
	  Entrada:	'chr' car cter
	  Salida:	anchura (en pixels) del car cter
****************************************************************************/
int chr_anchura(unsigned char chr)
{

if(_g_modovideo==G_MV_G1C256) return(_anchchr8x8[chr]);
else return(_anchchr[chr]);

}

/****************************************************************************
	CHR_MAXFIL: devuelve n£mero de filas de pantalla.
	  Salida:	n£mero de filas de caracteres de pantalla, 0 si modo
			de pantalla no v lido
****************************************************************************/
int chr_maxfil(void)
{

switch(_g_modovideo) {
	case G_MV_G1C16 :
		return(12);
	case G_MV_G2C16 :
		return(21);
	case G_MV_G3C16 :
		return(30);
	case G_MV_G1C256 :
		return(25);
	case G_MV_G2C256 :
		return(30);
}

return(0);
}

/****************************************************************************
	CHR_MAXCOL: devuelve n£mero de columnas de pantalla.
	  Salida:	n£mero de columnas de caracteres de pantalla, 0 si
 			modo de pantalla no v lido
****************************************************************************/
int chr_maxcol(void)
{

switch(_g_modovideo) {
	case G_MV_G1C16 :
		return(80);
	case G_MV_G2C16 :
		return(80);
	case G_MV_G3C16 :
		return(80);
	case G_MV_G1C256 :
		return(40);
	case G_MV_G2C256 :
		return(45);
}

return(0);
}

/****************************************************************************
	IMP_CHR_POS: coloca la posici¢n de impresi¢n de textos.
	  Entrada:      'x', 'y' posici¢n de impresi¢n
	  Salida:       posici¢n de dibujo actualizada
****************************************************************************/
void imp_chr_pos(int x, int y)
{

ult_x=x;
ult_y=y;

}

/****************************************************************************
	IMP_CHR: imprime un car cter en una posici¢n de pantalla.
	  Entrada:      'chr' car cter a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'modo' modo de dibujo (CHR_NORM, CHR_XOR, CHR_OR,
			CHR_AND)
	  Salida:       anchura del car cter impreso
			posici¢n de impresi¢n actualizada
****************************************************************************/
int imp_chr(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo)
{
unsigned char far *pchr;
unsigned char masc, masci, mascd;
int x, y, maxx, maxy, alt, anch, varshift;

/* coordenadas de impresi¢n */
x=ult_x;
y=ult_y;

/* calcula par metros seg£n modo de v¡deo */
switch(_g_modovideo) {
	case G_MV_G1C16 :	/* 640x200x16 */
		maxx=639;
		maxy=199;
		alt=CHR_ALT;
		anch=_anchchr[chr];
		break;
	case G_MV_G2C16 :	/* 640x350x16 */
		maxx=639;
		maxy=349;
		alt=CHR_ALT;
		anch=_anchchr[chr];
		break;
	case G_MV_G3C16 :	/* 640x480x16 */
		maxx=639;
		maxy=479;
		alt=CHR_ALT;
		anch=_anchchr[chr];
		break;
	case G_MV_G1C256 :	/* 320x200x256 */
		maxx=319;
		maxy=199;
		alt=8;
		anch=_anchchr8x8[chr];
		break;
	case G_MV_G2C256 :	/* 360x480x256 */
		maxx=359;
		maxy=479;
		alt=CHR_ALT;
		anch=_anchchr[chr];
		break;
	default :
		return(0);
}

/* si car cter no cabe, lo pasa a siguiente l¡nea */
if((x+anch-1)>maxx) {
	x=0;
	y+=alt;
	if((y+alt-1)>maxy) y=0;
}

/* m scara de car cter */
masc=0xff>>anch;

/* modo de 256 colores */
if(_g_modovideo==G_MV_G1C256) {
	pchr=_defchr8x8+(chr*8);

	asm {
		push di
		push si
		push ds

		mov bx,x
		mov ax,y
		xchg ah,al
		add bx,ax
		shr ax,1
		shr ax,1
		add bx,ax
		mov ax,0a000h
		mov es,ax
		mov di,bx		// ES:DI=dir. en buffer de v¡deo
		lds si,pchr             // DS:SI=dir. definici¢n car cter
		mov cx,alt              // CX=altura en pixels del car cter
		mov bl,color   		// BL=valor pixel de imagen
		mov bh,colorf  		// BH=valor pixel de fondo
	}
	ic256_1:
	asm {
		push cx
		mov cx,8                // CX=anchura car cter en pixels
		lodsb                   // AL=byte de car cter
		mov ah,al               // AH=patr¢n de bit para sgte. fila
		mov dl,masc		// m scara car cter
	}
	ic256_2:
	asm {
		mov al,bl               // AL=valor de pixel imagen
		shl ah,1                // CARRY=bit alto
		jc ic_xor               // salta si es pixel de imagen
		mov al,bh               // AL=valor de pixel fondo
	}
	ic_xor:
	asm {
		cmp modo,CHR_XOR
		jne ic_or
		xor es:[di],al
		inc di
		loop ic256_2
		jmp sgte_linea
	}
	ic_or:
	asm {
		cmp modo,CHR_OR
		jne ic_and
		or es:[di],al
		inc di
		loop ic256_2
		jmp sgte_linea
	}
	ic_and:
	asm {
		cmp modo,CHR_AND
		jne ic_norm
		and es:[di],al
		inc di
		loop ic256_2
		jmp sgte_linea
	}
	ic_norm:
	asm {
		shl dl,1		// CARRY=bit alto
		jc no_imp
		stosb
		loop ic256_2
		jmp sgte_linea
	}
	no_imp:
	asm {
		inc di
		loop ic256_2
	}
	sgte_linea:
	asm {
		add di,312              // siguiente l¡nea en pantalla
		pop cx
		loop ic256_1

		pop ds
		pop si
		pop di
	}
}
/* modo de 256 colores extendido */
else if(_g_modovideo==G_MV_G2C256) {
	/* direcci¢n de car cter */
	pchr=_defchr+(chr*CHR_ALT);

	asm {
		push di
		push si
		push ds

		mov dx,03ceh	// puerto del registro de direcciones
		mov ah,modo
		mov al,3	// AL=registro rotar dato/seleccionar funci¢n
		out dx,ax	// inicializa registro seleccionar funci¢n

		mov ax,y
		mov dx,90
		mul dx		// AX=90*y
		mov bx,x
		shr bx,1
		shr bx,1 	// BX=x/4
		add bx,ax	// BX=(90*y)+(x/4)
		mov ax,0a000h
		mov es,ax
		mov di,bx	// ES:DI=dir. en buffer de v¡deo
		lds si,pchr     // DS:SI=dir. definici¢n car cter

		mov cx,x
		and cx,3
		mov ax,1
		shl ax,cl
		mov ah,al	// AH=m scara planos de bit

		mov cx,alt      // CX=altura en pixels del car cter
		mov bl,color   	// BL=valor pixel de imagen
		mov bh,colorf  	// BH=valor pixel de fondo
	}
	ic256x_1:
	asm {
		push cx
		push ax
		mov cl,8        // CL=anchura car cter en pixels
		lodsb           // AL=byte de car cter
		mov dh,al       // DH=patr¢n de bit para sgte. fila
		mov dl,masc	// m scara car cter
	}
	ic256x_2:
	asm {
		push ax
		push dx
		mov dx,03c4h	// puerto del secuenciador
		mov al,2
		out dx,ax	// selecciona plano
		pop dx
		pop ax

		mov al,bl       // AL=valor de pixel imagen
		shl dh,1        // CARRY=bit alto
		jc ic256x_3     // salta si es pixel de imagen
		mov al,bh       // AL=valor de pixel fondo
	}
	ic256x_3:
	asm {
		shl dl,1	// CARRY=bit alto
		jc no_imp256x

		mov ch,es:[di]	// actualiza 'latches'
		mov es:[di],al
	}
	no_imp256x:
	asm {
		cmp ah,8	// comprueba si hay que cambiar de byte
		je ic256x_4
		shl ah,1	// siguiente plano de bits
		dec cl
		jnz ic256x_2
		jmp sgte_lin256x
	}
	ic256x_4:
	asm {
		mov ah,1
		inc di
		dec cl
		jnz ic256x_2
	}
	sgte_lin256x:
	asm {
		add di,88     	// siguiente l¡nea en pantalla
		pop ax
		pop cx
		loop ic256x_1

		mov dx,03ceh	// puerto del registro de direcciones
		mov ax,0003h	// inicializa registro seleccionar funci¢n
		out dx,ax

		pop ds
		pop si
		pop di
	}
}
/* modos de 16 colores */
else {
	pchr=_defchr+(chr*CHR_ALT);
	/* invierte m scara */
	masc^=0xff;

	asm {
		push di
		push si
		push ds

		mov bx,x
		mov ax,y
		mov cl,bl
		mov dx,80
		mul dx
		shr bx,1
		shr bx,1
		shr bx,1
		add bx,ax
		mov ax,0a000h
		mov ds,ax
		mov si,bx		// DS:SI=dir. en buffer de v¡deo

		and cl,7
		xor cl,7       		// CL=n§ de bits a desplazar izq.
		inc cx
		and cl,7		// CL=n§ bits a despl. para enmascarar
		mov ch,0ffh
		shl ch,cl		// CH=m scara lado derecho de car cter
		mov varshift,cx

		les di,pchr             // ES:DI=dir. definici¢n de car cter
		mov cx,alt              // CX=altura en pixels del car cter

		// establecer registros del controlador de gr ficos

		mov dx,3ceh             // puerto del registro de direcciones
		mov ax,0a05h            // modo escritura 2, modo lectura 1
		out dx,ax
		mov ah,modo             // modo de dibujo: 18h=XOR, 10h=OR,
					// 08h=AND, 00h=NORM
		mov al,3
		out dx,ax
		mov ax,0007h            // color "don't care" todos los planos
		out dx,ax

		mov bl,color   		// BL=valor de pixel imagen
		mov bh,colorf  		// BH=valor de pixel fondo

		cmp byte ptr varshift,0
		jne chr_no_alin         // salta si car cter no alineado

		// rutina para caracteres alineados

		mov al,8                // AL=n§ registro de m scara de bit
	}
	chr_alin:
	asm {
		mov ah,es:[di]          // AH=patr¢n de fila de pixels
		and ah,masc		// enmascara
		out dx,ax               // lo carga en registro m scara de bit
		and [si],bl             // actualiza pixels de imagen
		not ah
		and ah,masc		// enmascara
		out dx,ax
		and [si],bh             // actualiza pixels de fondo

		inc di                  // siguiente byte del car cter
		add si,80               // siguiente l¡nea de pantalla

		loop chr_alin
		jmp chr_fin

		// rutina para caracteres no alineados
	}
	chr_no_alin:
	asm {
		push cx                 // guarda contador n§ de l¡neas de car.
		mov cx,varshift         // CH=m scara lado derecho
					// CL=n§ de bits a desplazar
		mov al,masc		// AL=m scara de car cter
		xor ah,ah
		shl ax,cl		// AH = izquierdo, AL = derecho
		mov masci,ah
		mov mascd,al

		// dibuja lado izquierdo del car cter
		mov al,es:[di]          // AL=patr¢n de fila de pixels
		xor ah,ah
		shl ax,cl               // AH=patr¢n para lado izquierdo
					// AL=patr¢n para lado derecho
		push ax                 // guarda patrones
		mov al,8                // n§ de registro de m scara de bit
		and ah,masci		// enmascara
		out dx,ax               // carga patr¢n de lado izquierdo
		and [si],bl             // actualiza pixels de imagen
		not ch                  // CH=m scara para lado izquierdo
		xor ah,ch
		and ah,masci		// enmascara
		out dx,ax
		and [si],bh             // actualiza pixels de fondo
		// dibuja lado derecho del car cter
		pop ax                  // recupera patrones
		mov ah,al               // AH=patr¢n lado derecho
		mov al,8                // AL=n§ registro de m scara de bit
		and ah,mascd		// enmascara
		out dx,ax               // carga patr¢n
		inc si                  // posici¢n en buffer de v¡deo
		and [si],bl             // actualiza pixels de imagen
		not ch                  // CH=m scara para lado derecho
		xor ah,ch
		and ah,mascd		// enmascara
		out dx,ax
		and [si],bh             // actualiza pixels de fondo

		inc di                  // siguiente byte del car cter
		dec si
		add si,80               // siguiente l¡nea de pantalla

		pop cx
		loop chr_no_alin

		// restaura registros de controlador a sus valores por defecto
	}
	chr_fin:
	asm {
		mov ax,0ff08h           // m scara de bit
		out dx,ax
		mov ax,0005h            // registro de modo
		out dx,ax
		mov ax,0003h            // rotar dato/selecc. funci¢n
		out dx,ax
		mov ax,0f07h            // color "don't care"
		out dx,ax

		pop ds
		pop si
		pop di
	}
}

/* actualiza coordenada X */
x+=anch;
if(x>maxx) {
	x=0;
	y+=alt;
}
/* actualiza coordenada Y, si se sale de pantalla pasa a (0,0) */
if((y+alt-1)>maxy) {
	x=0;
	y=0;
}

ult_x=x;
ult_y=y;

return(anch);
}

/****************************************************************************
	DEF_CHR_SET: actualiza los punteros a las definiciones de los
	  juegos de caracteres.
	  Entrada:      'ptr_set1' puntero a juego de caracteres de
			8xCHR_ALT; si es NULL, se apuntar  al juego de
			caracteres por defecto
			'ptr_set2' puntero a juego de caracteres de 8x8, si
			es NULL, se apuntar  al juego de caracteres por
			defecto
			'ptr_anch1' puntero a tabla de anchuras del juego
			de caracteres de 8xCHR_ALT; si es NULL, se apuntar 
			a tabla por defecto
			'ptr_anch2' puntero a tabla de anchuras del juego
			de caracteres de 8x8; si es NULL, se apuntar  a
			tabla por defecto
****************************************************************************/
void def_chr_set(unsigned char *ptr_set1, unsigned char *ptr_set2,
  unsigned char *ptr_anch1, unsigned char *ptr_anch2)
{

/* actualiza puntero a juego de caracteres de 8xCHR_ALT */
if(ptr_set1==NULL) _defchr=&_defchr_sys[0][0];
else _defchr=ptr_set1;

/* actualiza puntero a juego de caracteres de 8x8 */
if(ptr_set2==NULL) _defchr8x8=&_defchr8x8_sys[0][0];
else _defchr8x8=ptr_set2;

/* actualiza puntero a anchuras de caracteres de 8xCHR_ALT */
if(ptr_anch1==NULL) _anchchr=&_anchchr_sys[0];
else _anchchr=ptr_anch1;

/* actualiza puntero a anchuras de caracteres de 8x8 */
if(ptr_anch2==NULL) _anchchr8x8=&_anchchr8x8_sys[0];
else _anchchr8x8=ptr_anch2;

}
