;
; PUBLIC DOMAIN
;

.model large

extrn _vbase:word
extrn _maxx:word
extrn _maxy:word
extrn _current_color:byte
extrn _videomethod:word

public DCLS, DPUTC, DPUTS, DCLRWND, DSCROLLUP, DSCROLLDN

.CODE

DCLS	proc far

	push bp
	mov bp,sp

	push di
	mov es,_vbase		; set the es segment to the bottom of video

	mov bx,_maxx
	mov ax,_maxy
	mul bl				; determine size of screen in words

	mov cx,ax			; set up the count for the screen size
	xor di,di			; point es:di to start of screen

	mov ah,_current_color ; set the attribute
	mov al,20h			; write a space

	cld 				; make sure we're going in the right direction...
	repnz stosw			; slam that sucker out to memory..

	pop di
	pop bp

	ret

DCLS	endp

DPUTC	proc far
	push bp
	mov bp,sp

	pushf
	cld

	cmp _videomethod,1
	je bsputc

	mov bx,[bp+6]		; get the character to write
	mov dx,[bp+8]		; get the y location
	mov es,_vbase		; set up the video segment

	mov ax,_maxx
	dec dl				; off by one because of 1 base coords
	mul dl
	add ax,[bp+10]		; get base video offset
	dec ax				; off by one because of 1 based coords
	shl ax,1			; double it (because of attribute)

	mov bh,_current_color

	xchg ax,bx
	mov es:[bx],ax		; put it on the screen

	popf
	pop bp
	ret 6

bsputc: mov ah,02h
	mov cx,[bp+8]
	mov dx,[bp+10]
	mov dh,cl
	dec dh
	dec dl
	mov bh,0
	int 10h

	mov ax,[bp+6]
	mov ah,09h
	mov bh,0
	mov cx,1
	mov bl,_current_color
	int 10h

	popf
	pop bp
	ret 6

DPUTC	endp

DPUTS	proc far

	push bp
	mov bp,sp

	pushf
	cld

	cmp _videomethod,1
	je bsputs

	push di 			; save needed registers
	push si
	push ds

	mov ax,_maxx
	mov es,_vbase		; set up the video segment
	mov dh,_current_color

	lds si,[bp+6]		; get offset
	mov bx,[bp+10]		; get the y location
	mov di,[bp+12]		; get the x location

	dec bl				; off by one because of 1 base coords
	mul bl
	add di,ax			; get base video offset
	dec di				; off by one because of 1 based coords
	shl di,1			; double it (because of attribute)

	mov ah,dh			; put the attribute in ah
	mov dx,si


l0:	lodsb				; load character
	cmp al,0			; test for zero byte
	jz l1				; end of string
	stosw				; write character and attribute
	jmp l0

l1:	sub si,dx			; how many characters were written
	mov ax,si
	dec ax

	pop ds				; restore registers
	pop si
	pop di

	popf

	pop bp
	ret 8

bsputs: mov cx,[bp+10]
	mov dx,[bp+12]
	mov dh,cl
	dec dh
	dec dl
	mov bh,0
	
	push ds
	pop es
	push si
	push di
	lds si,[bp+6]
	mov di,si
	mov bl,es:_current_color
	mov cx,1

l5:	mov ah,2
	int 10h
	inc dl
	mov ah,9
	lodsb	
	cmp al,0
	jz l6
	int 10h
	jmp l5

l6:	mov ax,si
	sub ax,di
	dec ax
	pop di
	pop si
	push es
	pop ds

	popf

	pop bp
	ret 8
	
DPUTS endp

DCLRWND proc far
	push bp
	mov bp,sp

	push di
	push si

	mov es,_vbase	; video segment

	mov di,[bp+12] ; (x2)

	mov cx,[bp+6] ;  (y1)
	sub cx,[bp+10]  ; how many lines? (y1 - y2)
	inc cx

	mov ax,_maxx
	mov bx,[bp+10]
	dec bl
	mul bl
	add di,ax
	dec di
	mov si,di

	mov ah,_current_color
	mov al,20h

	mov bx,[bp+8]  ; x2
	sub bx,[bp+12]  ; how many columns (x2 - x1)
	inc bx

l2: mov dx,bx
	xchg cx,dx
	shl di,1
	rep stosw
	add si,_maxx
	mov di,si
	xchg cx,dx
	loop l2

	pop si
	pop di

	pop bp
	ret 8

DCLRWND endp

DSCROLLUP proc far

	push bp
	mov bp,sp

	sub sp,2		; allocate a local

	pushf

	push di 		; save registers
	push si
	push ds

	cld 			; set direction to forward

	mov al,_current_color
	mov [bp-2],al

	mov es,_vbase	; point to the video screen
	mov ax,_maxx	; how many columns on the screen
	mov si,ax

	mov cx,[bp+6]
	sub cx,[bp+10]	; how many lines?

	mov di,[bp+12]	; x offset of first line
	mov bx,[bp+10]	; y offset of first line
	dec bl			; adjust for 1 base
	mul bl
	add di,ax
	dec di			; first line offset in di

	shl si,1
	shl di,1

	mov ax,si		; save maxx * 2
	add si,di		; offset of second line in si

	push es
	pop ds			; point both at video

	mov bx,[bp+8]
	sub bx,[bp+12]	; how many columns
	inc bx

l3: mov dx,bx
	xchg cx,dx
	push si 		; store this offset
	rep movsw
	pop di
	mov si,ax
	add si,di
	xchg cx,dx
	loop l3

	xchg cx,bx
	mov ah,[bp-2]
	mov al,20h
	rep stosw

	pop ds			; restore registers
	pop si
	pop di

	popf

	mov sp,bp		; drop locals

	pop bp
	ret 8

DSCROLLUP endp

DSCROLLDN proc far

	push bp
	mov bp,sp

	sub sp,2		; allocate a local

	pushf

	push di 		; save registers
	push si
	push ds

	cld 			; set direction to forward

	mov al,_current_color
	mov [bp-2],al

	mov es,_vbase	; point to the video screen
	mov ax,_maxx	; how many columns on the screen
	mov si,ax

	mov cx,[bp+6]
	sub cx,[bp+10]	; how many lines?

	mov di,[bp+12]	; x offset of first line
	mov bx,[bp+6]	; y offset of last line
	dec bl			; adjust for 1 base
	mul bl
	add di,ax
	dec di			; last line offset in si

	shl si,1
	shl di,1

	mov ax,si		; save maxx * 2
	mov si,di
	sub si,ax		; offset of second to last line in di

	push es
	pop ds			; point both at video

	mov bx,[bp+8]
	sub bx,[bp+12]	; how many columns
	inc bx

l4: mov dx,bx
	xchg cx,dx
	push si		; store this offset
	rep movsw
	pop di
	mov si,di
	sub si,ax
	xchg cx,dx
	loop l4

	xchg cx,bx
	mov ah,[bp-2]
	mov al,20h
	rep stosw

	pop ds			; restore registers
	pop si
	pop di

	popf

	mov sp,bp		; drop locals

	pop bp
	ret 8

DSCROLLDN endp

end
