ScrParticipantsList_Start proc
	call GUI_ClearScreen

	lea si, Str_ParticipantsList
	call GUI_SetTitle

	lea si, Str_ParticipantsListKeys
	call GUI_SetHelpText

	mov dx, 0102h
	lea si, Str_Event
	call GUI_WriteAtPos

	mov dh, 08h
	call GUI_SetCursorPos

	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov ah, 0eh
	lea si, EventList_0_Name[bx]
	mov cx, si
	add cx, 70d 	; store the address, limit to print only 70 chars

	@@eventprintloop:
	lodsb
	cmp al, 0
	je @@endeventprintloop
	int 10h
	cmp si, cx 		; see if already reached limit
	jl @@eventprintloop
	mov al, 247d	; ≈
	int 10h
	@@endeventprintloop:

	mov dx, 0104h
	call GUI_SetCursorPos

	mov ah, 0eh 	; teletype output
	mov ch, 20d 	; 20 values per column
	mov cl, EventList_0_PtcpntsCount[bx] 	; store the participants count for that event
	lea si, EventList_0_Ptcpnts[bx]
	mov bl, 0 		; i, start the loop with number 0

	@@colloop:
	cmp bl, cl
	jae @@colloopend

	mov al, bl 			
	inc al 					; +1 to print the correct number
	call GUI_PrintNumber

	mov al, '.'
	int 10h
	mov al, ' '
	int 10h
	mov al, 'T'
	int 10h
	mov al, 'P'
	int 10h

	lodsb
	call GUI_PrintTwoDigits
	lodsb
	call GUI_PrintTwoDigits
	lodsb
	call GUI_PrintTwoDigits

	inc bl 			; increment the counter
	dec ch 			; countdown how many rows until we shift column
	inc dl 			; move the cursor to the next row

	call GUI_SetCursorPos

	cmp ch, 0
	ja @@colloop

	add dh, 16d 	; move on to the next column
	mov dl, 04h 	; y = 4
	call GUI_SetCursorPos

	jmp @@colloop
	@@colloopend:

	mov GUI_KeyPressHandler, offset ScrParticipantsList_KeyPressHandler

	ret
ScrParticipantsList_Start endp

ScrParticipantsList_KeyPressHandler proc
	cmp ah, 01h
	je @@EscKey

	jmp @@Done

	@@EscKey:
	jmp ScrEventDetails_Start

	@@Done:
	ret
ScrParticipantsList_KeyPressHandler endp