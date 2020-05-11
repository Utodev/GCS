	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _fii_mem_fcuncomp
	;Uncompress color palette onto a buffer.
_fii_mem_fcuncomp PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	push cx
	push bx
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	lodsw
	mov bx, ax	;get the count
	test bx,bx
	jmp endu
u:
	lodsb	    	;get colors to skip
	add di,ax
	add di,ax
	add di,ax
	lodsb		;get colors to copy
	or al,al	;test for zero
	jnz u2
	mov ax,256
u2:
	mov cx,ax
	add cx,ax
	add cx,ax
	rep movsb
	dec bx
endu:
	jnz u

	pop bx
	pop cx
	pop di
	pop si
	pop ds
	pop bp
	ret
_fii_mem_fcuncomp ENDP

END
