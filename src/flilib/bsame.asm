	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _i86_bsame
	;i86_bsame(d, count)
	;Find out how many bytes in a row are the same value.
_i86_bsame PROC far
	push bp
	mov bp,sp
	push di
	cld

	les di,[bp+4+2]
	mov cx,[bp+8+2]
	mov ax,es:[di]
	inc cx
	repe scasb

	mov ax,[bp+8+2]
	sub ax,cx

	pop di
	pop bp
	ret
_i86_bsame ENDP

END
