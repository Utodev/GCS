	DOSSEG
	.MODEL large
	.CODE

linect 	equ word ptr[bp-2]

	PUBLIC _fii_unlccomp
	;fii_unlccomp_(cbuf, screen)
_fii_unlccomp PROC far
	;save the world and set the basepage
	push bp
	mov bp,sp
	sub sp,4
	push es
	push ds
	push si
	push di
	push bx
	push cx

	cld	;clear direction flag in case Aztec or someone mucks with it.

	lds si,[bp+4+2]
	les di,[bp+8+2]
	lodsw		;get the count of # of lines to skip
	mov dx,320
	mul dx
	add di,ax
	lodsw		;get line count
	mov linect,ax	;save it on stack
	mov dx,di	;keep pointer to start of line in dx
	xor ah,ah	;clear hi bit of ah cause lots of unsigned uses to follow
linelp:
	mov di,dx
	lodsb		;get op count for this line
	mov bl,al
	test bl,bl
	jmp endulcloop
ulcloop:
	lodsb		;load in the byte skip
	add di,ax
	lodsb		;load op/count
	test al,al
	js ulcrun
	mov cx,ax
	rep movsb
	dec bl
	jnz ulcloop
	jmp ulcout
ulcrun:
	neg al
	mov cx,ax 	;get signed count
	lodsb	  	;value to repeat in al
	rep stosb
	dec bl
endulcloop:
	jnz ulcloop
ulcout:
	add dx,320
	dec linect
	jnz linelp

	pop cx
	pop bx
	pop di
	pop si
	pop ds
	pop es
	mov sp,bp
	pop bp
	ret
_fii_unlccomp ENDP

END
