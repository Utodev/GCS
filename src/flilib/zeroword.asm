	DOSSEG
	.MODEL large
	.CODE

	PUBLIC	_i86_wzero
	;i86_wzero(soffset, ssegment, words)
_i86_wzero PROC far
	push bp
	mov bp,sp
	push es
	push di
	push cx

	les di,[bp+6]
	mov cx,[bp+10]
	xor ax,ax
	rep stosw

	pop cx
	pop di
	pop es
	pop bp
	ret
_i86_wzero ENDP

END
