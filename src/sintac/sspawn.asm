;
; SSPAWN
;
; El prototipo para llamar a la funci¢n desde C es:
;
;	unsigned int sspawn(char *comando, char *param, char *ruta);
;
;		- 'comando'	comando a ejecutar (NULL o cadena vac¡a
;				para COMMAND.COM), se debe a¤adir la
;				extensi¢n .EXE o .COM
;		- 'param' 	par metros que se pasan
;		- 'ruta' 	ruta para fichero temporal
;
; La funci¢n devuelve el c¢digo de retorno del programa ejecutado o uno
; de estos valores si hubo error:
;
;       0x8001 = imposible encontrar la variable de entorno COMSPEC
;       0x8002 = imposible crear fichero temporal
;       0x8003 = error de escritura en fichero temporal
;       0x8004 = imposible liberar exceso de memoria
;       0x8005 = error al intentar ejecutar comando
;

IDEAL

MODEL LARGE,C

EXTRN   _psp:word

CODESEG

STRUC exec_pb
   environment  DW ?
   cmdline      DD ?
   fcb1         DD ?
   fcb2         DD ?
ENDS

STRUC MCB
   type         DB ?
   owner        DW ?
   size         DW ?
   unused       DB 3 dup (?)
   dos4         DB 8 dup (?)
ENDS

stkseg          DW ?
stkptr          DW ?
newstk          DW 128 dup (?)
ourmcb          DW ?
handle          DW ?
pageframe       DW ?
pagenum         DW ?
count           DW ?
srcseg          DW ?
keep            DW ?
orgsize         DW ?
retval          DW ?
prgname         DB 128 dup (?)
params          DB 128 dup (?)
fcb1            DB 0
                DB 11 dup (' ')
                DB 25 dup (0)
fcb2            DB 0
		DB 11 dup (' ')
		DB 25 dup (0)
cmdcom          DB 'COMSPEC=',0
ems_device_name DB 'EMMXXXX0', 0
filespec        DB 128 dup (0)
errmsg1         DB 'Imposible recuperar memoria original.',0DH,0AH,'$'
errmsg2         DB 'Imposible restaurar programa desde disco.',0DH,0AH,'$'
exec_parms      exec_pb ?


PROC    sspawn
	ARG     cmd:DWORD
	ARG	param:DWORD
	ARG     path:DWORD

	push    ds              ; guarda estado
	push    si
	push    di
	cld
;
; Primero hay que copiar los par metros pasados en la cola de comandos que
; ser  usada cuando llamemos a la funci¢n EXEC.
;
	push    cs
	pop     es
	mov     di,offset params       	; incializa cola comandos
	xor     al,al
	stosb
	lds     si,[param]              ; coge puntero a par metros
	mov     ax,ds                   ; coge segmento
	or      ax,si                   ; ¨es NULL?
	jz      get_cmd
	mov     al,[si]                 ; ¨es cadena vac¡a?
	or      al,al
	jz      get_cmd

	mov     cx,127                  ; m xima longitud de cola de par metros
copy_par:
	lodsb                           ; a¤ade par metros a cola
	or      al,al
	jz      par_copied
	stosb
	loop    copy_par
par_copied:
	mov     al,13                   ; a¤ade fin de l¡nea
	stosb
	mov     ax,127                  ; calcula longitud
	sub     ax,cx
	mov     [cs:params],al
;
; Creamos entorno para ejecutar el comando especificado. Si es NULL o cadena
; vac¡a ejecutaremos el COMMAND.COM con los par metros especificados.
;
get_cmd:
	lds     si,[cmd]              	; coge puntero a comando
	mov     ax,ds                   ; coge segmento
	or      ax,si                   ; ¨es NULL?
	jz      get_cmdcom
	mov     al,[si]                 ; ¨es cadena vac¡a?
	or      al,al
	jz      get_cmdcom

	push    cs
	pop     ds
	mov     ax,SEG _psp
	mov     es,ax
	mov     ax,[es:_psp]            ; coger nuestro segmento PSP
	mov     es,ax
	dec     ax
	mov     [ourmcb],ax
	mov     ax,[es:002CH]           ; coger segmento del entorno
	mov     [exec_parms.environment],ax

	lds     si,[cmd]              	; coge puntero a comando
	push	cs
	pop	es
	mov 	di,offset prgname
copy_cmd:
	lodsb                           ; coge car cter del comando
        or      al,al
        stosb
        jnz     copy_cmd                ; contin£a si no acab¢
	jmp 	save_mem
;
; Buscamos a trav‚s de las variables de entorno y encontrar donde est  el
; COMMAND.COM. Necesitamos esto para pas rselo a la funci¢n EXEC.
;
get_cmdcom:
	push    cs
	pop     ds
	mov     ax,SEG _psp
	mov     es,ax
	mov     ax,[es:_psp]            ; coger nuestro segmento PSP
	mov     es,ax
	dec     ax
	mov     [ourmcb],ax
	mov     ax,[es:002CH]           ; coger segmento del entorno
	mov     [exec_parms.environment],ax
	mov     es,ax
        xor     di,di                   ; comenzar b£squeda al inicio del entorno
genv1:
        mov     si,offset cmdcom        ; puntero a nombre a buscar
        cmp     [byte ptr es:di],0      ; ¨fin del entorno?
        jne     genv2
        mov     ax,8001H
        jmp     error
genv2:
        lodsb                           ; coge car cter del nombre
        or      al,al                   ; ¨fin del nombre?
        jz      copy_cmdcom
        cmp     al,[es:di]              ; ¨siguiente car cter coincide?
        jne     genv3                   ; no es el mismo, salta al siguiente
        inc     di
        jmp     genv2                   ; contin£a la comparaci¢n
genv3:
        xor     al,al                   ; busca el inicio de la siguiente cadena
        mov     cx,0FFFFH               ; en la tabla de entorno
        repnz   scasb
        jmp     genv1
copy_cmdcom:
        push    es                      ; intercambia registros de segmento
        pop     ds
        push    cs
        pop     es
        mov     si,di
        mov     di,offset prgname
nextchar:
        lodsb                           ; coge car cter de la cadena de entorno
        or      al,al
        stosb
        jnz     nextchar                ; contin£a si no acab¢
;
; Ahora tenemos que guardar todo los que hay tras este m¢dulo hasta el final
; de nuestro espacio de memoria a EMS si hay suficiente, o a un fichero
; temporal.
;
save_mem:
        push    cs
        pop     ds
        mov     ax,[ourmcb]             ; apunta a nuestro MCB
        mov     es,ax
        xor     bx,bx
        mov     cx,[es:(MCB ptr bx).size] ; tama¤o de nuestra memoria en p rrafos
        mov     [orgsize],cx            ; lo guarda para recuperarlo despu‚s
        mov     ax,offset last_addr     ; calcula el n£mero de p rrafos a guardar
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
        mov     bx,ax
        mov     ax,cs
        add     ax,bx
        mov     [srcseg],ax             ; guarda p rrafo inicial
        sub     ax,[ourmcb]
        mov     [keep],ax               ; guarda cantidad a mantene
        dec     ax                      ; tiene en cuenta el MCB
        sub     cx,ax                   ; CX=n£mero de p rrafos
        mov     [count],cx              ; los guarda
;
; Comprueba si hay un 'driver' EMS cargado.
;
        mov     ax,3D00H                ; abre fichero, s¢lo lectura
        mov     dx,offset ems_device_name
        int     21H
        jnc     emsck1
        jmp     use_file
emsck1: mov     bx,ax
        push    bx
        mov     ax,4400H                ; coge informaci¢n del 'driver'
        int     21H
        pop     bx
        jnc     emsck2
        jmp     use_file
emsck2: test    dx,0080H                ; ¨es un 'driver'?
        jnz     emsck3
        jmp     ems_no_driver
emsck3: push    bx
        mov     ax,4407H                ; ¨est  preparado?
        int     21H
        pop     bx
        or      al,al
        jnz     emsck4
        jmp     ems_no_driver
emsck4: mov     ax,3E00H                ; cierra el 'driver'
        int     21H
;
; Comprueba la versi¢n EMS, si no es 3.0 o superior no usa EMS.
;
        mov     ah,46H
        int     67H
        cmp     al,30H
        jge     emsck5
        jmp     use_file
;
; Coge el 'pageframe' y lo guarda.
;
emsck5: mov     ah,41H
        int     67H
        mov     [pageframe],bx
        or      ah,ah
        jz      emsck6
        jmp     use_file
;
; Comprueba si hay suficiente memoria EMS para guardar nuestra memoria.
;
emsck6: mov     ah,42H
        int     67H
        mov     ax,[count]              ; convierte de p rrafos a p ginas EMS
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
	shr 	ax,1
        inc     ax
        cmp     ax,bx
        jle     emsck7
        jmp     use_file
;
; Reserva el n£mero de p ginas EMS que necesitamos.
;
emsck7: mov     bx,ax                   ; AX=n£mero de p ginas
        mov     ah,43H
        int     67H
        mov     [handle],dx
        mov     ah,47H                  ; guarda contexto de mapeado
        int     67H
        push    [srcseg]                ; para que podamos restaurarlo
        push    [count]
        mov     [pagenum],0
        mov     ax,[pageframe]
        mov     es,ax
emsnextblock:
        mov     ax,[cs:srcseg]          ; DS=siguiente bloque de datos
        mov     ds,ax
        mov     ax,[cs:count]           ; calcular n£mero de p rrafos a copiar
        or      ax,ax
        jz      emsdone
        cmp     ax,400H
        jc      emsnb1
        mov     ax,400H
emsnb1: sub     [cs:count],ax
        add     [cs:srcseg],ax
	shl	ax,1			; calcular n£mero de palabras
	shl	ax,1
	shl	ax,1
        mov     cx,ax
        mov     bx,[cs:pagenum]         ; siguiente p gina
        mov     dx,[cs:handle]
        mov     ax,4400H
        push    cx
        int     67H
        pop     cx
        xor     di,di                   ; copia datos a p gina EMS
        xor     si,si
        repnz   movsw
        inc     [cs:pagenum]
        jmp     emsnextblock
emsdone:
        push    cs                      ; recupera DS
        pop     ds
        pop     [count]
        pop     [srcseg]

        jmp     relmem

ems_no_driver:
        mov  ax,3E00H                   ; cierra 'driver' EMS y usa fichero
        int  21H
;
; Guarda el resto de la memoria en un fichero.
;
use_file:
        mov     [pageframe],0           ; indicador de que no usamos EMS
        mov     [filespec],0
        push    cs
        pop     es
        mov     di,offset filespec
        lds     si,[path]               ; comprueba ruta de fichero
        mov     ax,ds
        or      ax,si                   ; ¨es NULL?
        jz      createfile
copypath:
        lodsb                           ; copia la ruta
	or      al,al
        stosb
        jnz     copypath
createfile:
        push    cs
        pop     ds
        mov     ah,5AH                  ; crea fichero temporal
        xor     cx,cx
        mov     dx,offset filespec
        int     21H
        jnc     save2file
        mov     ax,8002H
        jmp     error
save2file:
        mov     [handle],ax             ; guarda el 'handle'
        push    [srcseg]                ; guarda para recuperar despu‚s
        push    [count]
nextblock:
        mov     ax,[cs:srcseg]          ; DS=siguiente bloque de datos
        mov     ds,ax
        mov     ax,[cs:count]           ; calcula cuantos p rrafos a escribir
        or      ax,ax
        jz      closefile
        cmp     ax,800H
        jc      nb1
        mov     ax,800H
nb1:    sub     [cs:count],ax
        add     [cs:srcseg],ax
	shl 	ax,1			; calcula n£mero de bytes
	shl 	ax,1
	shl 	ax,1
	shl 	ax,1
        mov     cx,ax
        xor     dx,dx
        mov     ah,40H
        mov     bx,[cs:handle]
        int     21H
        jnc     nextblock
        add     sp,4
        mov     ax,8003H
        jmp     error
closefile:
        push    cs                      ; recupera DS y cierra el fichero
        pop     ds
        pop     [count]
        pop     [srcseg]
        mov     bx,[handle]
        mov     ah,3EH
        int     21H
        jnc     relmem
        mov     ax,8004H
        jmp     error
;
; Lo siguiente que hay que hacer es guardar el puntero de la pila y el
; segmento, y despu‚s inicializarlo a nuestra zona de almacenamiento interna.
; Despu‚s cambiar el tama¤o de nuestro bloque de memoria para liberar toda la
; memoria sin usar (que ha sido guardada).
;
relmem:
        mov     ax,ss                   ; guarda el segmento de la pila
        mov     [stkseg],ax
        mov     [stkptr],sp             ; guarda el puntero de la pila
        cli                             ; inicializa pila a nuestra zona interna
        mov     sp,offset newstk+256
        mov     ax,cs
        mov     ss,ax
        sti
        mov     ax,[ourmcb]
        inc     ax
        mov     es,ax
        mov     ah,4AH                  ; cambia tama¤o de nuestro bloque de
        mov     bx,[keep]               ; memoria para liberar lo que no necesitamos
        int     21H
;
; Ejecuta el comando usando la funci¢n EXEC.
;
        mov     [retval],0
        mov     ax,offset params
        mov     [word ptr exec_parms.cmdline],ax
        mov     ax,offset fcb1
        mov     [word ptr exec_parms.fcb1],ax
        mov     ax,offset fcb2
        mov     [word ptr exec_parms.fcb2],ax
        mov     ax,cs
        mov     [word ptr (exec_parms.cmdline)+2],ax
        mov     [word ptr (exec_parms.fcb1)+2],ax
        mov     [word ptr (exec_parms.fcb2)+2],ax
        mov     es,ax
        mov     dx,offset prgname
        mov     bx,offset exec_parms
        mov     ax,4B00H
        int     21H
        jc      execerr
	mov	ah,4DH			; coge el c¢digo de retorno
	int	21H			; del programa ejecutado y lo
	mov	ah,0			; guarda para devolverlo
	mov	[cs:retval],ax
	jmp 	restmem
execerr:
	mov     [cs:retval],8005H
;
; Cambia la memoria a su tama¤o original y restaura su contenido.
;
restmem:
        push    cs
        pop     ds
        mov     ax,[ourmcb]
        inc     ax
        mov     es,ax
        mov     ah,4AH                  ; cambia tama¤o de la memoria
        mov     bx,[orgsize]            ; a su tama¤o original
        int     21H
        jnc     restmem2
	mov     dx,offset errmsg1       ; ­no se puede recuperar memoria­
        mov     ah,9                    ; avisa
        int     21H
        mov     ax,4C01H
        int     21H                     ; y sale
;
; Si hemos usado EMS recuperamos de ah¡, si no recuperamos de fichero.
;
restmem2:
        test    [pageframe],0FFFFH      ; ¨en EMS?
        jz      restfile                ; si no recupera de fichero
        mov     [pagenum],0
        mov     ax,[pageframe]          ; coge segmento de 'pageframe' de EMS
        mov     ds,ax
emsreadblock:
        mov     ax,[cs:srcseg]          ; ES=siguiente bloque de datos
        mov     es,ax
        mov     ax,[cs:count]           ; calcula p rrafos a copiar
        or      ax,ax
        jz      emsrdone
        cmp     ax,400H                 ; si quedan m s de 16K
        jc      emsrb1
        mov     ax,400H                 ; redondea a 16K
emsrb1: sub     [cs:count],ax
        add     [cs:srcseg],ax
	shl	ax,1			; calcula n£mero de palabras
	shl	ax,1
	shl	ax,1
        mov     cx,ax
        mov     bx,[cs:pagenum]         ; siguiente p gina
        mov     dx,[cs:handle]
        mov     ax,4400H
        push    cx
        int     67H
        pop     cx
        xor     di,di                   ; copia datos desde la p gina EMS
        xor     si,si
        repnz   movsw
        inc     [cs:pagenum]
        jmp     emsreadblock            ; siguiente bloque
emsrdone:
        push    cs                      ; recupera DS y libera memoria EMS
        pop     ds
        mov     ah,48H                  ; primero recupera el contexto de mapeado
        mov     dx,[handle]
        int     67H
        mov     ah,45H
        int     67H
        jmp     restmem5

restfile:
        mov     ax,3D00H                ; abre fichero
        mov     dx,offset filespec
        int     21H
        jnc     restfile2
	mov     dx,offset errmsg2       ; ­no puede abrir fichero!
        mov     ah,9                    ; avisa
        int     21H
        mov     ax,4C02H
        int     21H                     ; y sale
restfile2:
        mov     [handle],ax
readblock:
        mov     ax,[cs:srcseg]          ; DS=siguiente bloque de datos
        mov     ds,ax
        mov     ax,[cs:count]           ; calcula p rrafos a leer
        or      ax,ax
        jz      restmem4
        cmp     ax,800H                 ; si quedan m s de 32K
        jc      rb1
        mov     ax,800H                 ; redondea a 32K
rb1:    sub     [cs:count],ax
        add     [cs:srcseg],ax
	shl	ax,1			; calcula n£mero de bytes
	shl	ax,1
	shl	ax,1
	shl	ax,1
        mov     cx,ax
        xor     dx,dx
        mov     ah,3FH
        mov     bx,[cs:handle]
        int     21H
        jnc     readblock
	mov     dx,offset errmsg2       ; ­error de lectura de fichero!
        mov     ah,9                    ; avisa
        int     21H
        mov     ax,4C03H
        int     21H                     ; y sale
restmem4:
        push    cs                      ; recupera DS y cierra fichero
        pop     ds
        mov     bx,[handle]
        mov     ah,3EH
        int     21H
        mov     ah,41H                  ; borra fichero
        mov     dx,offset filespec
        int     21H
;
; Recupera la pila.
;
restmem5:
        cli
        mov     ax,[stkseg]
        mov     ss,ax
        mov     sp,[stkptr]
        sti
;
; Vuelve al punto de llamada.
;
        mov     ax,[retval]
error:  pop     di
        pop     si
        pop     ds
        ret
ENDP

PUBLIC  sspawn

last_addr:

END
