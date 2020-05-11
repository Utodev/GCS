	DOSSEG
	.MODEL large
	.CODE

	PUBLIC _aa_wait_vblank
_aa_wait_vblank PROC far
	mov dx,3dah	;video status port
wvb:
	in al,dx
	test al,8
	jz wvb
	ret
_aa_wait_vblank ENDP

END
