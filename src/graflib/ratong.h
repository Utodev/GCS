/**************************************
	Fichero de cabecera de
	las funciones de manejo
	del rat¢n de RATONG.C
**************************************/

#if !defined (RATONG_H)
#define RATONG_H

/*** Constantes ***/
#define RG_NOVAL	9999	/* valor para rat¢n no inicializado */
#define RG_MUESTRA      1       /* para mostrar puntero de rat¢n */
#define RG_OCULTA      	2       /* para ocultar puntero de rat¢n */

/*** Tipos de datos y estructuras ***/
typedef struct {
	int boton1, boton2;     /* estado de los botones del rat¢n */
				/* 1 pulsado, 0 suelto */
	int xv, yv;           	/* coordenadas virtuales del rat¢n */
	int x, y;		/* coordenadas reales del rat¢n */
	int fil, col;         	/* fila y columna del rat¢n */
} STC_RATONG;

/*** Prototipos ***/
int rg_inicializa(void);
int rg_puntero(int modo);
int rg_estado(STC_RATONG *raton);
int rg_pon_puntero(int x, int y);
void rg_desconecta(void);

#endif  /* RATONG_H */
