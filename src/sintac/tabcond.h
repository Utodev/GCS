/**************************************
	Fichero de cabecera con
	datos sobre los condactos
	usados por el SINTAC
**************************************/

#if !defined (TABCOND_H)
#define TABCOND_H

/* n£mero de condactos definidos */
#define N_CONDACTOS 121

/* lista de condactos */
struct {
	char cnd[LNGCOND+1];    /* nombre del condacto */
	BYTE tipo;              /* tipo del condacto */
} condacto[N_CONDACTOS]={
	{""       , 0},         /* tipo 0 = sin par metros */
	{"PROCESS", 1},         /* tipo 1 = 'PROCESS' */
	{"DONE   ", 0},
	{"NOTDONE", 0},
	{"RESP   ", 0},
	{"NORESP ", 0},
	{"DEFWIN ", 2},         /* tipo 2 = 'DEFWIN'*/
	{"WINDOW ", 3},         /* tipo 3 = n§ de ventana */
	{"CLW    ", 3},
	{"LET    ", 4},         /* tipo 4 = (byte), (byte) */
	{"EQ     ", 4},
	{"NOTEQ  ", 4},
	{"LT     ", 4},
	{"GT     ", 4},
	{"MES    ", 5},		/* tipo 5 = (byte), n§ mensaje */
	{"NEWLINE", 0},
	{"MESSAGE", 5},
	{"SYSMESS", 6},         /* tipo 6 = n§ mensaje del sistema */
	{"DESC   ", 7},         /* tipo 7 = n§ localidad */
	{"ADD    ", 4},
	{"SUB    ", 4},
	{"INC    ", 8},         /* tipo 8 = (byte) */
	{"DEC    ", 8},
	{"SET    ", 8},
	{"CLEAR  ", 8},
	{"ZERO   ", 8},
	{"NOTZERO", 8},
	{"PLACE  ", 9},         /* tipo 9 = n§ objeto, n§ localidad */
	{"GET    ", 10},        /* tipo 10 = n§ objeto */
	{"DROP   ", 10},
	{"INPUT  ", 0},
	{"PARSE  ", 0},
	{"SKIP   ", 11},        /* tipo 11 = 'SKIP' */
	{"AT     ", 8},
	{"NOTAT  ", 8},
	{"ATGT   ", 8},
	{"ATLT   ", 8},
	{"ADJECT1", 13},        /* tipo 13 = n§ adjetivo */
	{"NOUN2  ", 14},        /* tipo 14 = n§ nombre */
	{"ADJECT2", 13},
	{"LISTAT ", 12},        /* tipo 12 = n§ localidad+ */
	{"ISAT   ", 9},
	{"ISNOTAT", 9},
	{"PRESENT", 10},
	{"ABSENT ", 10},
	{"WORN   ", 10},
	{"NOTWORN", 10},
	{"CARRIED", 10},
	{"NOTCARR", 10},
	{"WEAR   ", 10},
	{"REMOVE ", 10},
	{"CREATE ", 10},
	{"DESTROY", 10},
	{"SWAP   ", 15},        /* tipo 15 = n§ objeto, n§ objeto */
	{"RESTART", 0},
	{"WHATO  ", 0},
	{"MOVE   ", 8},
	{"ISMOV  ", 0},
	{"GOTO   ", 7},
	{"PRINT  ", 8},
	{"DPRINT ", 8},
	{"CLS    ", 0},
	{"ANYKEY ", 0},
	{"PAUSE  ", 8},
	{"LISTOBJ", 0},
	{"FIRSTO ", 0},
	{"NEXTO  ", 12},
	{"SYNONYM", 16},        /* tipo 16 = 'SYNONYM' */
	{"HASAT  ", 17},        /* tipo 17 = bandera usuario objeto */
	{"HASNAT ", 17},
	{"LIGHT  ", 0},
	{"NOLIGHT", 0},
	{"RANDOM ", 4},
	{"SEED   ", 8},
	{"PUTO   ", 12},
	{"INKEY  ", 0},
	{"COPYOV ", 18},        /* tipo 18 = n§ objeto, (byte) */
	{"CHANCE ", 19},        /* tipo 19 = valor 0-100 'CHANCE' */
	{"RAMSAVE", 20},        /* tipo 20 = n§ banco RAM */
	{"RAMLOAD", 21},        /* tipo 21 = n§ banco RAM, (byte), (byte) */
	{"ABILITY", 8},
	{"AUTOG  ", 0},
	{"AUTOD  ", 0},
	{"AUTOW  ", 0},
	{"AUTOR  ", 0},
	{"ISDOALL", 0},
	{"ASK    ", 22},        /* tipo 22 = n§ mens. sist., n§ mens. sist. */
	{"QUIT   ", 0},
	{"SAVE   ", 0},
	{"LOAD   ", 4},
	{"EXIT   ", 23},        /* tipo 23 = valor 0-1 'EXIT' */
	{"END    ", 0},
	{"PRINTAT", 4},
	{"SAVEAT ", 0},
	{"BACKAT ", 0},
	{"NEWTEXT", 0},
	{"PRINTC ", 8},
	{"INK    ", 8},
	{"PAPER  ", 8},
	{"BRIGHT ", 26},        /* tipo 26 = atributo parpadeo o brillo */
	{"BLINK  ", 26},
	{"COLOR  ", 8},
	{"DEBUG  ", 23},
	{"WBORDER", 27},        /* tipo 27 = n§ ventana, n§ borde */
	{"CHARSET", 8},
	{"EXTERN ", 4},
	{"GTIME  ", 8},
	{"TIME   ", 28},        /* tipo 28 = (byte), 0-1 */
	{"TIMEOUT", 0},
	{"MODE   ", 23},
	{"GRAPHIC", 31},	/* tipo 31 = (byte), (byte), (byte) */
	{"REMAPC ", 29},        /* tipo 29 = color, r, g, b */
	{"SETAT  ", 17},
	{"CLEARAT", 17},
	{"GETRGB ", 30},        /* tipo 30 = (byte), (byte), (byte), (byte) */
	{"PLAYFLI", 28},
	{"MOUSE  ", 23},
	{"MOUSEXY", 30},
	{"MOUSEBT", 8},
	{"PLAY   ", 24},	/* tipo 24 = nota, (byte) */
	{"MUSIC  ", 28},
};

#endif  /* TABCOND_H */
