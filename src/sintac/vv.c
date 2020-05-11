/****************************************************************************
	Conjunto de funciones para gestionar impresi¢n por pantalla en
	ventanas, en los modos gr ficos de 16 y 256 colores de VGA.
****************************************************************************/

#include <bios.h>
#include "graf.h"
#include "vv.h"

/*** Variables globales ***/
/* lista de teclas admitidas por la funci¢n vv_lee_tecla() */
char Cod_Teclas[]="ABCDEFGHIJKLMN¥OPQRSTUVWXYZ"
		  "abcdefghijklmn¤opqrstuvwxyz ‚¡¢£"
		  "0123456789"
		  ".,;:!­?¨\"\'\r\b "
		  "\xff\xfe\xfd\xfc\xfb\xfa\xf9\xf8\xf7\xf6\xf5"
		  "\xf4\xf3\xf2\xf1\xf0\xef\xee";
/* diferentes tipos de bordes para ventanas */
char Borde[][6]={
	"Ä³Ú¿ÀÙ",
	"ÍºÉ»È¼",
	"Í³Õ¸Ô¾",
	"ÄºÖ·Ó½",
	"°°°°°°",
	"±±±±±±",
	"²²²²²²",
	"ÛÛÛÛÛÛ",
};

/*** Prototipos de funciones internas ***/
static void vv_imp_borde(STC_VV *v);
static int vv_esta_en(char *s, char c);

/****************************************************************************
	VV_IMP_BORDE: imprime el borde de una ventana.
	  Entrada:      'v' puntero a ventana.
****************************************************************************/
void vv_imp_borde(STC_VV *v)
{
int i;

rg_puntero(RG_OCULTA);

if(v->borde) {
	/* esquina superior izquierda */
	imp_chr_pos(v->vx*8,v->vy*v->chralt);
	imp_chr(Borde[v->borde-1][2],v->colorf,v->color,CHR_NORM);

	/* esquina inferior izquierda */
	imp_chr_pos(v->vx*8,(v->vy+v->ly-1)*v->chralt);
	imp_chr(Borde[v->borde-1][4],v->colorf,v->color,CHR_NORM);

	/* esquina superior derecha */
	imp_chr_pos((v->vx+v->lx-1)*8,v->vy*v->chralt);
	imp_chr(Borde[v->borde-1][3],v->colorf,v->color,CHR_NORM);

	/* esquina inferior derecha */
	imp_chr_pos((v->vx+v->lx-1)*8,(v->vy+v->ly-1)*v->chralt);
	imp_chr(Borde[v->borde-1][5],v->colorf,v->color,CHR_NORM);

	/* lineas superior e inferior */
	for(i=v->vx+1; i<(v->vx+v->lx-1); i++) {
		imp_chr_pos(i*8,v->vy*v->chralt);
		imp_chr(Borde[v->borde-1][0],v->colorf,v->color,CHR_NORM);
		imp_chr_pos(i*8,(v->vy+v->ly-1)*v->chralt);
		imp_chr(Borde[v->borde-1][0],v->colorf,v->color,CHR_NORM);
	}

	/* lineas laterales */
	for(i=v->vy+1; i<(v->vy+v->ly-1); i++) {
		imp_chr_pos(v->vx*8,i*v->chralt);
		imp_chr(Borde[v->borde-1][1],v->colorf,v->color,CHR_NORM);
		imp_chr_pos((v->vx+v->lx-1)*8,i*v->chralt);
		imp_chr(Borde[v->borde-1][1],v->colorf,v->color,CHR_NORM);
	}
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	VV_ESTA_EN: comprueba si un car cter est  en una cadena.
	  Entrada:      's' cadena con la que se comprobar  el car cter
			'c' car cter a comprobar
	  Salida:       1 si el car cter est  en la cadena
			0 si no
****************************************************************************/
int vv_esta_en(char *s, char c)
{

while(*s) {
	if(*s++==c) return(1);
}

return(0);
}

/****************************************************************************
	VV_SCROLL: realiza el scroll de una ventana dada, una l¡nea hacia
	  arriba.
	  Entrada:      'v' puntero a ventana
****************************************************************************/
void vv_scroll(STC_VV *v)
{

rg_puntero(RG_OCULTA);
g_scroll_arr(v->vyi,v->vxi,v->lxi,v->lyi,v->colorf);
rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	VV_CREA: crea una ventana.
	  Entrada:      'f', 'c' fila y columna
			'ancho', 'alto' dimensiones de la ventana
			'colorf' color del fondo
			'color' color del primer plano
			'borde' tipo de borde para la ventana
			'v' puntero a la ventana
****************************************************************************/
void vv_crea(BYTE f, BYTE c, BYTE ancho, BYTE alto, BYTE colorf, BYTE color,
  BYTE borde, STC_VV *v)
{

/* inicializa origen */
v->vx=c;
v->vy=f;

/* inicializa dimensiones */
v->lx=ancho;
v->ly=alto;

/* inicializa color */
v->colorf=colorf;
v->color=color;
v->colortf=colorf;
v->colort=color;

/* inicializa tipo de borde */
v->borde=borde;

/* inicializa coordenadas y dimensiones del interior de la ventana */
if(borde) {
	v->vxi=(BYTE)(v->vx+1);
	v->vyi=(BYTE)(v->vy+1);
	v->lxi=(BYTE)(v->lx-2);
	v->lyi=(BYTE)(v->ly-2);
}
else {
	v->vxi=v->vx;
	v->vyi=v->vy;
	v->lxi=v->lx;
	v->lyi=v->ly;
}

/* inicializa posici¢n cursor dentro de ventana */
v->cvx=0;
v->cvy=0;
v->cvxs=0;
v->cvys=0;

/* borra indicador de scroll */
v->scroll=0;

/* altura de los caracteres */
v->chralt=chr_altura();

}

/****************************************************************************
	VV_CLS: borra/inicializa una ventana.
	  Entrada:      'v' puntero a ventana a inicializar
****************************************************************************/
void vv_cls(STC_VV *v)
{

rg_puntero(RG_OCULTA);

g_rectangulo(v->vxi*8,v->vyi*v->chralt,((v->vxi+v->lxi)*8)-1,
  ((v->vyi+v->lyi)*v->chralt)-1,v->colorf,G_NORM,1);

/* posiciona el cursor al inicio de la ventana */
v->cvx=0;
v->cvy=0;

/* resetea el color temporal */
v->colortf=v->colorf;
v->colort=v->color;

/* borra indicador de scroll */
v->scroll=0;

/* imprime el borde de la ventana */
vv_imp_borde(v);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	VV_IMPC: imprime un car cter en una ventana. S¢lo tiene en cuenta
	  como car cter especial el c¢digo 13 (avance de l¡nea).
	  Entrada:      'c' car cter a imprimir
			'v' puntero a ventana
	  Salida:	anchura del car cter
****************************************************************************/
int vv_impc(char c, STC_VV *v)
{
STC_RATONG r;
int scroll=0, sobre_car=0, x, y, esqx, esqy, anch;
/* buffer para guardar car cter cuando hay espera de scroll */
/* el tama¤o es m s que el necesario para guardar un car cter */
char img_car[90];

x=(v->vxi*8)+v->cvx;
y=(v->vyi*v->chralt)+v->cvy;
anch=chr_anchura(c);

/* si el puntero del rat¢n est  sobre el car cter lo oculta */
rg_estado(&r);
/* si es modo de 16 colores redondea coordenada X a m£ltiplo de 8 */
if(g_coge_modovideo()!=G_MV_G1C256) r.x=(r.x/8)*8;
if((x>(r.x-anch)) && (x<(r.x+24)) && (y>(r.y-v->chralt)) && (y<(r.y+16))) {
	rg_puntero(RG_OCULTA);
	sobre_car=1;
}

if(c=='\n') {
	v->cvx=0;
	v->cvy+=v->chralt;
	if(v->cvy>((v->lyi-1)*v->chralt)) scroll=1;
}
else if((v->cvx+anch)>(v->lxi*8)) {
	v->cvx=0;
	v->cvy+=v->chralt;
	if(v->cvy>((v->lyi-1)*v->chralt)) scroll=1;
}

if(scroll) {
	/* coloca en £ltima l¡nea e incrementa indicador de scroll */
	v->cvy=(v->lyi-1)*v->chralt;
	v->scroll++;
	if(v->scroll>v->lyi) v->scroll=1;
	/* espera pulsaci¢n de tecla antes de scroll */
	if(v->scroll==1) {
		/* imprime car cter de scroll y espera tecla */
		esqx=(v->vxi+v->lxi-1)*8;
		if(v->borde!=NO_BORDE) esqy=(v->vyi+v->lyi)*v->chralt;
		else esqy=(v->vyi+v->lyi-1)*v->chralt;

		/* guarda car cter sobre el que imprimir  */
		rg_puntero(RG_OCULTA);
		blq_coge(esqx,esqy,esqx+7,esqy+v->chralt-1,
		  (unsigned char far *)img_car);

		/* imprime car cter de espera scroll */
		imp_chr_pos(esqx,esqy);
		imp_chr(CAR_SCROLL,v->colorf,v->color,CHR_NORM);
		rg_puntero(RG_MUESTRA);

		/* espera tecla */
		vv_lee_tecla();

		/* borra car cter de espera */
		rg_puntero(RG_OCULTA);
		blq_pon(esqx,esqy,(unsigned char far *)img_car);
		rg_puntero(RG_MUESTRA);
	}
	/* si el indicador de scroll est  activo, hace scroll */
	if(v->scroll) vv_scroll(v);
}

if(c!='\n') {
	imp_chr_pos((v->vxi*8)+v->cvx,(v->vyi*v->chralt)+v->cvy);
	imp_chr(c,v->colortf,v->colort,CHR_NORM);
	v->cvx+=anch;
}

if(sobre_car) rg_puntero(RG_MUESTRA);

return(anch);
}

/****************************************************************************
	VV_IMPS: imprime una cadena en una ventana. Las palabras de final de
	  l¡nea que no quepan dentro de la ventana se pasan a la l¡nea
	  siguiente.
	  Entrada:      's' cadena a imprimir
			'v' puntero a ventana
****************************************************************************/
void vv_imps(char *s, STC_VV *v)
{
char b[MAX_PAL];                /* buffer para guardar palabras */
int i, cuenta=0, anch=0;

while(1) {
	/* si se encontr¢ fin de frase, espacio o avance de l¡nea */
	if(!*s || (*s==' ') || (*s=='\n')) {
		/* si no hay palabra almacenada */
		if(!cuenta && *s) {
			vv_impc(*s,v);
			s++;
		}
		else {
			/* incluye espacio en palabra */
			if(*s==' ') {
				anch+=chr_anchura(*s);
				b[cuenta]=*s++;
				cuenta++;
			}
			/* si la palabra almacenada no cabe en lo que */
			/* queda de l¡nea pero cabe en la siguiente, */
			/* la imprime en la siguiente l¡nea, si no la */
			/* imprime sin m s */
			if((anch>((v->lxi*8)-v->cvx)) &&
			  (anch<=(v->lxi*8))) vv_impc('\n',v);
			for(i=0; i<cuenta; i++) vv_impc(b[i],v);
			cuenta=0;
			anch=0;
			/* si fin de frase, sale */
			if(!*s) break;
		}
	}
	else {
		/* si letra no es espacio ni avance de l¡nea la guarda */
		anch+=chr_anchura(*s);
		b[cuenta]=*s++;
		cuenta++;
		/* si se llena buffer */
		if(cuenta==MAX_PAL) {
			/* imprime lo que hay almacenado */
			for(i=0; i<MAX_PAL; i++) vv_impc(b[i],v);
			cuenta=0;
			anch=0;
			/* imprime resto de palabra */
			while(*s && (*s!=' ') && (*s!='\n')) {
				vv_impc(*s,v);
				s++;
			}
		}
	}
}

}

/****************************************************************************
	VV_IMPN: imprime un n£mero en una ventana.
	  Entrada:      'n' n£mero (16 bits) a imprimir
			'v' puntero a ventana
****************************************************************************/
void vv_impn(unsigned n, STC_VV *v)
{
int i=0;
char d[5];

do {
	/* coge el resto de n/10 y lo pasa a ASCII */
	d[i]=(char)((n%10)+'0');
	n/=10;
	i++;
} while(n);

/* imprime los d¡gitos en orden inverso a como fueron hayados */
while(i--) vv_impc(d[i],v);

}

/****************************************************************************
	VV_LEE_TECLA: devuelve el c¢digo ASCII de la tecla pulsada.
	  Espera hasta que se pulse una tecla, o hasta que la tecla pulsada
	  sea una de las contenidas en la cadena Cod_Teclas (global).
	  Salida:       c¢digo ASCII de la tecla pulsada.
			si se puls¢ una tecla de cursor devuelve COD_IZQ,
			COD_DER, COD_ARR, COD_ABJ, COD_ORG o COD_FIN;
			si se puls¢ una tecla de funci¢n devuelve COD_Fx;
			si se puls¢ un bot¢n del rat¢n devuelve COD_RAT
****************************************************************************/
unsigned vv_lee_tecla(void)
{
STC_RATONG r;
unsigned tecla;

do {
	tecla=0;
	while(!bioskey(1)) {
		rg_estado(&r);
		if(r.boton1 || r.boton2) {
			tecla=COD_RAT;
			break;
		}
	}
	/* byte bajo=ASCII, byte alto=CODIGO */
	if(tecla!=COD_RAT) tecla=bioskey(0);

	/* comprueba teclas especiales y pone ASCII=c¢digo tecla */
	if(tecla==IZQ) tecla=COD_IZQ;
	else if(tecla==DER) tecla=COD_DER;
	else if(tecla==ARR) tecla=COD_ARR;
	else if(tecla==ABJ) tecla=COD_ABJ;
	else if(tecla==ORG) tecla=COD_ORG;
	else if(tecla==FIN) tecla=COD_FIN;
	else if(tecla==SUP) tecla=COD_SUP;
	else if(tecla==F1) tecla=COD_F1;
	else if(tecla==F2) tecla=COD_F2;
	else if(tecla==F3) tecla=COD_F3;
	else if(tecla==F4) tecla=COD_F4;
	else if(tecla==F5) tecla=COD_F5;
	else if(tecla==F6) tecla=COD_F6;
	else if(tecla==F7) tecla=COD_F7;
	else if(tecla==F8) tecla=COD_F8;
	else if(tecla==F9) tecla=COD_F9;
	else if(tecla==F10) tecla=COD_F10;

	/* desprecia byte alto */
	tecla &= 0x00ff;
} while(!vv_esta_en(Cod_Teclas,(char)tecla));

return(tecla);
}
