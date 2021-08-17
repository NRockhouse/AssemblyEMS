ScrEventList_Start proc
	call GUI_ClearScreen

	lea si, Str_EventList
	call GUI_SetTitle

	mov al, 1
	mov cx, 0102h		; Events rect
	mov dx, 3d17h
	call GUI_DrawRect

	mov dx, 1c02h
	lea si, Str_Events
	call GUI_WriteAtPos

	mov cx, 3f02h 		; Keys rect
	mov dx, 4e0eh
	call GUI_DrawRect

	cmp Session, 0ffh 	; check Students or Staff
	jne @@Students

	mov dx, 4003h
	lea si, Str_EventListAdminKeys
	call GUI_WriteAtPos

	jmp @@Both

	@@Students:
	mov cl, 11h
	mov dl, 15h
	call GUI_DrawRect

	mov cl, 4eh
	mov dh, 13h
	call GUI_DrawDivider

	mov dx, 4003h
	lea si, Str_EventListKeys
	call GUI_WriteAtPos

	mov dx, 4511h
	lea si, Str_User
	call GUI_WriteAtPos

	mov dx, 4112h
	lea si, Str_TP
	call GUI_WriteAtPos	

	mov ah, 02h
	mov bh, 0
	mov dx, 1243h
	int 10h
	
	mov al, Session[0]
	call GUI_PrintTwoDigits
	mov al, Session[1]
	call GUI_PrintTwoDigits
	mov al, Session[2]
	call GUI_PrintTwoDigits

	mov dx, 4513h
	lea si, Str_Total
	call GUI_WriteAtPos

	mov dx, 4114h
	lea si, Str_RM
	call GUI_WriteAtPos

	mov dx, 4414h
	call GUI_SetCursorPos

	mov ah, 0eh
	mov al, '0'
	int 10h

	@@Both:
	mov dx, 4502h
	lea si, Str_Keys
	call GUI_WriteAtPos

	mov GUI_KeyPressHandler, offset ScrEventList_KeyPressHandler

	cmp EventList_Count, 0
	jne @@GotEvents
	mov dx, 090bh
	lea si, Str_NoEvents
	call GUI_WriteAtPos

	ret

	@@GotEvents:
	mov SelectedEvent, 0

	mov dx, 0203h 		; set it to select first event list item
	call GUI_SetCursorPos

	mov al, 0
	call ScrEventList_UpdateFromIndex

	ret
ScrEventList_Start endp

; =============================================
;	ScrEventList_UpdateFromIndex
;	Input:
;		al - Starting index (starting from 0)
;	Output:
;		N/A
; =============================================
ScrEventList_UpdateFromIndex proc
	mov ah, 03h 	; get current cur pos
	mov bh, 0
	int 10h

	xchg dh, dl 	; change coord system
	push dx			; save it first
	push dx

	push ax 		; ran out of registers to keep track of index to print, keeping in stack
	push ax			; !loop warning!, beware of push and pops in loop

	mov dl, 08h 	; y of first event lower right corner

	@@itemloop: 	; dl is used as iterator in this entire loop
	call Misc_GetEventListDisplacement
	mov dh, 3ch 	; x
	xchg dh, dl 	; we have to deal with colouring first, so using BIOS coord system here

	mov cx, dx 		; calculate the upper left corner (cx) from the dx
	sub ch, 5d
	sub cl, 58d

	mov ah, 06h
	mov al, 0
	mov bh, EventList_0_Color[bx]
	int 10h

	xchg ch, cl 	; swap coordinate system to draw the border now, ez!
	xchg dh, dl
	call GUI_DrawRect

	sub dl, 4d 		; move y +4 to start writing the texts
	mov dh, 03h 	; x
	call GUI_SetCursorPos

	pop ax
	call Misc_GetEventListDisplacement
	mov ah, 0eh
	mov cx, 57d 	; counter for printline
	lea si, EventList_0_Name[bx]
	call @@printline

	inc dl
	call GUI_SetCursorPos

	mov cx, 57d
	lea si, EventList_0_Desc[bx]
	call @@printline

	mov dh, 3bh 	; put the cursor at the first line description's ≈
	call GUI_SetCursorPos	; it is so that we can remove it.. I'm lazy to add more checks on
							; @@printline

	mov al, [si-1]	; read back the discarded character and print it in place of ≈
	int 10h

	mov dh, 03h 	; put cursor on second line of description
	inc dl
	call GUI_SetCursorPos

	mov cx, 57d
	call @@printline

	inc dl
	lea si, Str_Participants
	call GUI_WriteAtPos

	mov dh, 11h
	call GUI_SetCursorPos

	mov al, EventList_0_PtcpntsCount[bx]
	call GUI_PrintNumber

	mov al, '/'
	int 10h

	mov al, EventList_0_Limit[bx]
	call GUI_PrintNumber

	mov dh, 1ch
	lea si, Str_Price
	call GUI_WriteAtPos

	mov dh, 25h
	call GUI_SetCursorPos

	mov al, EventList_0_Price[bx]
	call GUI_PrintNumber

	mov dh, 2ch
	lea si, Str_Status
	call GUI_WriteAtPos

	pop ax 			; pop the item index for further usage below

	mov dh, 34h
	call GUI_PrintEventStatus

	cmp dl, 15h 	; are we at the last item now?
	jge @@itemloopend

	add dl, 8d 		; increment y to the next item's lower right corner
	inc al 			; increment the item index to print

	cmp al, EventList_Count 	; because index uses 0-indexed, we have to use the incremented al
	jge @@itemloopend

	push ax 		; push back the relevant number of pops in this loop
	push ax
	jmp @@itemloop
	@@itemloopend:

	pop dx 			; retrieve current cursor position (my coord system)
	call GUI_SetCursorPos

	mov ah, 0eh
	mov al, 177d 	; ▒
	mov cx, 59d
	call @@printasterik

	add dl, 5d
	call GUI_SetCursorPos

	mov cx, 59d
	call @@printasterik

	mov cx, 4d
	@@printleft:
	dec dl
	call GUI_SetCursorPos
	int 10h
	loop @@printleft

	mov dh, 3ch 	; x
	mov cx, 4d
	@@printright:
	call GUI_SetCursorPos
	int 10h
	inc dl
	loop @@printright

	mov dx, 4817h 			; write the selected event indicator on the bottom right of screen
	call GUI_SetCursorPos

	mov al, SelectedEvent 	; SelectedEvent uses index 0
	inc al
	call GUI_PrintTwoDigits

	mov ah, 0eh
	mov al, ' '
	int 10h
	mov al, '/'
	int 10h
	mov al, ' '
	int 10h

	mov al, EventList_Count
	call GUI_PrintTwoDigits

	pop dx 					; set back the original cursor pos to before this proc
	call GUI_SetCursorPos

	ret

	@@printline:
	lodsb
	cmp al, 0
	je @@endprintline
	cmp al, 0ah		; resolve newlines as just spaces
	jne @@notnewlinechar
	mov al, ' '
	@@notnewlinechar:
	cmp cx, 1d
	jne @@justprint
	mov al, 247d	; ≈
	@@justprint:
	int 10h
	loop @@printline
	@@endprintline:
	ret

	@@printasterik:
	int 10h
	loop @@printasterik
	ret
ScrEventList_UpdateFromIndex endp

ScrEventList_KeyPressHandler proc
	cmp ah, 31h
	je @@NewKey

	cmp ah, 48h
	je @@UpArrow_near

	cmp ah, 50h
	je @@DownArrow_near

	cmp ah, 39h
	je @@Spacebar_near

	cmp ah, 1ch
	je @@EnterKey_near

	cmp ah, 01h
	je @@EscKey_near

	jmp @@Done

	; the jumps are too far for je to jump to, so we make a nearer pointer for it
	@@UpArrow_near:		jmp @@UpArrow
	@@DownArrow_near:	jmp @@DownArrow
	@@Spacebar_near:	jmp @@Spacebar
	@@EnterKey_near:	jmp @@EnterKey
	@@EscKey_near:		jmp @@EscKey
	@@Done_near:		jmp @@Done

	@@NewKey:
	cmp Session, 0ffh
	jne @@Done_near

	cmp EventList_Count, 10d
	jl @@CreateNewEvent

	lea bx, ScrEventList_MsgBoxKeyPressHandler
	mov cx, 0308h
	mov dx, 4c10h
	lea si, Str_LimitReached
	call GUI_MsgBox
	ret

		@@CreateNewEvent:
		mov al, EventList_Count
		call Misc_GetEventListDisplacement

		lea si, Str_NewEvent		; ref: https://faculty.kfupm.edu.sa/COE/aimane/assembly/pagegen-139.aspx.htm
		lea di, EventList_0_Name[bx]
		mov cx, 10					; strlen(Str_NewEvent) = 10
		cld 						; clear direction flag
		rep movsb

		mov EventList_0_Color[bx], 07h 	; default: black bg, white text
		mov EventList_0_Limit[bx], 100d	; default: 100 participants max
		mov EventList_0_Status[bx], 01d ; default: open

		mov al, EventList_Count
		mov SelectedEvent, al

		inc EventList_Count

		mov al, EventList_Count 	; add number behind "New Event "
		aam
		add ax, 3030h
		mov [di], ah
		mov [di+1], al

		call Misc_SaveData

		call ScrEventDetails_Start
		jmp ScrEventDetails_TitleEditMode

	@@UpArrow:
	cmp SelectedEvent, 00h
	je @@Done_near

	dec SelectedEvent

	mov ah, 03h 	; get cursor pos
	mov bh, 0
	int 10h

	xchg dh, dl 	; change to my coord system because this is my program

	cmp dl, 11h

	mov dh, 02h 	; x
	mov al, SelectedEvent

	jne @@movetofirst

	mov dl, 0ah 			; y, if cursor is at last item, move to second item
	dec al 					; index to print = selected event - 1

	jmp @@movetomid

		@@movetofirst:
		mov dl, 03h 			; y, set cursor to first item
		
		@@movetomid:
		call GUI_SetCursorPos
		call ScrEventList_UpdateFromIndex
		ret

	@@Done_near2: 	jmp @@Done

	@@DownArrow:
	mov al, EventList_Count
	dec al 						; since SelectedEvent is zero-indexed

	cmp SelectedEvent, al
	jge @@Done_near2

	inc SelectedEvent

	mov ah, 03h 	; get cursor pos
	mov bh, 0
	int 10h

	xchg dh, dl 	; change to my coord system because this is my program

	mov dh, 02h 	; x
	mov al, SelectedEvent
	dec al 			; index to print = selected event - 1

	cmp dl, 03h 	; compare using original cursor y value

	je @@movetomid2

	mov dl, 11h 	; y, set cursor to last item
	dec al 			; index to print = selected event - 2

	jmp @@movetolast

		@@movetomid2:
		mov dl, 0ah 			; y, if cursor is at first item, move to second item
		
		@@movetolast:
		call GUI_SetCursorPos
		call ScrEventList_UpdateFromIndex
		ret

	@@Spacebar:
	cmp Session, 0ffh 	; staff doesn't need this
	je @@Done_near2

	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	cmp EventList_0_Status[bx], 0
	je @@Done_near2

	mov al, EventList_0_Limit[bx]
	cmp EventList_0_PtcpntsCount[bx], al
	je @@Done_near2

	cmp EventList_0_Status[bx], 1
	je @@Open

	cmp EventList_0_Status[bx], 2
	je @@Selected

		@@Open:
		mov EventList_0_Status[bx], 2
		jmp @@comparedone

		@@Selected:
		mov EventList_0_Status[bx], 1

		@@comparedone:

	mov ah, 03h 	; get cursor pos
	mov bh, 0
	int 10h

	push dx 		; put the original cursor pos on stack

	mov ah, 06h
	mov al, 0
	mov bh, 07h 	; black bg white text
	add dh, 4d 		; setup bounds to blacken the Status indicator
	add dl, 50d
	mov cx, dx
	add dl, 7d
	int 10h

	mov dx, cx 		; take the start bounds and put it to dx, swap coord system
	xchg dh, dl
	mov al, SelectedEvent
	call GUI_PrintEventStatus

	mov dx, 4414h 				; update the total price
	call GUI_SetCursorPos

	mov ah, 0ah 	; print without moving cursor
	mov al, ' ' 	; clear the Total area
	mov bh, 0
	mov cx, 4d 		; maximum possible = 2550 (so 4 characters)
	int 10h

	call ScrEventList_GetTotalSelected
	call GUI_PrintLargeNumber

	pop dx 				; set back the original cursor pos
	xchg dh, dl
	call GUI_SetCursorPos

	ret

	@@EnterKey:
	jmp ScrEventDetails_Start

	@@EscKey:
	cmp Session, 0ffh
	je @@exitsilently

	call ScrEventList_GetTotalSelected

	cmp cl, 0
	je @@exitsilently

	lea bx, ScrEventList_PaymentDialogKeyPressHandler
	mov cx, 0807h
	mov dx, 4811h
	lea si, Str_ConfirmPayment
	call GUI_MsgBox

	call ScrEventList_GetTotalSelected

	mov dx, 4109h
	call GUI_SetCursorPos
	call GUI_PrintLargeNumber 	; ax = total price

	mov dx, 1d09h
	call GUI_SetCursorPos

	mov al, cl 					; cl = total events joined
	call GUI_PrintNumber

	lea si, Str_ConfirmPaymentKeys
	call GUI_SetHelpText

	mov ah, 4bh 				; simulate pressing left arrow in the payment dialog
	jmp ScrEventList_PaymentDialogKeyPressHandler

		@@exitsilently:
		jmp ScrMainMenu_Start

	@@Done:
	ret
ScrEventList_KeyPressHandler endp

ScrEventList_MsgBoxKeyPressHandler proc
	cmp ah, 1ch
	je @@Dismiss

	cmp ah, 01h
	je @@Dismiss

	ret

	@@Dismiss:
	jmp ScrEventList_Start
ScrEventList_MsgBoxKeyPressHandler endp

ScrEventList_PaymentDialogKeyPressHandler proc
	cmp ah, 4bh
	je @@LeftArrow

	cmp ah, 4dh
	je @@RightArrow

	cmp ah, 1ch
	je @@EnterKey

	cmp ah, 01h
	je @@EscKey_near

	jmp @@Done

	@@EscKey_near:	jmp @@EscKey

	@@LeftArrow:
	mov ah, 06h 	; clear selection area to blue
	mov al, 0
	mov bh, 17h 	; blue bg, white text
	mov cx, 103ch
	mov dx, 1047h
	int 10h

	mov bh, 47h 	; red bg, white text
	mov dl, 40h
	int 10h

	mov dx, 3c10h
	lea si, Str_YesNo
	call GUI_WriteAtPos

	call GUI_SetCursorPos
	ret

	@@RightArrow:
	mov ah, 06h 	; clear selection area to blue
	mov al, 0
	mov bh, 17h 	; blue bg, white text
	mov cx, 103ch
	mov dx, 1047h
	int 10h

	mov bh, 47h 	; red bg, white text
	mov cl, 44h
	mov dl, 47h
	int 10h

	mov dx, 3c10h
	lea si, Str_YesNo
	call GUI_WriteAtPos

	mov dx, 4410h
	call GUI_SetCursorPos
	ret

	@@EnterKey:
	mov ah, 03h
	mov bh, 0
	int 10h
	xchg dh, dl

	cmp dh, 3ch
	je @@Yes

	cmp dh, 44h
	je @@No

		@@Yes:
		mov al, EventList_Count

			@@joinloop:
			cmp al, 0
			je @@doneloop

			dec al
			call Misc_GetEventListDisplacement

			cmp EventList_0_Status[bx], 2 	; Selected
			jne @@joinloop

			mov EventList_0_Status[bx], 1 	; set the status back to Open

			mov dl, al 		; we're using al as counter, so we'll keep the original value first

			mov al, 3d 		; address to write = Ptcpnts addr + (participants count * 3)
			mul byte ptr EventList_0_PtcpntsCount[bx]

			inc byte ptr EventList_0_PtcpntsCount[bx]

			add bx, ax 		; add the address

			mov cx, 3d
			lea si, Session
			lea di, EventList_0_Ptcpnts[bx]
			cld
			rep movsb

			mov al, dl 		; give the original al value back

			jmp @@joinloop
			@@doneloop:

		call Misc_SaveData

		@@No:
		jmp ScrMainMenu_Start

	@@EscKey:
	jmp ScrEventList_Start

	@@Done:
	ret
ScrEventList_PaymentDialogKeyPressHandler endp

; =============================================
;	ScrEventList_GetTotalSelected
;	Input:
;		N/A
;	Output:
;		ax - total price
;		cl - total selected
; =============================================
ScrEventList_GetTotalSelected proc
	mov cx, 0 			; we'll use cx as the price accumulator
	mov dl, 0
	mov al, EventList_Count

	@@addloop:
	cmp al, 0
	je @@doneloop

	dec al
	call Misc_GetEventListDisplacement

	cmp EventList_0_Status[bx], 2 	; Selected
	jne @@addloop

	inc dl

	mov bl, EventList_0_Price[bx]
	mov bh, 00h
	add cx, bx

	jmp @@addloop

	@@doneloop:
	mov ax, cx
	mov cl, dl
	ret
ScrEventList_GetTotalSelected endp