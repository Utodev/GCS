	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _i86_bzero
	;i86_bzero(soffset, ssegment, bytes)
_i86_bzero PROC far
	push bp
	mov bp,sp
	push es
	push di
	push cx

	les di,[bp+6]
	mov cx,[bp+10]
	xor ax,ax
	rep stosb

	pop cx
	pop di
	pop es
	pop bp
	ret
_i86_bzero ENDP

END
