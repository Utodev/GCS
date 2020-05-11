	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _i86_bcontrast
	;i86_bcontrast(s1, s2, count)
	;Return how many bytes of s1 and s2 are different.
_i86_bcontrast PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	mov cx,[bp+12+2]
	repne cmpsb
	inc cx
	mov ax,[bp+12+2]
	sub ax,cx

	pop di
	pop si
	pop ds
	pop bp
	ret
_i86_bcontrast ENDP

END
