; =============================================
;	GUI_Start
;	Input:
;		N/A
;	Output:
;		N/A
; =============================================
GUI_Start proc
	mov ah, 06h 	; draw blue bar
	mov al, 00h
	mov bh, 17h 	; blue bg, white text
	xor cx, cx
	mov dx, 004fh
	int 10h

	call GUI_HideCursor
	call GUI_UpdateClock
	ret
GUI_Start endp

; =============================================
;	GUI_SetTitle
;	Input:
;		si - Gets offset of string (use lea)
;	Output:
;		N/A
; =============================================
GUI_SetTitle proc
	mov ah, 06h 	; clear the title bar to remove remnants
	mov al, 0
	mov bh, 17h
	xor cx, cx
	mov dx, 003ah
	int 10h

	xor dx, dx
	call GUI_WriteAtPos
	ret
GUI_SetTitle endp

; =============================================
;	GUI_SetHelpText
;	Input:
;		si - Gets offset of string (use lea)
;	Output:
;		N/A
; =============================================
GUI_SetHelpText proc
	mov ah, 06h
	mov al, 0
	mov bh, 70h		; white background black text
	mov cx, 1800h
	mov dx, 184fh
	int 10h 		; draw bottom help bar

	mov dx, cx
	xchg dh, dl
	call GUI_WriteAtPos
	ret
GUI_SetHelpText endp

; =============================================
;	GUI_WriteAtPos
;	Input:
;		dh - x
;		dl - y
;		si - Offset of string
; =============================================
GUI_WriteAtPos proc
	call Misc_PushAllRegs

	push dx 		; put the dx input into the stack first

	mov ah, 03h 	; get cursor pos (dx = y,x)
	mov bh, 0
	int 10h

	pop cx 			; get back the original dx input into cx first
	push dx			; put the original cursor pos into stack
	mov dx, cx 		; put the original dx input back to dx so operation goes on normally
	
	xchg dh, dl 	; int 10h/ah=2 takes in cursor pos as dx = (y,x), so swap it

	mov ah, 2
	int 10h		; set pos

	mov ah, 0eh
	@@printloop:	; printing manually because ah=9/int21h is not dealing with newlines the way I like
	lodsb 		; get one char from si to al, then increment si
	cmp al, '$'
	je @@endloop
	cmp al, 00h
	je @@endloop
	cmp al, 0ah
	je @@movecursordown
	int 10h
	jmp @@printloop
	@@movecursordown:
	mov ah, 02h 	; move cursor down one line if 0ah character encountered
	inc dh
	int 10h

	mov ah, 0eh 	; set ah back to 0eh so the printer process goes on normally
	jmp @@printloop
	@@endloop:

	mov ah, 2 		; set back original cursor pos
	pop dx 			; get original cursor pos from stack
	int 10h

	call Misc_PopAllRegs
	ret
GUI_WriteAtPos endp

; =============================================
;	GUI_UpdateClock
;	Input:
;		N/A
;	Output:
;		N/A
; =============================================
GUI_UpdateClock proc
	call Misc_PushAllRegs

	mov ah, 03h 	; get original cursor pos (dx = y,x)
	mov bh, 0
	int 10h
	push dx 		; store that in the stack

	mov dx, 003Bh	; set cursor pos to 3B,00
	mov ah, 2
	int 10h

	mov ah, 2ah ; get system date, format: DL/DH/CX
	int 21h

	mov al, dl
	call GUI_PrintTwoDigits
	
	mov ah, 0eh
	mov al, '/'
	int 10h

	mov al, dh
	call GUI_PrintTwoDigits

	mov al, '/'
	int 10h			; print '/'

	mov ax, cx 		; assuming year = 2021 (0x07E5):
	mov cl, 100d	; year divided by 100
	div cl 			; after div: al = 20 (result), ah = 21 (remainder)

	; al is already what we needed
	call GUI_PrintTwoDigits
	mov al, ah
	call GUI_PrintTwoDigits

	mov ah, 0eh
	mov al, ' '
	int 10h

	mov ah, 2ch		; get system time, format: CH:CL:DH[.DL] (DL ignored)
	int 21h

	mov ah, 0
	mov al, ch 		; set ax = ch
	mov dl, 12d		; divide by 12 (the smart way of minus 12 :) )
	div dl 			; al = (0 = AM, 1 = PM), ah = 12-hour clock hour (remainder)

	mov ch, al 		; put AM/PM into ch

	cmp ah, 0		; check if hour is 00
	jne @@hournotzero
	mov ah, 12		; set hour to 12 if it's 00
	@@hournotzero:

	mov al, ah
	call GUI_PrintTwoDigits

	mov ah, 0eh
	mov al, ':'
	int 10h

	mov al, cl
	call GUI_PrintTwoDigits

	mov al, ':'
	int 10h			; print ':'

	mov al, dh
	call GUI_PrintTwoDigits

	cmp ch, 0		; check is AM or PM
	mov al, 'A'
	je @@AMjump
	mov al, 'P'
	@@AMjump:
	int 10h			; prints either 'A' or 'P'

	mov al, 'M'		; prints 'M'
	int 10h

	mov ah, 2 		; set back original cursor pos
	pop dx 			; get original cursor pos from stack
	int 10h

	call Misc_PopAllRegs
	ret
GUI_UpdateClock endp

; =============================================
;	GUI_PrintNumber
;	Input:
;		al - number
;	Output:
;		N/A
; =============================================
GUI_PrintNumber proc
	call Misc_PushAllRegs

	aam				; split AL into (AH)(AL)

	cmp ah, 00h 	; if tens is 0
	je @@lastprint

	mov dx, ax 		; put ah,al to dh,dl (to make it consistent despite printing hundreds or tens)

	cmp ah, 10d 	; if tens is >=10, means it is in hundreds
	jl @@printtens

	mov al, ah 		; now a three digit number is (AH)(AL)(DL)
	aam
	mov dh, al 		; (AH)(DH)(DL)

	add ah, 30h
	mov al, ah
	mov ah, 0eh
	int 10h

	@@printtens:
	mov ah, 0eh
	add dh, 30h
	mov al, dh
	int 10h

	mov al, dl

	@@lastprint:
	mov ah, 0eh
	add al, 30h
	int 10h
	call Misc_PopAllRegs
	ret
GUI_PrintNumber endp

; =============================================
;	GUI_PrintTwoDigits
;	Input:
;		al - two digit number
;	Output:
;		N/A
; =============================================
GUI_PrintTwoDigits proc
	call Misc_PushAllRegs

	aam				; split AL value two digits to (AH)(AL)
	add ax, 3030h	; convert to printable ASCII values

	mov dh, al

	mov al, ah
	mov ah, 0eh
	int 10h

	mov al, dh
	int 10h

	call Misc_PopAllRegs
	ret
GUI_PrintTwoDigits endp

; =============================================
;	GUI_PrintLargeNumber
;	Input:
;		ax - number
;	Output:
;		N/A
; =============================================
GUI_PrintLargeNumber proc
	call Misc_PushAllRegs

	cmp ax, 10000d
	jae @@printtenthousands

	mov dx, ax 		; copy ax input to dx because the latter functions uses dx

	cmp ax, 1000d
	jae @@printthousands

	cmp ax, 256d
	jae @@printhundreds

	jmp @@notmyjob

	@@printtenthousands:
	mov dx, 00h
	mov cx, 10000d
	div cx

	mov ah, 0eh
	add al, 30h 	; make it ascii
	int 10h

	@@printthousands:
	mov ax, dx		; bring the remainder to ax
	mov dx, 00h
	mov cx, 1000d
	div cx

	mov ah, 0eh
	add al, 30h
	int 10h

	@@printhundreds:
	mov ax, dx
	mov cl, 100d
	div cl

	mov dl, ah 		; keep the remainder in dl

	mov ah, 0eh
	add al, 30h
	int 10h

	mov al, dl 		; put the remainder to al for GUI_PrintNumber

	@@notmyjob:
	call GUI_PrintNumber
	
	call Misc_PopAllRegs
	ret
GUI_PrintLargeNumber endp

; =============================================
;	GUI_DrawRect
;	Input:
;		al - 乇乂丅尺卂 丅卄工匚匚 border? (1-yes, 0-no)
;		ch - begin x
;		cl - begin y
;		dh - end x
;		dl - end y
;	Output:
;		N/A
; =============================================
GUI_DrawRect proc
	call Misc_PushAllRegs

					; prepare bx for length values
	mov bh, dh 		; put end x to bh
	sub bh, ch 		; bh (horizontal length) = end x (bh) - begin x (ch)
	dec bh 			; minus one for corner characters
	mov bl, dl 		; put end y to bl
	sub bl, cl 		; bl (vertical length) = end y (bl) - begin y (cl)
	dec bl

	push bx			; put some values on the stack first for later use
	push ax
	push bx
	push cx
	push bx
	push dx
	push bx
	push ax

	mov ah, 02h		; put cursor to beginning x,y
	mov dl, ch 		; begin x
	mov dh, cl 		; begin y
	mov bh, 0
	int 10h

	mov ah, 0eh
	cmp al, 0
	mov al, 218d	; single border upper left character
	je @@notthicc1
	mov al, 201d	; double border upper left character
	@@notthicc1:
	int 10h

	pop ax			; al = thiccness, ah is useless
	mov ah, 0eh
	pop cx 			; cx = length values
	mov cl, ch 		; let cx = horizontal length / loop count
	mov ch, 00h
	cmp al, 0
	mov al, 196d	; single border horizontal line character
	je @@drawhorizontaltop
	mov al, 205d	; double border horizontal line character
	@@drawhorizontaltop:
	int 10h
	loop @@drawhorizontaltop

	mov al, 191d 	; single border upper right character
	je @@notthicc2 	; the flags remain from the previous cmp
	mov al, 187d	; double border upper right character
	@@notthicc2:
	int 10h

	mov al, 179d	; single border vertical line character
	je @@notthicc3
	mov al, 186d	; double border vertical line character
	@@notthicc3:
	; @@drawvertical deals with the rest now so no int 10h here

	pop cx 			; cx = end x,y
	mov dl, ch 		; put end x to dl, we want y (dh) to still be begin y
	pop cx 			; cx = length values
	mov ch, 00h 	; let cx = vertical length only
	call @@drawvertical

	pop dx 			; prepare cursor to be back at begin x,y (inc will be done by drawvertical)
	xchg dh, dl
	pop cx 			; cx = length values
	mov ch, 00h 	; let cx = vertical length only
	call @@drawvertical

	mov ah, 02h
	inc dh 			; upper snippet from drawvertical, to move cursor down one line
	int 10h

	mov ah, 0eh
	pop cx 			; cl = thiccness, ch is useless
	cmp cl, 0
	mov al, 192d 	; single border lower left character
	je @@notthicc4
	mov al, 200d 	; double border lower left character
	@@notthicc4:
	int 10h

	pop cx 			; cx = length values
	mov cl, ch 		; let cx = horizontal length / loop count
	mov ch, 00h
	mov al, 196d	; single border horizontal line character
	je @@drawhorizontalbottom
	mov al, 205d	; double border horizontal line character
	@@drawhorizontalbottom:
	int 10h
	loop @@drawhorizontalbottom

	mov al, 217d	; single border lower right character
	je @@notthicc5
	mov al, 188d	; double border lower right character
	@@notthicc5:
	int 10h 		; phew, finally complete :')

	call Misc_PopAllRegs
	ret

	; =======================================================
	@@drawvertical:
	mov ah, 02h
	inc dh 			; first run: dh is already begin y
	int 10h
	mov ah, 0eh 	; al is set before calling this print
	int 10h
	loop @@drawvertical
	ret
GUI_DrawRect endp

; =============================================
;	GUI_DrawDivider
;	Input:
;		al - 乇乂丅尺卂 丅卄工匚匚 border? (1-yes, 0-no)
;		ch - start x
;		cl - end x
;		dh - y
;	Output:
;		N/A
; =============================================
GUI_DrawDivider proc
	call Misc_PushAllRegs

	mov bl, al 		; put that parameter to bl (using al to remain consistent with GUI_DrawRect)

	mov ah, 02h		; set cursor pos to start x,y first
	mov bh, 0
	mov dl, ch
	int 10h

	mov ah, 0eh
	cmp bl, 0
	mov al, 195d 	; ├
	je @@notthicc1
	mov al, 204d 	; ╠
	@@notthicc1:
	int 10h

	sub cl, ch 		; make cx = number of times to print the horizontal line
	dec cl
	mov ch, 00h

	cmp bl, 0
	mov al, 196d	; ─
	je @@horizontalloop
	mov al, 205d	; ═
	@@horizontalloop:
	int 10h
	loop @@horizontalloop

	mov al, 180d	; ┤
	je @@notthicc2
	mov al, 185d	; ╣
	@@notthicc2:
	int 10h

	call Misc_PopAllRegs
	ret
GUI_DrawDivider endp

; =============================================
;	GUI_ClearScreen
;	Input:
;		N/A
;	Output:
;		N/A
; =============================================
GUI_ClearScreen proc
	mov ah, 06h
	mov al, 00h
	mov bh, 07h		; clear area with black
	mov cx, 0100h
	mov dx, 184fh
	int 10h

	ret
GUI_ClearScreen endp

; =============================================
;	GUI_MsgBox
;	Input:
;		bx - address of key press handler
;		ch - begin x
;		cl - begin y
;		dh - end x
;		dl - end y
;		si - message
;	Output:
;		N/A
; =============================================
GUI_MsgBox proc
	call Misc_PushAllRegs

	mov GUI_KeyPressHandler, bx

	push dx 	; save it first so we can pop them back later
	push cx
	push cx

	add cx, 0101h	; shift position by 1 for shadow
	add dx, 0101h
	xchg ch, cl 	; int 10h uses y,x coord system
	xchg dh, dl

	mov ah, 06h 	; set shadow box
	mov al, 0
	mov bh, 07h 	; black background, white text
	int 10h

	sub cx, 0101h 	; shift position by -1 for the blue msgbox
	sub dx, 0101h

	mov bh, 17h		; blue background, white text
	int 10h

	add cx, 0101h 	; shift position by 1 again to print the shadow dots
	add dx, 0101h

	mov bl, dl 		; put end x to bl first

	mov ah, 02h 	; move cursor pos
	mov dl, cl 		; set x = begin x (meanwhile y = end y)
	mov bh, 0
	int 10h

	mov dl, bl 		; get end x from bl back to dl
	sub dl, cl 		; dl = horizontal length
	sub dh, ch 		; dh = vertical length

	mov bh, ch 		; make bx = begin y, end x first (bl is already end x)

	mov ch, 00h 	; make cx = horizontal length
	mov cl, dl
	mov ah, 0eh
	mov al, 176d	; ░

	@@horizontalshadow:
	int 10h
	loop @@horizontalshadow

	mov cl, dh 		; make cx = vertical length
	inc cl 			; plus one, otherwise the corner is lost

	mov ah, 02h
	mov dx, bx 		; bx = end x, begin y
	mov bh, 0
	int 10h

	@@verticalshadow:
	mov ah, 0eh
	int 10h
	mov ah, 02h 	; move cursor down
	inc dh
	int 10h
	loop @@verticalshadow

	pop dx 				; dx here is begin x,y
	add dx, 0302h 		; print the string in x+3, y+2
	call GUI_WriteAtPos	; si is already the message to print

	pop cx 			; get back original cx, dx input values
	pop dx

	mov al, 1
	call GUI_DrawRect

	mov cl, dh 		; set cl = end x (meanwhile ch = begin x)
	mov dh, dl 		; set dh = end y
	sub dh, 2d 		; divider is two rows above end y
	call GUI_DrawDivider

	mov ah, 06h
	mov al, 0
	mov bh, 47h		; red background, white text
	inc dh 			; one row below divider
	mov ch, dh 		; start and end same row
	mov dl, cl 		; dl = cl = end x
	sub cl, 5d 		; start x is 4 column before end x
	sub dl, 2d 		; end x is 1 column before end x (lmao?)
	int 10h

	mov dh, cl 		; x = start x
	mov dl, ch 		; y
	lea si, Str_OK
	call GUI_WriteAtPos

	call Misc_PopAllRegs
	ret
GUI_MsgBox endp

; =============================================
;	GUI_ShowCursor
;	Input:
;		N/A
;	Output:
;		N/A
; =============================================
GUI_ShowCursor proc
	mov ah, 01h 	; make full block cursor
	mov cx, 0007h
	int 10h
	ret
GUI_ShowCursor endp

; =============================================
;	GUI_HideCursor
;	Input:
;		N/A
;	Output:
;		N/A
; =============================================
GUI_HideCursor proc
	mov ah, 01h 	; make cursor hidden
	mov cx, 2607h
	int 10h
	ret
GUI_HideCursor endp

; =============================================
;	GUI_SetCursorPos
;	Input:
;		dh - x
;		dl - y
;	Output:
;		N/A
; =============================================
GUI_SetCursorPos proc
	call Misc_PushAllRegs

	xchg dh, dl 	; swap coordinate system

	mov ah, 02h
	mov bh, 0
	int 10h

	call Misc_PopAllRegs
	ret
GUI_SetCursorPos endp

; =============================================
;	GUI_PrintEventStatus
;	Input:
; 		al - event index
;		dx - x,y
;	Output:
;		N/A
; =============================================
GUI_PrintEventStatus proc
	call Misc_PushAllRegs

	call Misc_GetEventListDisplacement

	mov ah, 06h 		; prepare the colouring
	mov al, 0
	xchg dh, dl
	mov cx, dx

	cmp EventList_0_Status[bx], 0
	je @@Closed

	push cx 			; I need a register for this next instruction, put cx to stack first

	mov cl, EventList_0_Limit[bx]
	cmp EventList_0_PtcpntsCount[bx], cl
	je @@Full

	pop cx 				; cx can have its value back

	cmp EventList_0_Status[bx], 1
	je @@Open

	cmp EventList_0_Status[bx], 2
	je @@Selected

	@@Closed:
	mov bh, 47h 	; red bg, wh text
	add dl, 5d 		; length of text - 1
	lea si, Str_Closed
	jmp @@Done

	@@Full:
	pop cx 			; we used cl for some comparison stuff just now, get it back
	mov bh, 47h 	; red bg, wh text
	add dl, 3d
	lea si, Str_Full
	jmp @@Done

	@@Open:
	mov bh, 27h 	; green bg, wh text
	add dl, 3d
	lea si, Str_Open
	jmp @@Done

	@@Selected:
	mov bh, 67h 	; brown bg, wh text
	add dl, 7d
	lea si, Str_Selected
	jmp @@Done

	@@Done:
	int 10h

	mov dx, cx
	xchg dh, dl
	call GUI_WriteAtPos

	call Misc_PopAllRegs
	ret
GUI_PrintEventStatus endp