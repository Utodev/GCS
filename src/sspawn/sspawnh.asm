COMMENT ~
/*-------------------------------------------------------------------------+
| Module: SSPAWN.ASM                                                       |
| Project: TOOLS                                                           |
| Author: Paul A. Penrose                                                  |
|         (c) 1993, 4D Interactive Systems, Inc.  All Rights Reserved.     |
| Purpose:                                                                 |
|   This module contains the code that will swap the current program to EMS|
|   or a disk file and then load COMMAND.COM to execute the command passed |
|   by the caller.  To assure that the maximum amount of memory is         |
|   available to execute the command, place this module in the link just   |
|   after the startup module (first in the project file).  Special thanks  |
|   to Ray Duncan for his book "MSDOS Programming", and Andrew Schulman    |
|   for his book "Undocumented DOS".  Both books were indispensable aids   |
|   when writing this code.                                                |
|                                                                          |
|--- History --------------------------------------------------------------|
| When        Who           What                                           |
| 25 Jan 93   P. Penrose    Initial creation.                              |
| 11 Dec 93   P. Penrose    Prepare for Freeware release.                  |
+-------------------------------------------------------------------------*/ ~

IDEAL

MODEL HUGE,C

P286N

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
command         DB 128 dup (?)
fcb1            DB 0
                DB 11 dup (' ')
                DB 25 dup (0)
fcb2            DB 0
                DB 11 dup (' ')
                DB 25 dup (0)
cmdcom          DB 'COMSPEC=',0
ems_device_name DB 'EMMXXXX0', 0
filespec        DB 128 dup (0)
errmsg1         DB 'ERROR: Unable to recover original memory.',0DH,0AH,'$'
errmsg2         DB 'ERROR: Unable to restore program from disk.',0DH,0AH,'$'
copyright       DB 'SSPAWN is (c) 1993, 4D Interactive Systems, Inc.',0
exec_parms      exec_pb ?


PROC    sspawn
        ARG     cmd:DWORD
        ARG     path:DWORD

        push    ds              ;Save caller's state
        push    si
        push    di
        cld
;
; First we need to copy the command passed by the caller into the command
; tail that will be used when we call EXEC.  If the address is NULL or the
; string is empty (first char is 0), then the command tail length is set to
; zero and just a command shell is spawned.
;
        push    cs
        pop     es
        mov     di,offset command       ;Set up command tail to COMMAND.COM
        xor     al,al                   ;Initially put nothing in it
        stosb
        lds     si,[cmd]                ;Get pointer to command
        mov     ax,ds                   ;Get Segment
        or      ax,si                   ;Is it NULL?
        jz      get_cmdcom
        mov     al,[si]                 ;Is it an empty string?
        or      al,al
        jz      get_cmdcom
        mov     al,'/'                  ;Put "/C " into command tail
        stosb
        mov     al,'C'
        stosb
        mov     al,' '
        stosb
        mov     cx,124                  ;Maximum length of command tail
copy_cmd:
        lodsb                           ;append cmd to command tail
        or      al,al
        jz      cmd_copied
        stosb
        loop    copy_cmd
cmd_copied:
        mov     al,13                   ;append C/R
        stosb
        mov     ax,127                  ;Calc length
        sub     ax,cx
        mov     [cs:command],al

;
; Now we need to search through the environment strings and find out where
; COMMAND.COM is located.  We need this to pass to EXEC to spawn a command
; shell.
;
get_cmdcom:
        push    cs
        pop     ds
        mov     ax,SEG _psp
        mov     es,ax
        mov     ax,[es:_psp]            ;Get our PSP segment
        mov     es,ax
        dec     ax
        mov     [ourmcb],ax
        mov     ax,[es:002CH]           ;Get segment of environment
        mov     [exec_parms.environment],ax
        mov     es,ax
        xor     di,di                   ;Start search at beginning of env
genv1:
        mov     si,offset cmdcom        ;Init pointer to name to find
        cmp     [byte ptr es:di],0      ;End of environment?
        jne     genv2
        mov     ax,8001H
        jmp     error
genv2:
        lodsb                           ;get char from name
        or      al,al                   ;end of name?
        jz      copy_cmdcom
        cmp     al,[es:di]              ;next char the same?
        jne     genv3                   ;not the one, goto next
        inc     di
        jmp     genv2                   ;keep comparing
genv3:
        xor     al,al                   ;scan forward to start of next string
        mov     cx,0FFFFH               ;  in the environment table
        repnz   scasb
        jmp     genv1
copy_cmdcom:
        push    es                      ;swap seg regs
        pop     ds
        push    cs
        pop     es
        mov     si,di
        mov     di,offset prgname
nextchar:
        lodsb                           ;Get char from environment string
        or      al,al
        stosb
        jnz     nextchar                ;Continue if not done
;
; Next we need to save everything after this module through the end of our
; allocated memory space to EMS if there is enough, or a temporary disk file.
;
        push    cs
        pop     ds
        mov     ax,[ourmcb]             ;Point to our MCB
        mov     es,ax
        xor     bx,bx
        mov     cx,[es:(MCB ptr bx).size] ;Get our mem size in paragraphs
        mov     [orgsize],cx            ;save it for restoring later
        mov     ax,offset last_addr     ;calc the number of paragraphs to save
        shr     ax,4
        mov     bx,ax
        mov     ax,cs
        add     ax,bx
        mov     [srcseg],ax             ;save starting paragraph
        sub     ax,[ourmcb]
        mov     [keep],ax               ;save amount to keep when we spawn
        dec     ax                      ;account for MCB
        sub     cx,ax                   ;cx = number of paragraphs
        mov     [count],cx              ;save count
;
; First check to see if there is an EMS driver loaded
;
        mov     ax,3D00H                ;Open file, read only
        mov     dx,offset ems_device_name
        int     21H
        jnc     emsck1
        jmp     use_file
emsck1: mov     bx,ax
        push    bx
        mov     ax,4400H                ;Get device info
        int     21H
        pop     bx
        jnc     emsck2
        jmp     use_file
emsck2: test    dx,0080H                ;Is it a device?
        jnz     emsck3
        jmp     ems_no_driver
emsck3: push    bx
        mov     ax,4407H                ;Is the device ready?
        int     21H
        pop     bx
        or      al,al
        jnz     emsck4
        jmp     ems_no_driver
emsck4: mov     ax,3E00H                ;Close the driver
        int     21H
;
; Check the EMS version, if not 3.0 or greater, don't use EMS
;
        mov     ah,46H
        int     67H
        cmp     al,30H
        jge     emsck5
        jmp     use_file
;
; Next get the pageframe and store it for later reference
;
emsck5: mov     ah,41H
        int     67H
        mov     [pageframe],bx
        or      ah,ah
        jz      emsck6
        jmp     use_file
;
; Now check to see if there is enough EMS to save our memory
;
emsck6: mov     ah,42H
        int     67H
        mov     ax,[count]              ;convert count from paras to EMS pages
        shr     ax,10
        inc     ax
        cmp     ax,bx
        jle     emsck7
        jmp     use_file
;
; Allocate the number of EMS pages we need and copy our memory to them
;
emsck7: mov     bx,ax                   ;AX still has the number of pages
        mov     ah,43H
        int     67H
        mov     [handle],dx
        mov     ah,47H                  ;save mapping context
        int     67H
        push    [srcseg]                ;save so we can restore it later
        push    [count]
        mov     [pagenum],0
        mov     ax,[pageframe]
        mov     es,ax
emsnextblock:
        mov     ax,[cs:srcseg]          ;Point DS at next block of data
        mov     ds,ax
        mov     ax,[cs:count]           ;figure out how many paras to copy
        or      ax,ax
        jz      emsdone
        cmp     ax,400H
        jc      emsnb1
        mov     ax,400H
emsnb1: sub     [cs:count],ax
        add     [cs:srcseg],ax
        shl     ax,3                    ;calc number of words
        mov     cx,ax
        mov     bx,[cs:pagenum]         ;page in the next page
        mov     dx,[cs:handle]
        mov     ax,4400H
        push    cx
        int     67H
        pop     cx
        xor     di,di                   ;copy the data to the ems page
        xor     si,si
        repnz   movsw
        inc     [cs:pagenum]
        jmp     emsnextblock
emsdone:
        push    cs                      ;Restore DS
        pop     ds
        pop     [count]
        pop     [srcseg]

        jmp     relmem

ems_no_driver:
        mov  ax,3E00H                   ;Close the EMS driver & use a file
        int  21H
;
; Now save the rest of the memory to a file
;
use_file:
        mov     [pageframe],0           ;flag that we did NOT use EMS
        mov     [filespec],0
        push    cs
        pop     es
        mov     di,offset filespec
        lds     si,[path]               ;check for a swap path
        mov     ax,ds
        or      ax,si                   ;is it NULL?
        jz      createfile
copypath:
        lodsb                           ;copy the path
        or      al,al
        stosb
        jnz     copypath
createfile:
        push    cs
        pop     ds
        mov     ah,5AH                  ;Create temp file
        xor     cx,cx
        mov     dx,offset filespec
        int     21H
        jnc     save2file
        mov     ax,8002H
        jmp     error
save2file:
        mov     [handle],ax             ;save the handle
        push    [srcseg]                ;save so we can restore it later
        push    [count]
nextblock:
        mov     ax,[cs:srcseg]          ;Point DS at next block of data
        mov     ds,ax
        mov     ax,[cs:count]           ;figure out how many paras to write
        or      ax,ax
        jz      closefile
        cmp     ax,800H
        jc      nb1
        mov     ax,800H
nb1:    sub     [cs:count],ax
        add     [cs:srcseg],ax
        shl     ax,4                    ;calc number of bytes
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
        push    cs                      ;Restore DS and close the file
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
; The next thing we need to do is save the current stack pointer and segment
; and then reset it to our internal storage.  Then we resize our memory block
; to release all unused memory (which was just saved).
;
relmem:
        mov     ax,ss                   ;save the stack segment
        mov     [stkseg],ax
        mov     [stkptr],sp             ;save the stack pointer
        cli                             ;reset the stack to our internal
        mov     sp,offset newstk+256
        mov     ax,cs
        mov     ss,ax
        sti
        mov     ax,[ourmcb]
        inc     ax
        mov     es,ax
        mov     ah,4AH                  ;resize our memory allocation
        mov     bx,[keep]               ;  to get rid of everything not needed
        int     21H
;
; Now spawn a command shell using the EXEC function
;
        mov     [retval],0
        mov     ax,offset command
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
        jnc     restmem
        mov     [cs:retval],8005H
;
; Now resize the memory allocation to the original size and then restore the
; memory contents.
restmem:
        push    cs
        pop     ds
        mov     ax,[ourmcb]
        inc     ax
        mov     es,ax
        mov     ah,4AH                  ;resize our memory allocation to the
        mov     bx,[orgsize]            ;  original size before the spawn
        int     21H
        jnc     restmem2
        mov     dx,offset errmsg1       ;could not get original memory back!
        mov     ah,9                    ;   tell user
        int     21H
        mov     ax,4C01H
        int     21H                     ;   and exit
;
; If we saved to EMS, then restore from there, else restore from the file
;
restmem2:
        test    [pageframe],0FFFFH      ;In EMS?
        jz      restfile                ;If not, go restore from file
        mov     [pagenum],0
        mov     ax,[pageframe]          ;Get segment of EMS pageframe
        mov     ds,ax
emsreadblock:
        mov     ax,[cs:srcseg]          ;Point ES at next block of data
        mov     es,ax
        mov     ax,[cs:count]           ;figure out how many paras to copy
        or      ax,ax
        jz      emsrdone
        cmp     ax,400H                 ;If more than 16K left, then
        jc      emsrb1
        mov     ax,400H                 ;truncate to 16K
emsrb1: sub     [cs:count],ax
        add     [cs:srcseg],ax
        shl     ax,3                    ;calc number of words
        mov     cx,ax
        mov     bx,[cs:pagenum]         ;page in the next page
        mov     dx,[cs:handle]
        mov     ax,4400H
        push    cx
        int     67H
        pop     cx
        xor     di,di                   ;copy the data from the ems page
        xor     si,si
        repnz   movsw
        inc     [cs:pagenum]
        jmp     emsreadblock            ;Go get the next block
emsrdone:
        push    cs                      ;Restore DS and release our EMS memory
        pop     ds
        mov     ah,48H                  ;  but first restore the mapping context
        mov     dx,[handle]
        int     67H
        mov     ah,45H
        int     67H
        jmp     restmem5

restfile:
        mov     ax,3D00H                ;open file
        mov     dx,offset filespec
        int     21H
        jnc     restfile2
        mov     dx,offset errmsg2       ;could not open swap file!
        mov     ah,9                    ;  tell the user
        int     21H
        mov     ax,4C02H
        int     21H                     ;  and exit
restfile2:
        mov     [handle],ax
readblock:
        mov     ax,[cs:srcseg]          ;Point DS at next block of data
        mov     ds,ax
        mov     ax,[cs:count]           ;figure out how many paras to read
        or      ax,ax
        jz      restmem4
        cmp     ax,800H                 ;If more than 32K left, then
        jc      rb1
        mov     ax,800H                 ;truncate to 32K
rb1:    sub     [cs:count],ax
        add     [cs:srcseg],ax
        shl     ax,4                    ;calc number of bytes
        mov     cx,ax
        xor     dx,dx
        mov     ah,3FH
        mov     bx,[cs:handle]
        int     21H
        jnc     readblock
        mov     dx,offset errmsg2       ;file read error!
        mov     ah,9                    ;  tell user
        int     21H
        mov     ax,4C03H
        int     21H                     ;  and exit
restmem4:
        push    cs                      ;Restore DS and close the file
        pop     ds
        mov     bx,[handle]
        mov     ah,3EH
        int     21H
        mov     ah,41H                  ;Delete the swap file
        mov     dx,offset filespec
        int     21H
;
; Finally restore the stack.
;
restmem5:
        cli
        mov     ax,[stkseg]
        mov     ss,ax
        mov     sp,[stkptr]
        sti
;
; Return to the caller
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
