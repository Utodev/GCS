/****************************************************************************
				     PCX.C

	Conjunto de funciones para decodificar y visualizar ficheros
 	gr ficos en formato PCX.

			    (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- pcx_decodifica: decodifica una imagen PCX
		- pcx_libera_mem: libera memoria ocupada por imagen
		- pcx_dibuja: dibuja una imagen PCX
		- pcx_visualiza: decodifica una imagen PCX y la va dibujando

	Las siguientes estructuras est n definidas en PCX.H:
		STC_CABPCX: cabecera de un fichero PCX
		STC_IMAGEN: imagen decodificada
****************************************************************************/

#include <stdio.h>
#include <alloc.h>
#include <mem.h>
#include "grf.h"
#include "pcx.h"

/*** Variables externas **/
extern int _g_modovideo;

/*** Variables globales internas ***/
static BYTE lin_scan[1024];
static BYTE color[256][3];

/*** Prototipos de funciones internas ***/
static void carga_paleta(BYTE far *nueva_paleta, int num_colores);
static void dibuja_lin16(BYTE far *lin, BYTE far *dirvideo, int anchura,
  int bytes_plano);
static void dibuja_lin256x(BYTE far *lin, BYTE far *dirvideo, int anchura,
  int masc_pln);

/****************************************************************************
	CARGA_PALETA: carga la paleta con colores dados.
	  Entrada:      'nueva_paleta' puntero a una tabla con los nuevos
			colores; cada entrada de la tabla contiene los 3
			componentes de color (rojo, verde y azul en este
			orden)
			'num_colores' n£mero de colores en la paleta
****************************************************************************/
void carga_paleta(BYTE far *nueva_paleta, int num_colores)
{

asm {
	mov ax,1012h            // actualizar bloque de registros del DAC
	mov bx,0                // primer registro
	mov cx,num_colores      // n£mero de registros a actualizar
	les dx,nueva_paleta     // direcci¢n tabla rojo-verde-azul
	int 10h
}

}

/****************************************************************************
	DIBUJA_LIN16: dibuja una l¡nea compuesta de 4 planos, en el modo de
	  16 colores.
	  Entrada:      'lin' puntero a l¡nea (los 4 planos deben estar
			seguidos en orden)
			'dirvideo' puntero a memoria de video donde se
			transferir  la l¡nea
			'anchura' porci¢n de la l¡nea que se dibujar 
			'bytes_plano' bytes por plano de la l¡nea
****************************************************************************/
void dibuja_lin16(BYTE far *lin, BYTE far *dirvideo, int anchura,
  int bytes_plano)
{

asm {
	push di
	push si
	push ds

	mov dx,03ceh            // DX=puerto controlador de gr ficos
	mov ax,0005h            // modo lectura 0, escritura 0
	out dx,ax
	mov ax,0003h            // selecciona funci¢n 'Replace'
	out dx,ax
	mov ax,0ff08h           // M scara de bit=0FFH
	out dx,ax
	mov dl,0c4h             // DX=puerto de secuenciador (03C4H)

	les di,dirvideo         // ES:DI=direcci¢n v¡deo

	mov ax,0102h            // AH=0001B, selecciona 1er plano
	lds si,lin              // DS:SI=direcci¢n buffer de l¡nea
}
planobit:
asm {
	out dx,ax               // selecciona plano de bits
	push di
	push si
	mov cx,anchura          // CX=n£mero de bytes a transferir
	rep movsb               // transfiere datos a plano activo
	pop si
	pop di
	add si,bytes_plano      // siguiente plano en buffer de l¡nea
	shl ah,1                // siguiente plano de bits
	cmp ah,16
	jnz planobit            // bucle a trav‚s de los planos de bits

	// aqu¡ DX debe valer 03C4H
	mov ax,0f02h            // valor por defecto de M scara de mapa
	out dx,ax

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	DIBUJA_LIN256X: dibuja una l¡nea en el modo de extendido de 256
	  colores.
	  Entrada:      'lin' puntero a l¡nea
			'dirvideo' puntero a memoria de video donde se
			transferir  la l¡nea
			'anchura' porci¢n de la l¡nea que se dibujar 
 			'masc_pln' m scara de planos de bit
****************************************************************************/
void dibuja_lin256x(BYTE far *lin, BYTE far *dirvideo, int anchura,
  int masc_pln)
{

asm {
	push di
	push si
	push ds

	mov dx,03ceh            // DX=puerto controlador de gr ficos
	mov ax,0003h            // selecciona funci¢n 'Replace'
	out dx,ax

	les di,dirvideo		// ES:DI=direcci¢n en buffer de v¡deo
	lds si,lin      	// DS:SI=direcci¢n de l¡nea
	mov cx,anchura		// anchura de la l¡nea

	mov dx,03c4h		// puerto del secuenciador
	mov ah,byte ptr masc_pln
	mov bx,0
}
sgte_byte1:
asm {
	mov al,2
	out dx,ax		// selecciona plano
	push bx
	push di
}
sgte_byte2:
asm {
       	//mov al,es:[di]	actualiza 'latches'
	mov al,ds:[si+bx]	// AL=byte de la l¡nea
	stosb

	add bx,4
	cmp bx,cx
	jb sgte_byte2

	pop di
	pop bx
	inc bx
	cmp bx,4		// comprueba si fin de l¡nea
	je fin_linea

	cmp ah,8        	// comprueba si est  en £ltimo plano
	je primer_plano
	shl ah,1		// siguiente plano de bits
	jmp sgte_byte1
}
primer_plano:
asm {
	inc di
	mov ah,1
	jmp sgte_byte1
}
fin_linea:
asm {
	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	PCX_DECODIFICA: decodifica una imagen PCX.
	  Entrada:      'nombre_pcx' nombre del fichero PCX
			'imagen' puntero a estructura para la imagen
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_APER - error de apertura de fichero PCX
			  E_PCX_LECT - error de lectura de fichero PCX
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MEM  - no hay memoria suficiente
****************************************************************************/
int pcx_decodifica(char *nombre_pcx, STC_IMAGEN *imagen)
{
FILE *fichpcx;
fpos_t fpos;
STC_CABPCX pcx;
int i, j, bpcx, num_colores, ancho, alto, tam_linea, bytes_decodif, cuenta,
  totbytes;
BYTE far *plin;

if((fichpcx=fopen(nombre_pcx,"rb"))==NULL) return(E_PCX_APER);

/* lee la cabecera */
if(fread(&pcx,sizeof(STC_CABPCX),1,fichpcx)!=1) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* comprueba que sea formato PCX */
if(pcx.propietario!=0x0a) {
	fclose(fichpcx);
	return(E_PCX_FORM);
}

/* n£mero de colores de la imagen */
num_colores=1 << (pcx.bits_pixel*pcx.num_planos);
imagen->num_colores=num_colores;

/* recoge informaci¢n de paleta */
if(num_colores<=16) {
	for(i=0; i<num_colores; i++) {
		imagen->paleta[i][0]=(BYTE)(pcx.paleta[i][0] >> 2);
		imagen->paleta[i][1]=(BYTE)(pcx.paleta[i][1] >> 2);
		imagen->paleta[i][2]=(BYTE)(pcx.paleta[i][2] >> 2);
	}
}
else {
	/* guarda el puntero del fichero */
	if(fgetpos(fichpcx,&fpos)) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}

	/* pone puntero del fichero al comienzo de informaci¢n de paleta */
	if(fseek(fichpcx,-769L,SEEK_END)) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}

	/* lee 1er byte de informaci¢n de cabecera y comprueba que sea 12 */
	if((bpcx=fgetc(fichpcx))==EOF) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
	if(bpcx!=12) {
		fclose(fichpcx);
		return(E_PCX_FORM);
	}

	/* lee y convierte la informaci¢n de paleta */
	for(i=0; i<num_colores; i++) {
		/* componente rojo */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}
		imagen->paleta[i][0]=(BYTE)(bpcx >> 2);
		/* componente verde */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}
		imagen->paleta[i][1]=(BYTE)(bpcx >> 2);
		/* componente azul */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}
		imagen->paleta[i][2]=(BYTE)(bpcx >> 2);
	}

	/* recupera el puntero del fichero */
	if(fsetpos(fichpcx,&fpos)) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
}

/* tama¤o de la imagen */
ancho=pcx.ventana[2]-pcx.ventana[0]+1;
alto=pcx.ventana[3]-pcx.ventana[1]+1;
imagen->ancho=ancho;
imagen->alto=alto;

/* reserva memoria para tabla de l¡neas */
imagen->lineas=NULL;

if((imagen->lineas=(BYTE far **)malloc(alto*sizeof(BYTE far *)))==NULL) {
	fclose(fichpcx);
	return(E_PCX_MEM);
}

/* tama¤o de una l¡nea (con todos sus planos) en bytes */
tam_linea=(ancho*pcx.bits_pixel*pcx.num_planos)/8;

/* reserva memoria para las l¡neas */
for(i=0; i<alto; i++) {
	if((imagen->lineas[i]=(BYTE far *)farmalloc(tam_linea))==NULL) {
		for(j=0; j<i; j++) farfree(imagen->lineas[i]);
		free(imagen->lineas);
		imagen->lineas=NULL;
		fclose(fichpcx);
		return(E_PCX_MEM);
	}
}

/* n£mero de bytes por l¡nea */
totbytes=pcx.bytes_scan*pcx.num_planos;

/* decodifica imagen */
for(i=0; i<alto; i++) {
	/* inicializa puntero a posici¢n de buffer de l¡nea */
	/* d¢nde se colocar  l¡nea de scan */
	plin=imagen->lineas[i];

	/* inicializa contador de bytes decodificados */
	bytes_decodif=0;

	/* bucle para decodificar una l¡nea de scan */
	do {
		/* lee 1 byte del fichero PCX */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			pcx_libera_mem(imagen);
			return(E_PCX_LECT);
		}

		/* si los 2 bits altos est n a 1, recoge los 6 bits bajos */
		/* y lee el siguiente byte */
		if((bpcx & 0xc0)==0xc0) {
			cuenta=bpcx & 0x3f;
			if((bpcx=fgetc(fichpcx))==EOF) {
				fclose(fichpcx);
				pcx_libera_mem(imagen);
				return(E_PCX_LECT);
			}
		}
		else cuenta=1;

		/* expande byte */
		for(j=0; j<cuenta; j++) {
			*plin++=(BYTE)bpcx;
			/* incrementa n£mero de bytes decodificados */
			bytes_decodif++;
			/* si se completa la l¡nea de scan, sale */
			if(bytes_decodif==totbytes) break;
		}
	} while(bytes_decodif<totbytes);
}

fclose(fichpcx);

return(0);
}

/****************************************************************************
	PCX_LIBERA_MEM: libera la memoria ocupada por una imagen.
	  Entrada:      'imagen' puntero a imagen.
****************************************************************************/
void pcx_libera_mem(STC_IMAGEN *imagen)
{
int i;

if(imagen->lineas!=NULL) {
	for(i=0; i<imagen->alto; i++) farfree(imagen->lineas[i]);
	free(imagen->lineas);
	imagen->lineas=NULL;
}

}

/****************************************************************************
	PCX_DIBUJA: dibuja una imagen PCX.
	  Entrada:	'imagen' puntero a estructura de la imagen
			'x', 'y' coordenadas de pantalla d¢nde se dibujar 
			el gr fico
			'anchura', 'altura' dimensiones (en pixels) de la
			zona del gr fico que se dibujar , si alguno de los
			valores es 0, se coger n las dimensiones dadas por
			la cabecera del fichero PCX
			'paleta' 0 si se debe coger la paleta de la imagen,
			1 si se debe respetar la paleta actual
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MODO - modo de pantalla incorrecto
****************************************************************************/
int pcx_dibuja(STC_IMAGEN *imagen, int x, int y, int anchura, int altura,
  int paleta)
{
register int i;
int ret, dimx, dimy, masc_pln, bytes_plano;
BYTE far *dirvideo, far *plin;

/* comprueba modo y calcula dimensiones de pantalla */
switch(_g_modovideo) {
	case G_MV_G1C16 :
		dimx=640;
		dimy=200;
		if(imagen->num_colores!=16) return(E_PCX_FORM);
		break;
	case G_MV_G2C16 :
		dimx=640;
		dimy=350;
		if(imagen->num_colores!=16) return(E_PCX_FORM);
		break;
	case G_MV_G3C16 :
		dimx=640;
		dimy=480;
		if(imagen->num_colores!=16) return(E_PCX_FORM);
		break;
	case G_MV_G1C256 :
		dimx=320;
		dimy=200;
		if(imagen->num_colores!=256) return(E_PCX_FORM);
		break;
	case G_MV_G2C256 :
		dimx=360;
		dimy=480;
		if(imagen->num_colores!=256) return(E_PCX_FORM);
		break;
	default :
		return(E_PCX_MODO);
}

if(paleta==0) carga_paleta(&imagen->paleta[0][0],imagen->num_colores);

/* si alguna dimensi¢n es 0 o mayor que la imagen, coge la de la imagen */
if(!altura || (altura>imagen->alto)) altura=imagen->alto;
if(!anchura || (anchura>imagen->ancho)) anchura=imagen->ancho;

/* si el gr fico a dibujar no cabe en la pantalla */
/* ajusta las dimensiones para que quepa */
if((y+altura)>dimy) altura=dimy-y;
if((x+anchura)>dimx) anchura=dimx-x;

if(_g_modovideo==G_MV_G1C256) {
	/* puntero al primer pixel a dibujar en buffer de v¡deo */
	dirvideo=(BYTE far *)0xa0000000L+(x+(y*TAMLINPCX256));

	/* dibuja imagen en pantalla */
	for(i=0; i<altura; i++) {
		plin=imagen->lineas[i];
		_fmemcpy(dirvideo,plin,anchura);
		dirvideo+=TAMLINPCX256;
	}
}
else if(_g_modovideo==G_MV_G2C256) {
	/* puntero al primer pixel a dibujar en buffer de v¡deo */
	dirvideo=(BYTE far *)0xa0000000L+((x/4)+(y*TAMLINPCX256X));

	/* m scara de planos de bit */
	masc_pln=(1 << (x & 3));

	/* dibuja imagen en pantalla */
	for(i=0; i<altura; i++) {
		dibuja_lin256x(imagen->lineas[i],dirvideo,anchura,masc_pln);
		dirvideo+=TAMLINPCX256X;
	}
}
else {
	/* calcula la anchura en bytes */
	anchura/=8;

	/* puntero al primer pixel a dibujar en buffer de v¡deo */
	dirvideo=(BYTE far *)0xa0000000L+((x/8)+(y*TAMLINPCX16));

	/* calcula n§ de bytes por plano */
	bytes_plano=imagen->ancho/8;

	/* dibuja imagen en pantalla */
	for(i=0; i<altura; i++) {
		dibuja_lin16(imagen->lineas[i],dirvideo,anchura,bytes_plano);
		dirvideo+=TAMLINPCX16;
	}
}

return(0);
}

/****************************************************************************
	PCX_VISUALIZA: decodifica una imagen PCX y la va dibujando.
	  Entrada:      'nombre_pcx' nombre del fichero PCX
			'x', 'y' coordenadas de pantalla d¢nde se dibujar 
			el gr fico
			'anchura', 'altura' dimensiones (en pixels) de la
			zona del gr fico que se dibujar , si alguno de los
			valores es 0, se coger n las dimensiones dadas por
			la cabecera del fichero PCX
			'paleta' 0 si se debe coger la paleta de la imagen,
			1 si se debe respetar la paleta actual
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_APER - error de apertura de fichero PCX
			  E_PCX_LECT - error de lectura de fichero PCX
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MODO - modo de pantalla incorrecto
****************************************************************************/
int pcx_visualiza(char *nombre_pcx, int x, int y, int anchura, int altura,
  int paleta)
{
FILE *fichpcx;
STC_CABPCX pcx;
fpos_t fpos;
BYTE far *plin, far *dirvideo;
register int i, j;
int num_colores, dimx, dimy, img_ancho, img_alto, bytes_decodif, cuenta,
  bpcx, totbytes, masc_pln, bytes_plano;

if((fichpcx=fopen(nombre_pcx,"rb"))==NULL) return(E_PCX_APER);

/* lee la cabecera */
if(fread(&pcx,sizeof(STC_CABPCX),1,fichpcx)!=1) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* comprueba que sea formato PCX */
if(pcx.propietario!=0x0a) {
	fclose(fichpcx);
	return(E_PCX_FORM);
}

/* n£mero de colores de la imagen */
num_colores=1 << (pcx.bits_pixel*pcx.num_planos);

/* comprueba modo y calcula dimensiones de pantalla */
switch(_g_modovideo) {
	case G_MV_G1C16 :
		dimx=640;
		dimy=200;
		if(num_colores!=16) return(E_PCX_FORM);
		break;
	case G_MV_G2C16 :
		dimx=640;
		dimy=350;
		if(num_colores!=16) return(E_PCX_FORM);
		break;
	case G_MV_G3C16 :
		dimx=640;
		dimy=480;
		if(num_colores!=16) return(E_PCX_FORM);
		break;
	case G_MV_G1C256 :
		dimx=320;
		dimy=200;
		if(num_colores!=256) return(E_PCX_FORM);
		break;
	case G_MV_G2C256 :
		dimx=360;
		dimy=480;
		if(num_colores!=256) return(E_PCX_FORM);
		break;
	default :
		return(E_PCX_MODO);
}

if(num_colores==16) {
	/* coge y convierte la informaci¢n de paleta */
	for(i=0; i<16; i++) {
		pcx.paleta[i][0]>>=2;
		pcx.paleta[i][1]>>=2;
		pcx.paleta[i][2]>>=2;
	}

	if(paleta==0) carga_paleta(&pcx.paleta[0][0],16);
}
else if(num_colores==256) {
	/* guarda el puntero del fichero */
	if(fgetpos(fichpcx,&fpos)) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}

	/* pone el puntero del fichero al comienzo de la informaci¢n de paleta */
	if(fseek(fichpcx,-769L,SEEK_END)) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}

	/* lee 1er byte de informaci¢n de cabecera y comprueba que sea 12 */
	if((bpcx=fgetc(fichpcx))==EOF) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
	if(bpcx!=12) {
		fclose(fichpcx);
		return(E_PCX_FORM);
	}

	/* lee y convierte la informaci¢n de paleta */
	for(i=0; i<256; i++) {
		/* componente rojo */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}
		color[i][0]=(BYTE)(bpcx >> 2);
		/* componente verde */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}
		color[i][1]=(BYTE)(bpcx >> 2);
		/* componente azul */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}
		color[i][2]=(BYTE)(bpcx >> 2);
	}

	/* recupera el puntero del fichero */
	if(fsetpos(fichpcx,&fpos)) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}

	if(paleta==0) carga_paleta(&color[0][0],256);
}

/* si alguna dimensi¢n es 0 o mayor que la imagen, coge la de la imagen */
img_alto=pcx.ventana[3]-pcx.ventana[1]+1;
img_ancho=pcx.ventana[2]-pcx.ventana[0]+1;
if(!altura || (altura>img_alto)) altura=img_alto;
if(!anchura || (anchura>img_ancho)) anchura=img_ancho;

/* si el gr fico a dibujar no cabe en la pantalla */
/* ajusta las dimensiones para que quepa */
if((y+altura)>dimy) altura=dimy-y;
if((x+anchura)>dimx) anchura=dimx-x;

/* n£mero de bytes por l¡nea */
totbytes=pcx.bytes_scan*pcx.num_planos;

if(_g_modovideo==G_MV_G1C256) {
	/* puntero al primer pixel a dibujar en buffer de v¡deo */
	dirvideo=(BYTE far *)0xa0000000L+(x+(y*TAMLINPCX256));
}
else if(_g_modovideo==G_MV_G2C256) {
	/* puntero al primer pixel a dibujar en buffer de v¡deo */
	dirvideo=(BYTE far *)0xa0000000L+((x/4)+(y*TAMLINPCX256X));
	/* m scara de planos de bit */
	masc_pln=(1 << (x & 3));
}
else {
	/* calcula la anchura en bytes */
	anchura/=8;
	/* direcci¢n en buffer de v¡deo de primer pixel a dibujar */
	dirvideo=(BYTE far *)0xa0000000L+((x/8)+(y*TAMLINPCX16));
	bytes_plano=pcx.bytes_scan;
}

/* bucle para decodificar/dibujar la imagen */
for(i=0; i<altura; i++) {
	/* inicializa puntero a buffer de l¡nea */
	plin=lin_scan;

	/* inicializa contador de bytes decodificados */
	bytes_decodif=0;

	/* bucle para decodificar una l¡nea de scan */
	do {
		/* lee 1 byte del fichero PCX */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}

		/* si los 2 bits altos est n a 1, recoge los 6 bits bajos */
		/* y lee el siguiente byte */
		if((bpcx & 0xc0)==0xc0) {
			cuenta=bpcx & 0x3f;
			if((bpcx=fgetc(fichpcx))==EOF) {
				fclose(fichpcx);
				return(E_PCX_LECT);
			}
		}
		else cuenta=1;

		/* expande byte */
		for(j=0; j<cuenta; j++) {
			*plin++=(BYTE)bpcx;
			/* incrementa n£mero de bytes decodificados */
			bytes_decodif++;
			/* si se completa la l¡nea de scan, sale */
			if(bytes_decodif==totbytes) break;
		}
	} while(bytes_decodif<totbytes);

	/* dibuja l¡nea en pantalla */
	if(_g_modovideo==G_MV_G1C256) {
		_fmemcpy(dirvideo,lin_scan,anchura);
		dirvideo+=TAMLINPCX256;
	}
	else if(_g_modovideo==G_MV_G2C256) {
		dibuja_lin256x(lin_scan,dirvideo,anchura,masc_pln);
		dirvideo+=TAMLINPCX256X;
	}
	else {
		/* dibuja la l¡nea */
		dibuja_lin16(lin_scan,dirvideo,anchura,bytes_plano);
		dirvideo+=TAMLINPCX16;
	}
}

fclose(fichpcx);

return(0);
}
