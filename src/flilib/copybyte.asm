	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _i86_bcopy
	;i86_bcopy(source, dest, count)
	;Copy count bytes from source to dest. Actual copy is done a byte at
	;a time. No allignment restrictions on source, dest. No parity
	;restrictions on count. Zero counts will trash memory!.
_i86_bcopy PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di

	cld
	lds si,[4+bp+2]
	les di,[8+bp+2]
	mov cx,[12+bp+2]
	rep movsb

	pop di
	pop si
	pop ds
	pop bp
	ret
_i86_bcopy endp

END
