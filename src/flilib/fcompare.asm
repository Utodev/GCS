	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _i86_wcompare
	;i86_wcompare(s1, s2, count)
_i86_wcompare PROC far
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
	repe cmpsw

	mov ax,[bp+12+2]
	sub ax,cx

	pop di
	pop si
	pop ds
	pop bp
	ret
_i86_wcompare ENDP

END
