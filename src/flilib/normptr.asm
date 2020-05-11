	DOSSEG
	.MODEL large
	.CODE

	PUBLIC	_i86_norm_ptr
	;i86_norm_ptr(offset, seg)
	;Add as much as possible of the offset of a pointer to the segment.
_i86_norm_ptr PROC far
	push bp
	mov bp,sp
	push cx

	mov ax,[bp+4+2]	;offset
	mov dx,[bp+6+2] ;segment
	mov cl,4
	shr ax,cl
	add dx,ax
	mov ax,[bp+4+2]	;offset
	and ax,15

	pop cx
	pop bp
	ret
_i86_norm_ptr ENDP

	PUBLIC	_i86_enorm_ptr
	;i86_norm_ptr(offset, seg)
	;Add as much as possible of the offset of a pointer to the segment
	;and make it evenly alligned.
_i86_enorm_ptr PROC far
	push bp
	mov bp,sp
	push cx

	mov ax,[bp+4+2]	;offset
	mov dx,[bp+6+2] ;segment
	inc ax
	and ax,0FFFEh	;force even allignment
	mov [bp+4+2],ax ;and save...
	mov cl,4
	shr ax,cl
	add dx,ax
	mov ax,[bp+4+2]	;offset
	and ax,15

	pop cx
	pop bp
	ret
_i86_enorm_ptr ENDP

END
