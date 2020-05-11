/****************************************************************************
				   SONIDO.C

	Conjunto de funciones para generar sonido.

			    (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- bpr_sonido: reproduce un sonido por el altavoz del PC
		- bpr_nota: toca una nota por el altavoz del PC
		- bpr_musica: reproduce un fichero SND por el altavoz del PC
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <dos.h>
#include <bios.h>
#include "sonido.h"

/*** Variables globales internas ***/
/* frecuencias en Hz para las 8 octavas */
static unsigned frec_notas[NUM_NOTAS]={
	/* DO, DO#, RE, RE#, MI, FA, FA#, SOL, SOL#, LA, LA#, SI */
	/* octava 0 */
	16, 17, 18, 19, 21, 22, 23, 24, 26, 27, 29, 31,
	/* octava 1 */
	33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62,
	/* octava 2 */
	65, 69, 73, 78, 82, 87, 92, 98, 104, 110, 117, 123,
	/* octava 3 */
	131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247,
	/* octava 4 */
	262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
	/* octava 5 */
	523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 923, 988,
	/* octava 6 */
	1046, 1109, 1175, 1245, 1329, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
	/* octava 7 */
	2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
};

/****************************************************************************
	BPR_SONIDO: reproduce un sonido por el altavoz del PC.
	  Entrada:	'frec' frecuencia en Hz
			'duraci¢n' duraci¢n en d‚cimas de segundo
****************************************************************************/
void bpr_sonido(unsigned frec, unsigned duracion)
{

sound(frec);
delay(100*duracion);
nosound();

}

/****************************************************************************
	BPR_NOTA: toca una nota por el altavoz del PC.
	  Entrada:	'nota' n£mero de nota a tocar (mirar tabla de
			notas 'frec_notas')
			'duraci¢n' duraci¢n en d‚cimas de segundo
****************************************************************************/
void bpr_nota(int nota, unsigned duracion)
{

bpr_sonido(frec_notas[nota],duracion);

}

/****************************************************************************
	BPR_MUSICA: reproduce un fichero SND por el altavoz del PC.
	  Entrada:	'nf_snd' nombre del fichero SND a reproducir
			'modo' modo de reproducci¢n:
			  SND_SOLO  - solo una vez
			  SND_MULT  - c¡clicamente
	  Salida:	0 si pudo reproducir fichero, c¢digo de error si no
			Errores:
			  E_SND_APER - error de apertura fichero SND
			  E_SND_LECT - error de lectura fichero SND
			  E_SND_FORM - formato de fichero no v lido
			  E_SND_MEM  - no hay suficiente memoria
****************************************************************************/
int bpr_musica(char *nf_snd, int modo)
{
STC_CABSND cab;
STC_SND snd, *mus, *p;
FILE *fsnd;
int i, salida=0;

if((fsnd=fopen(nf_snd,"rb"))==NULL) return(E_SND_APER);

/* lee cabecera */
if(fread(&cab,sizeof(STC_CABSND),1,fsnd)!=1) {
	fclose(fsnd);
	return(E_SND_LECT);
}

/* comprueba si es fichero SND v lido */
if(strcmp(cab.id,SND_ID)) {
	fclose(fsnd);
	return(E_SND_FORM);
}

/* lee fichero SND en memoria */
if((mus=(STC_SND *)malloc(sizeof(STC_SND)*cab.n_snd))==NULL) {
	fclose(fsnd);
	return(E_SND_MEM);
}

p=mus;
for(i=0; i<cab.n_snd; i++, p++) {
	if(fread(&snd,sizeof(STC_SND),1,fsnd)!=1) {
		fclose(fsnd);
		free(mus);
		return(E_SND_LECT);
	}
	p->nota=snd.nota;
	p->duracion=snd.duracion;
}
fclose(fsnd);

/* reproduce m£sica */
while(!salida) {
	p=mus;
	for(i=0; i<cab.n_snd; i++, p++) {
		if(p->nota==SND_SILENCIO) delay(100*p->duracion);
		else bpr_nota(p->nota,p->duracion);
		if(bioskey(1)) {
			bioskey(0);
			salida=1;
			break;
		}
	}
	if(modo==SND_SOLO) salida=1;
}

free(mus);

return(0);
}
