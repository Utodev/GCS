#include "aatypes.h"
#include "aaerr.h"

static char *err_msgs[]={
	"",
	"Error desconocido",
	"No hay suficiente memoria",
	"Fichero no encontrado",
	"No se puede crear fichero",
	"Tipo de fichero no v lido",
	"Fichero da¤ado internamente",
	"Fichero no es de 320x200",
	"Fichero demasiado peque¤o",
	"Imposible escribir todo el fichero",
	"Error en posicionamiento en fichero",
};

char *fli_error_message(Errval err)
{

if(err>=0) err=0;
else err=-err;

if(err>Array_els(err_msgs)) err=AA_ERR_MISC;

return(err_msgs[err]);
}
