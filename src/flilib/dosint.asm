	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _i86_dosint
	;i86_dosint(inregs, outregs)
	;Does a DOS interrupt (21h) from C.
	;Returns flags in ax.
	;inregs and outregs are the following structure:
	;struct i86_byte_regs {
	;	unsigned char al, ah, bl, bh, cl, ch, dl, dh;
	;	unsigned int si, di, ds, es;
	;};
	;Inregs and outregs may usually point to the same structure
	;This generates a warning during assembly but works ok.
_i86_dosint PROC far
	push bp
	mov bp,sp
	push bx
	push si
	push di
	push es
	push ds

	;point ds:di to input parameters
	lds di,[bp+4+2]
	mov ax,[di]
	mov bx,[di+2]
	mov cx,[di+4]
	mov dx,[di+6]
	mov si,[di+8]
	push ax
	mov ax,[di+14]
	mov es,ax
	mov ax,ss
	mov cs:oss,ax
	mov cs:osp,sp
	pop ax
	lds di,[di+10]

	int 21h
	cli

	mov cs:oax,ax
	mov ax,cs:oss
	mov ss,ax
	mov sp,cs:osp
	sti
	pop ax
	mov ax,cs:oax
        ;save ds:di and point 'em to output parameters
	push ds
	push di
	lds di,[bp+8+2]
	mov [di],ax
	mov [di+2],bx
	mov [di+4],cx
	mov [di+6],dx
	mov [di+8],si
        pop ax	;'di' into ax
	mov [di+10],ax
        pop ax  ;'ds' into ax
	mov [di+12],ax
	mov ax,es
	mov [di+14],ax

	;move flags to ax (the return value...)
	pushf
	pop ax

	pop ds
	pop es
	pop di
	pop si
	pop bx
	pop bp
	ret

oax equ this word
	dw 0
oss equ this word
	dw 0
osp equ this word
	dw 0
_i86_dosint ENDP

END
