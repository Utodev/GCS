	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _i86_bcompare
	;i86_bcompare(s1, s2, count)
	;Return # of bytes of s1 and s2 that match.
_i86_bcompare PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	mov cx,[bp+12+2]
	inc cx
	repe cmpsb

	mov ax,[bp+12+2]
	sub ax,cx

	pop di
	pop si
	pop ds
	pop bp
	ret
_i86_bcompare ENDP

END
