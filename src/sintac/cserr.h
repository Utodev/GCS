/**************************************
      Fichero de cabecera con los
      c¢digos de error del compilador
**************************************/

#if !defined (CSERR_H)
#define CSERR_H

/* c¢digos de error del compilador */
typedef enum {
	_E_NERR=0,      /* c¢digo de no error */
	_E_CIND,        /* c¢digo de indirecci¢n */
	_E_TABC,        /* TABLA DE CONDACTOS ERRONEA */
	_E_FTMP,        /* error apertura fichero temporal */
	_E_ETMP,        /* error de escritura fichero temporal */
	_E_LTMP,        /* error de lectura fichero temporal */
	_E_AFIN,        /* error apertura fichero de entrada */
	_E_LFIN,        /* error de lectura fichero de entrada */
	_E_EOFI,        /* fin de fichero de entrada */
	_E_AFOU,        /* error de apertura de fichero de salida */
	_E_EFOU,        /* error de escritura en fichero de salida */
	_E_MMEM,        /* no hay suficiente memoria para ejecutar programa */
	_E_SCCI,        /* secci¢n no v lida */
	_E_SCRP,        /* secci¢n repetida */
	_E_MSCC,        /* marca de secci¢n no encontrada */
	_E_MEND,        /* marca \END no encontrada */
	_E_NOSC,        /* faltan secciones */
	_E_MXXX,        /* INDICA QUE QUEDAN TABLAS DE MENSAJES POR COMPILAR */
	_E_CVOC,        /* car cter no v lido en palabra */
	_E_FCAM,        /* faltan campos */
	_E_NPAL,        /* n£mero de palabra no v lido */
	_E_CNUM,        /* campo num‚rico no v lido */
	_E_TPAL,        /* tipo de palabra no v lido */
	_E_PREP,        /* palabra repetida */
	_E_MVOC,        /* vocabulario lleno */
	_E_FALT,        /* falta '@' */
	_E_NFSC,        /* n£mero fuera de secuencia */
	_E_FMEM,        /* rebasada memoria reservada para secci¢n actual */
	_E_NVAL,        /* n£mero no v lido */
	_E_NPVC,        /* palabra no est  en vocabulario */
	_E_NNVC,        /* nombre no est  en vocabulario */
	_E_NAVC,        /* adjetivo no est  en vocabulario */
	_E_NMOV,        /* la palabra no es verbo de movimiento */
	_E_NNOM,        /* el primer campo debe ser un nombre */
	_E_NADJ,        /* el segundo campo debe ser un adjetivo */
	_E_LINC,        /* localidad inicial no v lida */
	_E_BAND,        /* banderas de objeto no v lidas */
	_E_OTEX,        /* falta texto de objeto */
	_E_NENT,        /* comienzo de entrada no v lido */
	_E_RBTL,        /* tabla de etiquetas llena */
	_E_EREP,        /* etiqueta repetida */
	_E_ENVL,        /* nombre de etiqueta no v lido */
	_E_NVNC,        /* primera palabra entrada no es verbo ni nomb.conv. */
	_E_NNNN,        /* segunda palabra entrada no es nombre */
	_E_NCND,        /* condacto no v lido */
	_E_PRPR,        /* un proceso no puede llamarse a s¡ mismo */
	_E_PRRB,        /* rebasado m ximo n£mero de llamadas 'forward' */
	_E_WPNW,        /* n£mero de ventana no v lido */
	_E_TMSG,	/* n£mero de tabla de mensajes no v lido */
	_E_MPNM,        /* n£mero de mensaje no v lido */
	_E_MPNL,        /* n£mero de localidad no v lido */
	_E_OPNV,        /* n£mero de objeto no v lido */
	_E_NLAB,        /* campo de etiqueta no v lido */
	_E_RBTS,        /* rebasado m ximo num. saltos 'forward' SKIP */
	_E_LBFR,        /* salto fuera de rango */
	_E_NCAD,        /* adjetivo no v lido */
	_E_NNMB,        /* nombre no v lido */
	_E_NVHT,        /* bandera de usuario no v lida */
	_E_CHNV,        /* par metro CHANCE debe estar entre 0 y 100 */
	_E_RBNV,        /* banco de memoria no v lido */
	_E_MPNP,        /* n£mero de 1er mensaje del sistema no v lido */
	_E_MPNS,        /* n£mero de 2§ mensaje del sistema no v lido */
	_E_NO01,        /* par metro debe ser 0 o 1 */
	_E_COLR,        /* color incorrecto */
	_E_CLOC,        /* conexi¢n a localidad no v lida */
	_E_RFFW,        /* etiqueta no definida */
	_E_LFFR,        /* salto 'forward' fuera de rango */
	_E_PRLL,        /* llamada a proceso inexistente */
	_E_TCCN,        /* tabla de constantes llena */
	_E_PCCN,        /* primer car cter de nombre de constante no v lido */
	_E_FVCN,        /* falta valor de constante */
	_E_CTND,        /* constante no definida */
	_E_NNCT,        /* nombre de constante no v lido */
	_E_VNCT,        /* error en valor de constante */
	_E_NBWN,        /* n£mero de borde de ventana no v lido */
	_E_SP01,        /* segundo par metro debe ser 0 o 1 */
	_E_CRGB,        /* componente RGB debe estar entre 0 y 63 */
	_E_NOTA,	/* n£mero de nota no v lido */
} COD_ERR;

#endif  /* CSERR_H */
