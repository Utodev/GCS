/**************************************
	Fichero de cabecera de
	las funciones de manejo
	del rat¢n de RATON.C
**************************************/

#if !defined (RATON_H)
#define RATON_H

/*** Constantes ***/
#define R_NOVAL		9999	/* valor para rat¢n no inicializado */
#define R_MUESTRA       1       /* para mostrar puntero de rat¢n */
#define R_OCULTA        2       /* para ocultar puntero de rat¢n */

/*** Tipos de datos y estructuras ***/
typedef struct {
	int boton1, boton2;     /* estado de los botones del rat¢n */
				/* 1 pulsado, 0 suelto */
	int xv, yv;           	/* posici¢n virtual del rat¢n */
	int fil, col;         	/* fila y columna del rat¢n */
} STC_RATON;

/*** Prototipos ***/
int r_inicializa(void);
int r_puntero(int modo);
int r_estado(STC_RATON *raton);
int r_pon_puntero(int fil, int col);
void r_desconecta(void);

#endif  /* RATON_H */
