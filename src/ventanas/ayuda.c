/****************************************************************************
				   AYUDA.C

	Biblioteca de funciones para gestionar ayuda en l¡nea.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- a_inicializa: inicializa el sistema de ayuda
		- a_elimina: libera la memoria ocupada por el sistema
 		    de ayuda
		- a_cambia_color: modifica los colores del sistema de ayuda
		- a_crea_fichero_indices: crea un fichero de ¡ndices de
		    un fichero de ayuda.
		- a_ayuda: muestra ayuda sobre un tema
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <io.h>
#include "ventana.h"
#include "cuadro.h"
#include "ayuda.h"

/*** Variables globales internas ***/
static STC_CUADRO cind;		/* ¡ndice */
static STC_CUADRO cayd;		/* ayuda tema */
static STC_ELEM_LISTA *lst_ayd;
static STC_ELEM_LISTA *lst_ref1;
static STC_ELEM_LISTA *lst_ref2;

/* extensiones de ficheros */
static char *ExtAyd=A_EXTAYD;
static char *ExtIyd=A_EXTIYD;

/* mensajes de error */
static char *Merr_Fayd=" No se puede abrir fichero de ayuda";
static char *Merr_Fiyd="No se puede abrir fichero de ¡ndices";

/*** Prototipos de funciones internas ***/
static void beep(void);
static char mayuscula(char c);
static void may_str(char *s);
static void error_ayuda(char *msg);
static int busca_ref(char *l, char *ref);
static int busca_tema(FILE *fiyd, FILE *fayd, char *tema);

/****************************************************************************
	BEEP: produce un pitido en el altavoz del PC.
****************************************************************************/
void beep(void)
{

asm {
	sub bx,bx               // p gina 0
	mov ax,0E07h            // escribe el car cter de alarma
	int 10h
}

}

/****************************************************************************
	MAYUSCULA: convierte una letra en may£scula.
	  Entrada:      'c' car cter a convertir
	  Salida:       may£scula del car cter
****************************************************************************/
char mayuscula(char c)
{

if((c>='a') && (c<='z')) return(c-(char)'a'+(char)'A');

switch(c) {
	case (char)'¤' :
		c=(char)'¥';
		break;
	case (char)' ' :
		c='A';
		break;
	case (char)'‚' :
		c='E';
		break;
	case (char)'¡' :
		c='I';
		break;
	case (char)'¢' :
		c='O';
		break;
	case (char)'£' :
	case (char)'' :
		c='U';
		break;
}

return(c);
}

/****************************************************************************
	MAY_STR: convierte una cadena en may£scula.
	  Entrada:      's' puntero a cadena
****************************************************************************/
void may_str(char *s)
{

while(*s) {
	*s=mayuscula(*s);
	s++;
}

}

/****************************************************************************
	ERROR_AYUDA: imprime mensajes de error de las rutinas de ayuda.
	  Entrada:      'msg' mensaje de error
****************************************************************************/
void error_ayuda(char *msg)
{
STC_CUADRO cerr;

c_crea_cuadro(&cerr," ERROR ",C_CENT,C_CENT,A_ERRANCHO,A_ERRALTO,
  cayd.v.clr_princ,cayd.v.clr_s1,cayd.v.clr_s2,cayd.clr_boton,cayd.clr_input,
  cayd.clr_sel,cayd.clr_tecla,cayd.clr_tecla_boton);

c_crea_elemento(&cerr,C_ELEM_BOTON,A_ERRALTO-4,(A_ERRANCHO-8)/2,"^Vale",8);
c_crea_elemento(&cerr,C_ELEM_TEXTO,0,0,msg,A_ERRANCHO-2,A_ERRALTO-4,
  C_TXTPASA,C_TXTNOBORDE);

c_abre(&cerr);
c_gestiona(&cerr);
c_cierra(&cerr);
c_elimina(&cerr);

}

/****************************************************************************
	BUSCA_REF: comprueba si una referencia (primera palabra de un tema)
	  o un tema se encuentran en una l¡nea de texto.
	  Entrada:      'l' l¡nea a analizar
			'ref' referencia a buscar
	  Salida:       1 si se encontr¢ la referencia o el tema, 0 si no
****************************************************************************/
int busca_ref(char *l, char *ref)
{
char pl[A_LNGREF];
int i;

/* comprueba si referencia y l¡nea son iguales */
if(!strcmp(l,ref)) return(1);

/* mira si la primera palabra de l¡nea se corresponde con referencia */
for(i=0; (i<(A_LNGREF-1)) && *l && (*l!=' '); i++) pl[i]=*l++;
pl[i]='\0';

if(!strcmp(pl,ref)) return(1);

return(0);
}

/****************************************************************************
	BUSCA_TEMA: busca ayuda sobre un tema, si lo encuentra recoge el
	  texto de ayuda y las referencias en dos listas.
	  Entrada:      'fiyd' puntero a fichero de ¡ndices
			'fayd' puntero a fichero de ayuda
			'tema' tema sobre el que buscar ayuda (o palabra
			inicial del tema)
	  Salida:       1 si encontr¢ ayuda sobre el tema, 0 si no
****************************************************************************/
int busca_tema(FILE *fiyd, FILE *fayd, char *tema)
{
STC_REFAYD ref;
char t[A_LNGREF], lin[A_LNGLINAYD], *l;
int i;

/* puntero a inicio de fichero de ¡ndices */
rewind(fiyd);

strcpy(t,tema);
may_str(t);

/* borra listas de ayuda */
c_borra_lista(lst_ayd);
c_borra_lista(lst_ref1);
c_borra_lista(lst_ref2);

/* busca tema en ¡ndice, si no da tema coge el primero */
if(!*t) fread(&ref,sizeof(STC_REFAYD),1,fiyd);
else {
	while(1) {
		if(feof(fiyd)) return(0);
		fread(&ref,sizeof(STC_REFAYD),1,fiyd);
		/* si encuentra tema o referencia, mete texto en lista */
		if(busca_ref(ref.txt,t)) break;
	}
}

/* mete texto de secci¢n en lista */
fsetpos(fayd,&ref.pos);
while(1) {
	if(fgets(lin,A_LNGLINAYD-1,fayd)==NULL) break;
	if(*lin==A_CHRSECC2) break;
	/* elimina '\n' final */
	i=strlen(lin);
	if(lin[i-1]=='\n') lin[i-1]='\0';

	/* si hay referencia, la mete en lista de referencias */
	for(l=lin; *l; l++) {
		if(*l==A_CHRREF) {
			/* si no di¢ tema mete en lista de ¡ndice */
			/* en otro caso mete en lista de referencias */
			if(!*t) c_mete_en_lista(lst_ref1,l+1);
			else c_mete_en_lista(lst_ref2,l+1);
			break;
		}
	}
	/* elimina texto de referencia */
	*l='\0';

	/* mete l¡nea de secci¢n en lista */
	if(*lin==A_CHRSECC1) c_mete_en_lista(lst_ayd,&lin[1]);
	else c_mete_en_lista(lst_ayd,lin);
}

return(1);
}

/****************************************************************************
	A_INICIALIZA: inicializa diversos datos usados por el resto de las
	  rutinas de ayuda.
	  Entrada:      'fil', 'col' posici¢n de la ventana de ayuda,
			C_CENT para centrar
			'ancho', 'alto' tama¤o de la ventana de ayuda
			'ancho_vref' anchura de ventana para mostrar lista
			de referencias
			'clr_princ' color principal
			'clr_s1', 'clr_s2' colores de sombreado
			'clr_boton' color de botones
			'clr_input' color de elemento de input
			'clr_tecla' color de teclas de activaci¢n
			'clr_tecla_boton' color de teclas activaci¢n botones
			'clr_selecc' color para elemento seleccionado
****************************************************************************/
void a_inicializa(int fil, int col, int ancho, int alto, int ancho_vref,
  BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_boton, BYTE clr_input,
  BYTE clr_tecla, BYTE clr_tecla_boton, BYTE clr_selecc)
{
STC_ELEM *elem;

c_crea_cuadro(&cind,"",fil,col,ancho,alto,clr_princ,clr_s1,clr_s2,clr_boton,
  clr_input,clr_selecc,clr_tecla,clr_tecla_boton);
elem=c_crea_elemento(&cind,C_ELEM_LISTA,0,0," ^Referencias ",ancho-2,alto-5,
  C_LSTNORMAL,C_LSTSINORDEN);
lst_ref1=(STC_ELEM_LISTA *)(elem->info);
c_crea_elemento(&cind,C_ELEM_BOTON,alto-4,1,"Seleccionar ^tema",20);
c_crea_elemento(&cind,C_ELEM_BOTON,alto-4,24,"^Salir",9);

c_crea_cuadro(&cayd,"",fil,col,ancho,alto,clr_princ,clr_s1,clr_s2,clr_boton,
  clr_input,clr_selecc,clr_tecla,clr_tecla_boton);
elem=c_crea_elemento(&cayd,C_ELEM_LISTA,0,0," ^Ayuda ",ancho-ancho_vref-2,
  alto-5,C_LSTPRIMERO,C_LSTSINORDEN);
lst_ayd=(STC_ELEM_LISTA *)(elem->info);
elem=c_crea_elemento(&cayd,C_ELEM_LISTA,0,ancho-ancho_vref-2,
  " ^Referencias ",ancho_vref,alto-5,C_LSTNORMAL,C_LSTSINORDEN);
lst_ref2=(STC_ELEM_LISTA *)(elem->info);
c_crea_elemento(&cayd,C_ELEM_BOTON,alto-4,1,"^Indice",10);
c_crea_elemento(&cayd,C_ELEM_BOTON,alto-4,14,"Ir a re^ferencia",19);
c_crea_elemento(&cayd,C_ELEM_BOTON,alto-4,36,"A^nterior",12);
c_crea_elemento(&cayd,C_ELEM_BOTON,alto-4,51,"^Salir",9);

}

/****************************************************************************
	A_ELIMINA: libera la memoria ocupada por el sistema de ayuda.
****************************************************************************/
void a_elimina(void)
{

c_elimina(&cind);
c_elimina(&cayd);

}

/****************************************************************************
	A_CAMBIA_COLOR: modifica los colores del sistema de ayuda.
	  Entrada:      'clr_princ' color principal
			'clr_s1', 'clr_s2' colores de sombreado
			'clr_boton' color de botones
			'clr_input' color de elemento de input
			'clr_tecla' color de teclas de activaci¢n
			'clr_tecla_boton' color de teclas activaci¢n botones
			'clr_selecc' color para elemento seleccionado
****************************************************************************/
void a_cambia_color(BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_boton, 
  BYTE clr_input, BYTE clr_tecla, BYTE clr_tecla_boton, BYTE clr_selecc)
{

c_cambia_color(&cind,clr_princ,clr_s1,clr_s2,clr_boton,clr_input,clr_selecc,
  clr_tecla,clr_tecla_boton,0);
c_cambia_color(&cayd,clr_princ,clr_s1,clr_s2,clr_boton,clr_input,clr_selecc,
  clr_tecla,clr_tecla_boton,0);

}

/****************************************************************************
	A_CREA_FICHERO_INDICES: crea un fichero de ¡ndices para un fichero
	  de ayuda.
	  Entrada:      'nfayd' nombre de fichero con la ayuda, sin
			extensi¢n
	  Salida:       1 si pudo crear fichero de ¡ndices, 0 si no
****************************************************************************/
int a_crea_fichero_indices(char *nfayd)
{
FILE *fayd, *fiyd;
STC_REFAYD ref;
fpos_t pos;
char nf[MAXPATH], lin[A_LNGLINAYD];
int i;

strcpy(nf,nfayd);
strcat(nf,ExtAyd);
if((fayd=fopen(nf,"rt"))==NULL) return(0);
strcpy(nf,nfayd);
strcat(nf,ExtIyd);
if((fiyd=fopen(nf,"wb"))==NULL) {
	fclose(fayd);
	return(0);
}

while(1) {
	fgetpos(fayd,&pos);
	if(fgets(lin,A_LNGLINAYD-1,fayd)==NULL) {
		if(feof(fayd)) break;
		else {
			fclose(fiyd);
			fclose(fayd);
			return(0);
		}
	}
	/* si es inicio de secci¢n de ayuda, almacena referencia */
	/* en fichero de ¡ndices */
	if(lin[0]==A_CHRSECC1) {
		ref.pos=pos;
		for(i=0; ((i<(A_LNGREF-1)) && (lin[i+1]!='\n')); i++)
		  ref.txt[i]=mayuscula(lin[i+1]);
		for(; i<A_LNGREF; i++) ref.txt[i]='\0';
		if(fwrite(&ref,sizeof(STC_REFAYD),1,fiyd)!=1) {
			fclose(fiyd);
			fclose(fayd);
			return(0);
		}
	}
}

fclose(fiyd);
fclose(fayd);

return(1);
}

/****************************************************************************
	A_AYUDA: rutina de gesti¢n principal de ayuda.
	  Entrada:      'nfayd' nombre de fichero, sin extensi¢n, que
			contiene la ayuda
			'tema' tema sobre el que buscar ayuda, "" si se
			quiere el ¡ndice de temas disponibles
	  Salida:       1 si se proces¢ ayuda con ‚xito, 0 si hubo error
****************************************************************************/
int a_ayuda(char *nfayd, char *tema)
{
FILE *fayd, *fiyd;
char nf[MAXPATH], tema_act[A_LNGREF], tema_ant[A_LNGREF];
int i, j;

/* abre fichero de ayuda y de ¡ndices */
strcpy(nf,nfayd);
strcat(nf,ExtAyd);
if((fayd=fopen(nf,"rt"))==NULL) {
	error_ayuda(Merr_Fayd);
	return(0);
}
strcpy(nf,nfayd);
strcat(nf,ExtIyd);
/* si no encuentra fichero de ¡ndices lo crea */
if(access(nf,0)) {
	if(!a_crea_fichero_indices(nfayd)) {
		error_ayuda(Merr_Fiyd);
		fclose(fayd);
		return(0);
	}
}
if((fiyd=fopen(nf,"rb"))==NULL) {
	error_ayuda(Merr_Fiyd);
	fclose(fayd);
	return(0);
}

/* inicializa tema anterior */
*tema_ant='\0';

/* si no se di¢ un tema, muestra el ¡ndice */
if(!*tema) {
	busca_tema(fiyd,fayd,"");
	c_abre(&cind);
	while(1) {
		i=c_gestiona(&cind);
		if(i==1) {			/* seleccionar tema */
			if(!busca_tema(fiyd,fayd,lst_ref1->selecc)) {
				beep();
				busca_tema(fiyd,fayd,"");
			}
			else break;
		}
		else if((i==-1) || (i==2)) {	/* salir */
			c_borra_lista(lst_ayd);
			c_borra_lista(lst_ref1);
			c_borra_lista(lst_ref2);
			c_cierra(&cind);
			return(1);
		}
	}
	c_cierra(&cind);
}
else {
	/* busca tema y lo guarda por si hay que retroceder a ‚l */
	if(!busca_tema(fiyd,fayd,tema)) {
		beep();
		fclose(fiyd);
		fclose(fayd);
		return(1);
	}
	else strcpy(tema_ant,tema);
}

c_abre(&cayd);
while(1) {
	i=c_gestiona(&cayd);
	if(i==2) {				/* indice */
		busca_tema(fiyd,fayd,"");
		c_cierra(&cayd);
		c_abre(&cind);
		while(1) {
			j=c_gestiona(&cind);
			if(j==1) {			/* seleccionar tema */
				if(!busca_tema(fiyd,fayd,lst_ref1->selecc)) {
					beep();
					busca_tema(fiyd,fayd,"");
				}
				else break;
			}
			else if((j==-1) || (j==2)) {	/* salir */
				c_borra_lista(lst_ayd);
				c_borra_lista(lst_ref1);
				c_borra_lista(lst_ref2);
				c_cierra(&cind);
				return(1);
			}
		}
		c_cierra(&cind);
		c_abre(&cayd);
	}
	else if(i==3) {				/* ir a referencia */
		c_cierra(&cayd);

		/* guarda tema actual y busca ayuda sobre */
		/* referencia seleccionada */
		strcpy(tema_act,(lst_ayd->elemento)->cadena);
		if(!busca_tema(fiyd,fayd,lst_ref2->selecc)) {
			beep();
			busca_tema(fiyd,fayd,tema_act);
		}
		else strcpy(tema_ant,tema_act);

		c_abre(&cayd);
	}
	else if(i==4) {				/* anterior */
		if(*tema_ant) busca_tema(fiyd,fayd,tema_ant);
	}
	else if((i==-1) || (i==5)) break;	/* salir */
}
c_cierra(&cayd);

c_borra_lista(lst_ayd);
c_borra_lista(lst_ref1);
c_borra_lista(lst_ref2);
fclose(fiyd);
fclose(fayd);

return(1);
}
