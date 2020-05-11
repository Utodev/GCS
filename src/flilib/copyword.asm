	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _i86_wcopy
	;i86_wcopy(source, dest, count)
	;Copy count 16-bit words from source to dest. Count must not be zero.
_i86_wcopy PROC far
	push bp
	mov bp,sp
	push es
	push di
	push ds
	push si
	push cx

	mov ax,[bp+6+2]		;s seg
	mov ds,ax
	mov si,[bp+4+2]		;s offset
	mov ax,[bp+10+2]	;d seg
	mov es,ax
	mov di,[bp+8+2] 	;d offset
	mov cx,[bp+12+2]	;count

	cld
	rep movsw

	pop cx
	pop si
	pop ds
	pop di
	pop es
	pop bp
	ret
_i86_wcopy ENDP

END
