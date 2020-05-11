	DOSSEG
	.MODEL large
	.CODE

CMODE	equ 043h
CDATA	equ 040h

	PUBLIC _aa_goclock
	;Set up clock registers for aa_getclock.
	;Should be called before aa_getclock.
_aa_goclock PROC far
	mov al,00110100b	;put it into linear count instead of divide by 2
	out CMODE,al
	xor al,al
	out CDATA,al
	out CDATA,al
	ret
_aa_goclock ENDP

	PUBLIC _aa_getclock
	;This routine returns a clock with occassional spikes where time
	;will look like its running backwards 1/18th of a second. The resolution
	;of the clock is 1/(18*256)=1/4608 second. 66 ticks of this clock
	;are supposed to be equal to a monitor 1/70 second tick.
_aa_getclock PROC far
	push cx

	mov ah,0	;get tick count from dos and use for hi 3 bytes
	int 01ah	;lo order count in dx, hi order in cx
	mov ah,dl
	mov dl,dh
	mov dh,cl

	mov al,0	;read lo byte straight from timer chip
	out CMODE,al	;latch count
	mov al,1
	out CMODE,al	;set up to read count
	in al,CDATA	;read in lo byte (and discard)
	in al,CDATA	;hi byte into al
	neg al		;make it so counting up instead of down

	pop cx
	ret
_aa_getclock ENDP

END
