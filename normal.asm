;
;
; assembler implementation of normalize for speed
;
; PUBLIC DOMAIN
;
;

EXTRN _rot13:word

public NORMALIZE

.model large
.code

ROT00   DB      000h, 001h, 002h, 003h, 004h, 005h, 006h, 007h
	DB      008h, 009h, 00ah, 00bh, 00ch, 00dh, 00eh, 00fh
	DB      010h, 011h, 012h, 013h, 014h, 015h, 016h, 017h
	DB      018h, 019h, 01ah, 01bh, 01ch, 01dh, 01eh, 01fh
	DB      020h, 021h, 022h, 023h, 024h, 025h, 026h, 027h
	DB      028h, 029h, 02ah, 02bh, 02ch, 02dh, 02eh, 02fh
	DB      030h, 031h, 032h, 033h, 034h, 035h, 036h, 037h
	DB      038h, 039h, 03ah, 03bh, 03ch, 03dh, 03eh, 03fh
	DB      040h, 041h, 042h, 043h, 044h, 045h, 046h, 047h
	DB      048h, 049h, 04ah, 04bh, 04ch, 04dh, 04eh, 04fh
	DB      050h, 051h, 052h, 053h, 054h, 055h, 056h, 057h
	DB      058h, 059h, 05ah, 05bh, 05ch, 05dh, 05eh, 05fh
	DB      060h, 061h, 062h, 063h, 064h, 065h, 066h, 067h
	DB      068h, 069h, 06ah, 06bh, 06ch, 06dh, 06eh, 06fh
	DB      070h, 071h, 072h, 073h, 074h, 075h, 076h, 077h
	DB      078h, 079h, 07ah, 07bh, 07ch, 07dh, 07eh, 07fh
	DB      080h, 081h, 082h, 083h, 084h, 085h, 086h, 087h
	DB      088h, 089h, 08ah, 08bh, 08ch, 08dh, 08eh, 08fh
	DB      090h, 091h, 092h, 093h, 094h, 095h, 096h, 097h
	DB      098h, 099h, 09ah, 09bh, 09ch, 09dh, 09eh, 09fh
	DB      0a0h, 0a1h, 0a2h, 0a3h, 0a4h, 0a5h, 0a6h, 0a7h
	DB      0a8h, 0a9h, 0aah, 0abh, 0ach, 0adh, 0aeh, 0afh
	DB      0b0h, 0b1h, 0b2h, 0b3h, 0b4h, 0b5h, 0b6h, 0b7h
	DB      0b8h, 0b9h, 0bah, 0bbh, 0bch, 0bdh, 0beh, 0bfh
	DB      0c0h, 0c1h, 0c2h, 0c3h, 0c4h, 0c5h, 0c6h, 0c7h
	DB      0c8h, 0c9h, 0cah, 0cbh, 0cch, 0cdh, 0ceh, 0cfh
	DB      0d0h, 0d1h, 0d2h, 0d3h, 0d4h, 0d5h, 0d6h, 0d7h
	DB      0d8h, 0d9h, 0dah, 0dbh, 0dch, 0ddh, 0deh, 0dfh
	DB      0e0h, 0e1h, 0e2h, 0e3h, 0e4h, 0e5h, 0e6h, 0e7h
	DB      0e8h, 0e9h, 0eah, 0ebh, 0ech, 0edh, 0eeh, 0efh
	DB      0f0h, 0f1h, 0f2h, 0f3h, 0f4h, 0f5h, 0f6h, 0f7h
	DB      0f8h, 0f9h, 0fah, 0fbh, 0fch, 0fdh, 0feh, 0ffh

ROT13   DB      000h, 001h, 002h, 003h, 004h, 005h, 006h, 007h
	DB      008h, 009h, 00ah, 00bh, 00ch, 00dh, 00eh, 00fh
	DB      010h, 011h, 012h, 013h, 014h, 015h, 016h, 017h
	DB      018h, 019h, 01ah, 01bh, 01ch, 01dh, 01eh, 01fh
	DB      020h, 021h, 022h, 023h, 024h, 025h, 026h, 027h
	DB      028h, 029h, 02ah, 02bh, 02ch, 02dh, 02eh, 02fh
	DB      030h, 031h, 032h, 033h, 034h, 035h, 036h, 037h
	DB      038h, 039h, 03ah, 03bh, 03ch, 03dh, 03eh, 03fh
	DB      040h, 04eh, 04fh, 050h, 051h, 052h, 053h, 054h
	DB      055h, 056h, 057h, 058h, 059h, 05ah, 041h, 042h
	DB      043h, 044h, 045h, 046h, 047h, 048h, 049h, 04ah
	DB      04bh, 04ch, 04dh, 05bh, 05ch, 05dh, 05eh, 05fh
	DB      060h, 06eh, 06fh, 070h, 071h, 072h, 073h, 074h
	DB      075h, 076h, 077h, 078h, 079h, 07ah, 061h, 062h
	DB      063h, 064h, 065h, 066h, 067h, 068h, 069h, 06ah
	DB      06bh, 06ch, 06dh, 07bh, 07ch, 07dh, 07eh, 07fh
	DB      080h, 081h, 082h, 083h, 084h, 085h, 086h, 087h
	DB      088h, 089h, 08ah, 08bh, 08ch, 08dh, 08eh, 08fh
	DB      090h, 091h, 092h, 093h, 094h, 095h, 096h, 097h
	DB      098h, 099h, 09ah, 09bh, 09ch, 09dh, 09eh, 09fh
	DB      0a0h, 0a1h, 0a2h, 0a3h, 0a4h, 0a5h, 0a6h, 0a7h
	DB      0a8h, 0a9h, 0aah, 0abh, 0ach, 0adh, 0aeh, 0afh
	DB      0b0h, 0b1h, 0b2h, 0b3h, 0b4h, 0b5h, 0b6h, 0b7h
	DB      0b8h, 0b9h, 0bah, 0bbh, 0bch, 0bdh, 0beh, 0bfh
	DB      0c0h, 0c1h, 0c2h, 0c3h, 0c4h, 0c5h, 0c6h, 0c7h
	DB      0c8h, 0c9h, 0cah, 0cbh, 0cch, 0cdh, 0ceh, 0cfh
	DB      0d0h, 0d1h, 0d2h, 0d3h, 0d4h, 0d5h, 0d6h, 0d7h
	DB      0d8h, 0d9h, 0dah, 0dbh, 0dch, 0ddh, 0deh, 0dfh
	DB      0e0h, 0e1h, 0e2h, 0e3h, 0e4h, 0e5h, 0e6h, 0e7h
	DB      0e8h, 0e9h, 0eah, 0ebh, 0ech, 0edh, 0eeh, 0efh
	DB      0f0h, 0f1h, 0f2h, 0f3h, 0f4h, 0f5h, 0f6h, 0f7h
	DB      0f8h, 0f9h, 0fah, 0fbh, 0fch, 0fdh, 0feh, 0ffh

ROT10   DB      000h, 001h, 002h, 003h, 004h, 005h, 006h, 007h
	DB      008h, 009h, 00ah, 00bh, 00ch, 00dh, 00eh, 00fh
	DB      010h, 011h, 012h, 013h, 014h, 015h, 016h, 017h
	DB      018h, 019h, 01ah, 01bh, 01ch, 01dh, 01eh, 01fh
	DB      020h, 050h, 051h, 052h, 053h, 054h, 055h, 056h
	DB      057h, 058h, 059h, 05ah, 05bh, 05ch, 05dh, 05eh
	DB      05fh, 060h, 061h, 062h, 063h, 064h, 065h, 066h
	DB      067h, 068h, 069h, 06ah, 06bh, 06ch, 06dh, 06eh
	DB      06fh, 070h, 071h, 072h, 073h, 074h, 075h, 076h
	DB      077h, 078h, 079h, 07ah, 07bh, 07ch, 07dh, 07eh
	DB      021h, 022h, 023h, 024h, 025h, 026h, 027h, 028h
	DB      029h, 02ah, 02bh, 02ch, 02dh, 02eh, 02fh, 030h
	DB      031h, 032h, 033h, 034h, 035h, 036h, 037h, 038h
	DB      039h, 03ah, 03bh, 03ch, 03dh, 03eh, 03fh, 040h
	DB      041h, 042h, 043h, 044h, 045h, 046h, 047h, 048h
	DB      049h, 04ah, 04bh, 04ch, 04dh, 04eh, 04fh, 050h
	DB      051h, 052h, 053h, 054h, 055h, 056h, 057h, 058h
	DB      059h, 05ah, 05bh, 05ch, 05dh, 05eh, 05fh, 060h
	DB      061h, 062h, 063h, 064h, 065h, 066h, 067h, 068h
	DB      069h, 06ah, 06bh, 06ch, 06dh, 06eh, 06fh, 070h
	DB      071h, 072h, 073h, 074h, 075h, 076h, 077h, 078h
	DB      079h, 07ah, 07bh, 07ch, 07dh, 07eh, 07fh, 080h
	DB      081h, 082h, 083h, 084h, 085h, 086h, 087h, 088h
	DB      089h, 08ah, 08bh, 08ch, 08dh, 08eh, 08fh, 090h
	DB      091h, 092h, 093h, 094h, 095h, 096h, 097h, 098h
	DB      099h, 09ah, 09bh, 09ch, 09dh, 09eh, 09fh, 0a0h
	DB      0a1h, 0a2h, 0a3h, 0a4h, 0a5h, 0a6h, 0a7h, 0a8h
	DB      0a9h, 0aah, 0abh, 0ach, 0adh, 0aeh, 0afh, 0b0h
	DB      0b1h, 0b2h, 0b3h, 0b4h, 0b5h, 0b6h, 0b7h, 0b8h
	DB      0b9h, 0bah, 0bbh, 0bch, 0bdh, 0beh, 0bfh, 0c0h
	DB      0c1h, 0c2h, 0c3h, 0c4h, 0c5h, 0c6h, 0c7h, 0c8h
	DB      0c9h, 0cah, 0cbh, 0cch, 0cdh, 0ceh, 0cfh, 0d0h


NORMALIZE proc far

	push bp
	mov bp,sp

	push di
	push si
	push ds

	cmp _rot13,1
	je drot13
	cmp _rot13,2
	je drot10
	mov bx,offset ROT00
	jmp start

drot13: mov bx,offset ROT13
	jmp start

drot10: mov bx,offset ROT10

start:  les di,[bp+6]
	push es
	pop ds
	mov si,di

l1:     lodsb
	cmp al,0
	jz l3
	cmp al,8dh
	je l1
	cmp al,0ah
	je l1
	cmp al,0dh
	je l2
	xlat cs:ROT00
	stosb
	jmp l1

l2:     mov al,0ah
	xlat cs:ROT00
	stosb
	jmp l1

l3:     stosb

	pop ds
	pop si
	pop di

	pop bp
	ret 4

NORMALIZE endp

end
