/**************************************
	Fichero de cabecera para
	las funciones de manejo de
	los condactos de CONDACTO.C
**************************************/

#if !defined (CONDACTO_H)
#define CONDACTO_H

/*** Constantes ***/
#define STK     100             /* profundidad de la pila del int‚rprete */

#define SEPARADOR   0
#define FIN_FRASE   1
#define PALABRA     2
#define TERMINACION 3
#define NO_PALABRA  4

/* pulsaciones por segundo del reloj del sistema */
#define TICKS_SEG   18          /* exactamente son 18.2 */

/*** Tipos de datos y estructuras ***/
/* definici¢n de condacto, funci¢n-n£mero de par metros */
typedef struct condacto {
	BOOLEAN (*cond)();      /* puntero a funci¢n del condacto */
	BYTE npar;              /* n£mero de par metros */
} STC_CONDACTO;

/* bancos para RAMSAVE y RAMLOAD */
typedef struct {
	BYTE bram[FLAGS+MAX_OBJ];
	char *tab_obj;          /* G3.25: para guardar la tabla de objetos */
	BOOLEAN usado;
} STC_BANCORAM;

/*** Prototipos ***/
void vv_imps2(char *s, STC_VV *v);
unsigned vv_inputt(char *plin, unsigned maxlong, char cursor, int conver,
  STC_VV *v, int tiempo, int modot);
BOOLEAN esta_en(char *s, char c);
int esta_en_voc(char *pal);
int analiza(char *(*pfrase), BYTE *tipo, BYTE *num);
BOOLEAN parse1(void);
void pausa(clock_t p);
int carga_def(char *nombre);
int carga_tabla_mes(BYTE nt);
void setflag(BYTE nv, BYTE value);
BYTE getflag(BYTE nv);
BOOLEAN getflagbit(BYTE nf, BYTE nb);
void setflagbit(BYTE nv, BYTE nb);
void clearflagbit(BYTE nf, BYTE nb);


BOOLEAN process(BYTE prc);
BOOLEAN done(void);
BOOLEAN notdone(void);
BOOLEAN resp(void);
BOOLEAN noresp(void);
BOOLEAN defwin(BYTE nw, BYTE cwf, BYTE cw, BYTE wy, BYTE wx, BYTE lx, BYTE ly);
BOOLEAN window(BYTE nw);
BOOLEAN clw(BYTE nw);
BOOLEAN let(BYTE nv,BYTE val);
BOOLEAN eq(BYTE nv, BYTE val);
BOOLEAN noteq(BYTE nv, BYTE val);
BOOLEAN lt(BYTE nv, BYTE val);
BOOLEAN gt(BYTE nv, BYTE val);
BOOLEAN mes(BYTE nt, BYTE nm);
BOOLEAN newline(void);
BOOLEAN message(BYTE nt, BYTE nm);
BOOLEAN sysmess(BYTE nm);
BOOLEAN desc(BYTE nl);
BOOLEAN add(BYTE nv, BYTE val);
BOOLEAN sub(BYTE nv, BYTE val);
BOOLEAN inc(BYTE nv);
BOOLEAN dec(BYTE nv);
BOOLEAN set(BYTE nf);
BOOLEAN bset(BYTE nb);
BOOLEAN clear(BYTE nf);
BOOLEAN bclear(BYTE nf);
BOOLEAN zero(BYTE nf);
BOOLEAN notzero(BYTE nf);
BOOLEAN bzero(BYTE nf);
BOOLEAN bnotzero(BYTE nf);
BOOLEAN place(BYTE nobj, BYTE nloc);
BOOLEAN get(BYTE nobj);
BOOLEAN drop(BYTE nobj);
BOOLEAN input(void);
BOOLEAN parse(void);
BOOLEAN skip(BYTE lsb, BYTE hsb);
BOOLEAN at(BYTE locno);
BOOLEAN notat(BYTE locno);
BOOLEAN atgt(BYTE locno);
BOOLEAN atlt(BYTE locno);
BOOLEAN adject1(BYTE adj);
BOOLEAN noun2(BYTE nomb);
BOOLEAN adject2(BYTE adj);
BOOLEAN listat(BYTE locno);
BOOLEAN isat(BYTE nobj, BYTE locno);
BOOLEAN isnotat(BYTE nobj, BYTE locno);
BOOLEAN present(BYTE nobj);
BOOLEAN absent(BYTE nobj);
BOOLEAN worn(BYTE nobj);
BOOLEAN notworn(BYTE nobj);
BOOLEAN carried(BYTE nobj);
BOOLEAN notcarr(BYTE nobj);
BOOLEAN wear(BYTE nobj);
BOOLEAN remove1(BYTE nobj);
BOOLEAN create(BYTE nobj);
BOOLEAN destroy(BYTE nobj);
BOOLEAN swap(BYTE nobj1, BYTE nobj2);
BOOLEAN restart(void);
BOOLEAN whato(void);
BOOLEAN move(BYTE nv);
BOOLEAN ismov(void);
BOOLEAN goto1(BYTE locno);
BOOLEAN print(BYTE nv);
BOOLEAN dprint(BYTE nv);
BOOLEAN cls(void);
BOOLEAN anykey(void);
BOOLEAN pause(BYTE pau);
BOOLEAN listobj(void);
BOOLEAN firsto(void);
BOOLEAN nexto(BYTE locno);
BOOLEAN synonym(BYTE verb, BYTE nomb);
BOOLEAN hasat(BYTE val);
BOOLEAN hasnat(BYTE val);
BOOLEAN light(void);
BOOLEAN nolight(void);
BOOLEAN random1(BYTE varno, BYTE rnd);
BOOLEAN seed(BYTE seed);
BOOLEAN puto(BYTE nloc);
BOOLEAN inkey(void);
BOOLEAN copyof(BYTE nobj, BYTE varno);
BOOLEAN chance(BYTE rnd);
BOOLEAN ramsave(BYTE banco);
BOOLEAN ramload(BYTE banco, BYTE ftop);
BOOLEAN ability(BYTE nobjs);
BOOLEAN autog(void);
BOOLEAN autod(void);
BOOLEAN autow(void);
BOOLEAN autor(void);
BOOLEAN isdoall(void);
BOOLEAN ask(BYTE smess1, BYTE smess2, BYTE varno);
BOOLEAN quit(void);
BOOLEAN save(void);
BOOLEAN load(BYTE ftop);
BOOLEAN exit1(BYTE ex);
BOOLEAN end1(void);
BOOLEAN printat(BYTE y, BYTE x);
BOOLEAN saveat(void);
BOOLEAN backat(void);
BOOLEAN newtext(void);
BOOLEAN printc(BYTE car);
BOOLEAN ink(BYTE color);
BOOLEAN paper(BYTE color);
BOOLEAN bright(BYTE b);
BOOLEAN blink(BYTE b);
BOOLEAN color(BYTE col);
BOOLEAN debug(BYTE modo);
BOOLEAN wborder(BYTE nw, BYTE borde);
BOOLEAN charset(BYTE set);
BOOLEAN extern1(BYTE prg, BYTE par);
BOOLEAN gtime(BYTE nv);
BOOLEAN time1(BYTE tiempo, BYTE modo);
BOOLEAN timeout(void);
BOOLEAN mode(BYTE modo);
BOOLEAN graphic(BYTE graf, BYTE img, BYTE modo);
BOOLEAN remapc(BYTE color, BYTE r, BYTE g, BYTE b);
BOOLEAN setat(BYTE val);
BOOLEAN clearat(BYTE val);
BOOLEAN getrgb(BYTE color, BYTE var1, BYTE var2, BYTE var3);
BOOLEAN playfli(BYTE fli, BYTE modo);
BOOLEAN mouse(BYTE modo);
BOOLEAN mousexy(BYTE fil0, BYTE col0, BYTE fil1, BYTE col1);
BOOLEAN mousebt(BYTE btn);
BOOLEAN play(BYTE nota, BYTE dur);
BOOLEAN music(BYTE mus, BYTE modo);

#endif  /* CONDACTO_H */
