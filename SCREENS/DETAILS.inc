ScrEventDetails_Start proc
	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov ah, 06h
	mov al, 00h
	mov bh, EventList_0_Color[bx]		; clear area with chosen colour
	mov cx, 0100h
	mov dx, 184fh
	int 10h

	lea si, Str_EventDetails
	call GUI_SetTitle

	mov dx, 0102h
	lea si, Str_Title
	call GUI_WriteAtPos

	mov al, 0
	mov cx, 0103h
	mov dx, 4e06h
	call GUI_DrawRect

	mov dx, 0108h
	lea si, Str_Description
	call GUI_WriteAtPos

	mov cl, 09h
	mov dx, 4e14h
	call GUI_DrawRect

	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	lea si, EventList_0_Name[bx]
	call ScrEventDetails_PrintTitle

	lea si, EventList_0_Desc[bx]
	call ScrEventDetails_PrintDesc

	mov dx, 0116h
	lea si, Str_Participants
	call GUI_WriteAtPos

	mov dh, 0fh
	call GUI_SetCursorPos

	mov al, EventList_0_PtcpntsCount[bx]
	call GUI_PrintNumber

	mov ah, 0eh
	mov al, '/'
	int 10h

	mov al, EventList_0_Limit[bx]
	call GUI_PrintNumber

	mov dh, 1bh
	lea si, Str_Price
	call GUI_WriteAtPos

	mov dh, 24h
	call GUI_SetCursorPos

	mov al, EventList_0_Price[bx]
	call GUI_PrintNumber

	mov dh, 2ch
	lea si, Str_Status
	call GUI_WriteAtPos

	mov al, SelectedEvent
	mov dh, 34h
	call GUI_PrintEventStatus

	mov GUI_KeyPressHandler, offset ScrEventDetails_KeyPressHandler

	cmp Session, 0ffh
	lea si, Str_DetailsKeys
	jne @@Students

	lea si, Str_DetailsAdminKeys

	@@Students:
	call GUI_SetHelpText
	ret
ScrEventDetails_Start endp

; =============================================
;	ScrEventDetails_PrintTitle
;	Input:
;		si - EventList_*_Title or Buffer
;	Output:
;		N/A
; =============================================
ScrEventDetails_PrintTitle proc
	call Misc_PushAllRegs

	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov ah, 06h 					; clear the title box area
	mov al, 0
	mov bh, EventList_0_Color[bx]
	mov cx, 0402h
	mov dx, 054dh
	int 10h

	mov dx, 0204h
	call GUI_SetCursorPos

	mov ah, 0eh
	mov cx, 76d 	; length of the text fields are 76 columns
	call @@titleloop

	cmp byte ptr [si-1], 0 		; if we already hit the end anyway then just ret
	je @@Done

	mov dl, 05h 	; move cursor to next line
	call GUI_SetCursorPos

	mov cx, 24d		; print the rest of the 24 chars in the title
	call @@titleloop

	@@Done:
	call Misc_PopAllRegs
	ret

	@@titleloop:
	lodsb
	cmp al, 0
	je @@endtitleloop
	int 10h
	loop @@titleloop
	@@endtitleloop:
	ret
ScrEventDetails_PrintTitle endp

; =============================================
;	ScrEventDetails_PrintDesc
;	Input:
;		si - EventList_*_Desc or Buffer
;	Output:
;		N/A
; =============================================
ScrEventDetails_PrintDesc proc
	call Misc_PushAllRegs

	push si 		; save original si input on stack first

	mov cx, 10d 	; set DescCharCount to all 0s
	lea si, ScrEventDetails_DescCharCount
	call Misc_ClearMemory

	pop si 			; get si input value back

	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov ah, 06h 					; clear the title box area
	mov al, 0
	mov bh, EventList_0_Color[bx]
	mov cx, 0a02h
	mov dx, 134dh
	int 10h

	mov dx, 020ah
	call GUI_SetCursorPos

	xor bx, bx 		; for DescCharCount indexing, set to starting point 0

	mov ah, 0eh
	mov cx, si
	add cx, 700d 	; let cx hold the addr of the end of the data

	@@descloop: 	; we'll keep track of cursor pointer internally in this func with dx,
	cmp si, cx		; so we don't have to keep querying the get cursor position int
	je @@enddescloop

	lodsb

	cmp al, 0
	je @@enddescloop

	cmp al, 0ah
	je @@newline
	
	int 10h
	inc dh

	cmp dh, 4eh 	; check if we're hitting the right border
	je @@newline
	jmp @@descloop

	@@newline:
	cmp dl, 13h 	; check if we're already at the bottom border
	je @@enddescloop

	sub dh, 02h 	; dh = current x pos - first col x (to get num of chars written in that row)
	mov ScrEventDetails_DescCharCount[bx], dh 	; save that char count, needed for char pos counting

	mov dh, 02h 	; x = 2, y = y++
	inc dl
	inc bl 			; ++ the indexing also
	call GUI_SetCursorPos

	jmp @@descloop
	
	@@enddescloop:
	sub dh, 02h 	; save the last printed line's char count (same operation as above)
	mov ScrEventDetails_DescCharCount[bx], dh

	call Misc_PopAllRegs
	ret
ScrEventDetails_PrintDesc endp

; making this proc public so ScrEventList's New Event can use this
ScrEventDetails_TitleEditMode proc
	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	lea si, EventList_0_Name[bx] 	; copy the contents of the name to a temp buffer for editing
	lea di, Buffer
	mov cx, 100d
	cld 			; clear direction flag
	rep movsb

	lea si, Buffer
	call ScrEventDetails_PrintTitle

	lea si, Str_TitleEditKeys
	call GUI_SetHelpText

	mov GUI_KeyPressHandler, offset ScrEventDetails_TitleEditKeyPressHandler

	call GUI_ShowCursor
	ret
ScrEventDetails_TitleEditMode endp

ScrEventDetails_KeyPressHandler proc
	cmp ah, 1ch
	je @@EnterKey

	cmp ah, 01h
	je @@EscKey

	jmp @@Done

	@@EnterKey:
	cmp Session, 0ffh
	jne @@Done

	lea si, Str_ActionMenuKeys
	call GUI_SetHelpText

	lea bx, ScrEventDetails_ActionMenuKeyPressHandler
	mov cx, 1b06h
	mov dx, 3512h
	lea si, Str_Empty ; we don't need the OK thing at the MsgBox, so we'll do some overwriting later
	call GUI_MsgBox

	mov al, 1 			; redraw the rect to remove the divider thing on both ends sticking out
	call GUI_DrawRect

	mov dx, 1d06h
	lea si, Str_ActionMenu
	call GUI_WriteAtPos

	mov ah, 06h 		; clear out the divider and the <OK> in MsgBox
	mov al, 0
	mov bh, 17h 		; blue background, white text
	mov cx, 101ch
	mov dx, 1134h
	int 10h

	mov bh, 47h 		; make the first selection part red
	mov cx, 081dh
	mov dx, 0833h
	int 10h

	mov dx, 1e08h
	lea si, Str_ActionMenuItems
	call GUI_WriteAtPos

	mov dx, 1d08h 			; put cursor at the end col of selection
	call GUI_SetCursorPos

	ret

	@@EscKey:
	jmp ScrEventList_Start

	@@Done:
	ret
ScrEventDetails_KeyPressHandler endp

ScrEventDetails_ActionMenuKeyPressHandler proc
	mov al, ah

	mov ah, 03h
	mov bh, 0
	int 10h

	xchg dh, dl 	; swap coordinate system because my program, my rules

	cmp al, 50h
	je @@DownArrow

	cmp al, 48h
	je @@UpArrow

	cmp al, 1ch
	je @@EnterKey

	cmp al, 01h
	je @@EscKey_near

	jmp @@Done

	@@EscKey_near: 	jmp @@EscKey
	@@Done_near: 	jmp @@Done

	@@DownArrow:
	cmp dl, 10h 	; 10h is the lowest y in the dialog
	jge @@Done_near

	mov dh, 1dh 			; move cursor down
	inc dl
	call GUI_SetCursorPos

	jmp @@RepaintMsgBox

	@@UpArrow:
	cmp dl, 08h 	; 08h is the highest y in the dialog
	jle @@Done_near

	mov dh, 1dh 			; move cursor up
	dec dl
	call GUI_SetCursorPos

	@@RepaintMsgBox:
	mov bl, dl 				; put the current cursor y to bl first

	mov ah, 06h 	; clear the msgbox to be fully blue
	mov al, 0
	mov bh, 17h 	; blue bg, white text
	mov cx, 081dh
	mov dx, 1033h
	int 10h

	mov bh, 47h 	; red bg, white text
	mov ch, bl 		; start y = cursor pos y
	mov cl, 1dh
	mov dh, bl 		; end y = cursor pos y
	mov dl, 33h
	int 10h

	mov dx, 1e08h
	lea si, Str_ActionMenuItems
	call GUI_WriteAtPos

	ret

	@@EnterKey:
	cmp dl, 08h
	je @@EditTitle

	cmp dl, 09h
	je @@EditDesc

	cmp dl, 0ah
	je @@SetBackgroundColour

	cmp dl, 0bh
	je @@SetTextColour_near

	cmp dl, 0ch
	je @@SetLimit_near

	cmp dl, 0dh
	je @@SetPrice_near

	cmp dl, 0eh
	je @@ToggleStatus_near

	cmp dl, 0fh
	je @@ShowParticipants_near

	cmp dl, 10h
	je @@DeleteEvent_near

	@@SetTextColour_near:		jmp @@SetTextColour
	@@SetLimit_near:			jmp @@SetLimit
	@@SetPrice_near: 			jmp @@SetPrice
	@@ToggleStatus_near:		jmp @@ToggleStatus
	@@ShowParticipants_near:	jmp @@ShowParticipants
	@@DeleteEvent_near:			jmp @@DeleteEvent

		@@EditTitle:
		call ScrEventDetails_Start
		jmp ScrEventDetails_TitleEditMode

		@@EditDesc:
		call ScrEventDetails_Start

		mov al, SelectedEvent
		call Misc_GetEventListDisplacement

		lea si, EventList_0_Desc[bx] 	; copy the contents of the desc to a temp buffer for editing
		lea di, Buffer
		mov cx, 700d
		cld 			; clear direction flag
		rep movsb

		lea si, Buffer
		call ScrEventDetails_PrintDesc

		lea si, Str_DescEditKeys
		call GUI_SetHelpText

		mov GUI_KeyPressHandler, offset ScrEventDetails_DescEditKeyPressHandler

		call GUI_ShowCursor
		ret

		@@SetBackgroundColour:
		call ScrEventDetails_Start

		lea si, Str_ActionMenuKeys
		call GUI_SetHelpText

		lea bx, ScrEventDetails_BgColorMenuKeyPressHandler
		mov cx, 1f06h
		mov dx, 3011h
		lea si, Str_Empty
		call GUI_MsgBox

		mov al, 1
		call GUI_DrawRect 		; redraw border to remov the divider

		mov ah, 06h 	; clear the OK and divider thing
		mov al, 0
		mov bh, 17h 	; blue bg white text
		mov cx, 0f20h
		mov dx, 102fh
		int 10h

		mov bh, 07h 	; start with black bg white text
		mov cx, 0823h
		mov dx, 082eh

			@@rowloop:
			cmp ch, 0fh
			jae @@endrowloop

			int 10h

			inc ch 			; move to next row
			inc dh
			add bh, 10h 	; change background colour

			jmp @@rowloop

			@@endrowloop:
			mov bh, 70h 	; for last colour (white), we need to give it black text
			int 10h

		mov dx, 2408h
		lea si, Str_Colours
		call GUI_WriteAtPos

		mov dx, 2108h
		call GUI_SetCursorPos

		mov ah, 0eh
		mov al, 16d 	; ►
		int 10h

		ret

		@@SetTextColour:
		call ScrEventDetails_Start

		lea si, Str_ActionMenuKeys
		call GUI_SetHelpText

		lea bx, ScrEventDetails_FgColorMenuKeyPressHandler
		mov cx, 1f06h
		mov dx, 3011h
		lea si, Str_Empty
		call GUI_MsgBox

		mov al, 1
		call GUI_DrawRect 		; redraw border to remov the divider

		mov ah, 06h 	; clear the OK and divider thing
		mov al, 0
		mov bh, 17h 	; blue bg white text
		mov cx, 0f20h
		mov dx, 102fh
		int 10h

		mov bh, 10h 	; start with blue bg black text
		mov cx, 0823h
		mov dx, 082eh

			@@textcolorrowloop:
			cmp ch, 0fh
			jae @@textcolorendrowloop

			int 10h

			inc ch 			; move to next row
			inc dh
			inc bh 			; change text colour

			jmp @@textcolorrowloop
			@@textcolorendrowloop:

		mov dx, 2408h
		lea si, Str_Colours
		call GUI_WriteAtPos

		mov dx, 2108h
		call GUI_SetCursorPos

		mov ah, 0eh
		mov al, 16d 	; ►
		int 10h

		ret

		ret

		@@SetLimit:
		call ScrEventDetails_Start

		lea bx, ScrEventDetails_LimitDialogKeyPressHandler
		mov cx, 0808h
		mov dx, 4710h
		lea si, Str_FillLimit
		call GUI_MsgBox

		mov ah, 06h 	; make the text field
		mov al, 0
		mov bh, 47h 	; red bg white text
		mov cx, 0c0bh
		mov dx, 0c44h
		int 10h

		mov word ptr Buffer, 0d 	; put 00 00 to buffer

		mov dx, 0b0ch
		call GUI_SetCursorPos

		call GUI_ShowCursor

		lea si, Str_TPDialogKeys
		call GUI_SetHelpText

		ret

		@@SetPrice:
		call ScrEventDetails_Start

		lea bx, ScrEventDetails_PriceDialogKeyPressHandler
		mov cx, 0708h
		mov dx, 4810h
		lea si, Str_FillPrice
		call GUI_MsgBox

		mov ah, 06h 	; make the text field
		mov al, 0
		mov bh, 47h 	; red bg white text
		mov cx, 0c0ah
		mov dx, 0c45h
		int 10h

		mov word ptr Buffer, 0 	; put 00 00 to buffer

		mov dx, 0a0ch
		lea si, Str_RM
		call GUI_WriteAtPos

		mov dh, 0ch
		call GUI_SetCursorPos

		call GUI_ShowCursor

		lea si, Str_TPDialogKeys
		call GUI_SetHelpText

		ret

		@@ToggleStatus:
		mov al, SelectedEvent
		call Misc_GetEventListDisplacement

		cmp EventList_0_Status[bx], 1
		je @@makeitzero

		mov EventList_0_Status[bx], 2 	; so after dec, then it will be 1

			@@makeitzero:
			dec EventList_0_Status[bx]

		call Misc_SaveData
		jmp ScrEventDetails_Start

		@@DeleteEvent:
		mov al, SelectedEvent 		; get the current event, put it as destination
		call Misc_GetEventListDisplacement

		lea di, EventList_0_Name[bx]

		inc al
		call Misc_GetEventListDisplacement

		lea si, EventList_0_Name[bx] 	; get the next event as source

		mov ah, 00h
		mov al, EventList_Count
		dec al 				; SelectedEvent is zero-indexed, so we gotta minus one for EventList_Count
		sub al, SelectedEvent
		mov bx, EventList_Size
		mul bx 					; ax = (number of events - currently selected event) * size of an event

		mov cx, ax 				; that will be number of bytes we have to move forward to cover up
		 						; the deleted event
		cld 					; clear direction flag
		rep movsb

		dec EventList_Count 	; decrement the number of events
		mov al, EventList_Count
		call Misc_GetEventListDisplacement 	; get the displacement of the previously last event

		mov cx, EventList_Size 	; clear the memory for that last event
		lea si, EventList_0_Name[bx]
		call Misc_ClearMemory

		call Misc_SaveData

		jmp ScrEventList_Start

		@@ShowParticipants:
		jmp ScrParticipantsList_Start

	@@EscKey:
	jmp ScrEventDetails_Start

	@@Done:
	ret
ScrEventDetails_ActionMenuKeyPressHandler endp

ScrEventDetails_TitleEditKeyPressHandler proc
	mov bl, ah

	mov ah, 03h 	; get cursor pos
	mov bh, 0
	int 10h

	xchg dh, dl 	; swap coordinate system because my program, my rules

	cmp bl, 4bh
	je @@LeftArrow

	cmp bl, 4dh
	je @@RightArrow

	cmp bl, 48h
	je @@UpArrow

	cmp bl, 50h
	je @@DownArrow

	cmp bl, 0eh
	je @@Backspace_near

	cmp bl, 01h
	je @@EscKey_near

	sub al, 20h 	; al >= 20h, al <= 7eh (printable ASCII range)
	cmp al, 94d
	jbe @@PrintableChar_near

	jmp @@Done

	@@Backspace_near:		jmp @@Backspace
	@@EscKey_near:			jmp @@EscKey
	@@PrintableChar_near:	jmp @@PrintableChar
	@@Done_near:			jmp @@Done

	@@LeftArrow:
	cmp dh, 02h
	jle @@leftborder

	dec dh
	call GUI_SetCursorPos
	ret

		@@leftborder:
		cmp dl, 04h 	; check if we're already at the first row
		jle @@Done_near

		mov dx, 4d04h
		call GUI_SetCursorPos
		ret

	@@RightArrow:
	call @@GetPos

	cmp Buffer[bx], 0 	; if cursor pointing at a null char, it's already end of string
	je @@Done_near

	cmp bx, 100 		; if already at 100th chars, don't do anything
	je @@Done_near

	cmp dh, 4dh 		; go to the next line if we hit the right border
	jne @@notrightborder

	mov dx, 0105h 		; put at x=1, y=5 because the next line will inc x for us

		@@notrightborder:
		inc dh

	call GUI_SetCursorPos
	ret

	@@UpArrow:
	cmp dl, 04h
	jle @@Done_near

	dec dl
	call GUI_SetCursorPos
	ret

	@@DownArrow:
	cmp dl, 05h
	jge @@Done_near

	cmp Buffer[75], 00h 	; check if a second line should even exist
	je @@Done_near

	call @@GetPos

	mov dl, 05h 			; set y = 05h

	add bx, 76d				; see if going to next line exceeds 100d
	cmp bx, 100d
	jge @@getlastchar

	call GUI_SetCursorPos 	; we already made y = 05h

	ret

		@@getlastchar:
		mov bx, 76d
		@@getlastcharloop:
		cmp Buffer[bx], 00h
		je @@getlastcharloopend
		cmp bx, 100d
		jge @@getlastcharloopend
		inc bl
		jmp @@getlastcharloop
		@@getlastcharloopend:
		mov dh, bl 		; cursor x pos (dh) = char index (bl) - length of one line (76d) + title
		sub dh, 74d 	; 					  box left border (2d)
		call GUI_SetCursorPos
		ret

	@@Done_near2:	jmp @@Done

	@@Backspace:
	cmp dx, 0204h 		; what are you trying to delete at index 0 ??
	je @@Done_near2

	call @@GetPos

		@@bsloop:
		cmp Buffer[bx], 00h
		je @@endbsloop

		cmp bx, 100d
		je @@endbsloop

		mov al, Buffer[bx] 		; move all character after cursor one byte ahead
		mov Buffer[bx-1], al
		inc bl
		jmp @@bsloop

		@@endbsloop:
		mov Buffer[bx-1], 00h

	call @@LeftArrow 	; simulate pressing left arrow

	lea si, Buffer
	call ScrEventDetails_PrintTitle

	call GUI_SetCursorPos 			; after printing title, cursor pos changes, so just set it back
	ret

	@@EscKey:
	call GUI_HideCursor

	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov cx, 100d
	lea si, Buffer
	lea di, EventList_0_Name[bx]
	rep cmpsb
	je @@nochanges

	lea si, Str_YesNoKeys
	call GUI_SetHelpText

	mov bx, offset ScrEventDetails_TitleEditConfirmKeyPressHandler
	mov cx, 1509h
	mov dx, 3b0fh
	lea si, Str_KeepChanges
	call GUI_MsgBox

	mov ah, 4bh 	; simulate left arrow press
	jmp ScrEventDetails_TitleEditConfirmKeyPressHandler

		@@nochanges:
		jmp ScrEventDetails_Start

	@@PrintableChar:
	cmp Buffer[99], 00h 	; if there's a char at 100th position, title is full
	jne @@Done

	add al, 20h		; add back the al that we've subtracted when used for range comparison far above

	call @@GetPos
	mov ah, bl 		; put the pointed index in ah

	mov bx, 99d 	; start shifting stuff from bx-1 to bx

		@@shiftloop:
		cmp bl, ah 				; if we hit the pointed index, time to stop
		je @@endshiftloop

		mov cl, Buffer[bx-1] 		; move all character after cursor one byte forward
		mov Buffer[bx], cl
		dec bl
		jmp @@shiftloop
		@@endshiftloop:
		mov Buffer[bx], al 			; al = ASCII of the pressed key according to BIOS
									; (not always accurate, but it'll do)

	call @@RightArrow 		; simulate pressing right arrow

	lea si, Buffer
	call ScrEventDetails_PrintTitle

	call GUI_SetCursorPos
	ret

	@@Done:
	ret

	@@GetPos: 	; puts the index of str pointed by current cursor pos (dx) into bx
	mov bh, 00h
	mov bl, dh 	; let bx = current cursor x pos
	sub bl, 02h

	cmp dl, 05h
	jge @@secondline

	ret

		@@secondline:
		add bx, 76d
		ret
ScrEventDetails_TitleEditKeyPressHandler endp

ScrEventDetails_TitleEditConfirmKeyPressHandler proc
	cmp ah, 4bh
	je @@LeftArrow

	cmp ah, 4dh
	je @@RightArrow

	cmp ah, 1ch
	je @@EnterKey

	jmp @@Done

	@@LeftArrow:
	mov ah, 06h 	; clear selection area to blue
	mov al, 0
	mov bh, 17h 	; blue bg, white text
	mov cx, 0e2eh
	mov dx, 0e39h
	int 10h

	mov bh, 47h 	; red bg, white text
	mov dl, 32h
	int 10h

	mov dx, 2e0eh
	lea si, Str_YesNo
	call GUI_WriteAtPos

	call GUI_SetCursorPos
	ret

	@@RightArrow:
	mov ah, 06h 	; clear selection area to blue
	mov al, 0
	mov bh, 17h 	; blue bg, white text
	mov cx, 0e2eh
	mov dx, 0e39h
	int 10h

	mov bh, 47h 	; red bg, white text
	mov cl, 36h
	mov dl, 39h
	int 10h

	mov dx, 2e0eh
	lea si, Str_YesNo
	call GUI_WriteAtPos

	mov dx, 360eh
	call GUI_SetCursorPos
	ret

	@@EnterKey:
	mov ah, 03h
	mov bh, 0
	int 10h
	xchg dh, dl

	cmp dh, 2eh
	je @@SaveChanges

	cmp dh, 36h
	je @@NoSaveChanges

		@@SaveChanges:
		mov al, SelectedEvent
		call Misc_GetEventListDisplacement

		mov cx, 100d				; copy from buffer back to data
		lea si, Buffer
		lea di, EventList_0_Name[bx]
		cld
		rep movsb 					; intentional fallthrough here

		call Misc_SaveData

		@@NoSaveChanges:
		jmp ScrEventDetails_Start

	@@Done:
	ret
ScrEventDetails_TitleEditConfirmKeyPressHandler endp

ScrEventDetails_DescEditKeyPressHandler proc
	mov bl, ah

	mov ah, 03h 	; get cursor pos
	mov bh, 0
	int 10h

	xchg dh, dl 	; swap coordinate system because my program, my rules

	cmp bl, 4bh
	je @@LeftArrow

	cmp bl, 4dh
	je @@RightArrow

	cmp bl, 48h
	je @@UpArrow_near

	cmp bl, 50h
	je @@DownArrow_near

	cmp bl, 0eh
	je @@Backspace_near

	cmp bl, 01h
	je @@EscKey_near

	cmp bl, 1ch
	je @@EnterKey_near

	sub al, 20h 	; al >= 20h, al <= 7eh (printable ASCII range)
	cmp al, 94d
	jbe @@PrintableChar_near

	jmp @@Done

	@@UpArrow_near:			jmp @@UpArrow
	@@DownArrow_near:		jmp @@DownArrow
	@@Backspace_near:		jmp @@Backspace
	@@EscKey_near:			jmp @@EscKey
	@@EnterKey_near:		jmp @@EnterKey
	@@PrintableChar_near:	jmp @@PrintableChar
	@@Done_near:			jmp @@Done

	@@LeftArrow:
	cmp dh, 02h
	jle @@leftborder

	dec dh
	call GUI_SetCursorPos
	ret

		@@leftborder:
		cmp dl, 0ah 	; check if we're already at the first row
		jle @@Done_near

		dec dl 			; decrement cursor y pos

		mov bh, 00h 	; let bx = current description row index
		mov bl, dl
		sub bl, 0ah

		mov dh, ScrEventDetails_DescCharCount[bx]

		cmp ScrEventDetails_DescCharCount[bx], 76d
		jne @@notfull

		mov dh, 4bh

		@@notfull:
		add dh, 02h

		call GUI_SetCursorPos
		ret

	@@RightArrow:
	call @@GetPos

	cmp Buffer[bx], 0 	; if cursor pointing at a null char, it's already end of string
	je @@Done_near

	cmp bx, 700d 		; if already at 700th chars, don't do anything
	je @@Done_near

	cmp dh, 4dh 		; go to the next line if we hit the right border
	je @@rightborder

	cmp Buffer[bx], 0ah 	; go to next line if we're at a 0ah
	je @@rightborder

	inc dh

	call GUI_SetCursorPos
	ret

		@@rightborder:
		cmp dl, 13h 		; we're already at the last line, can't go no more next line
		je @@Done_near

		mov dh, 02h
		inc dl 				; x=2, y++
		call GUI_SetCursorPos
		ret

	@@UpArrow:
	cmp dl, 0ah
	jle @@Done_near

	dec dl

	mov bh, 00h 	; let bx = (current y pos - 1) - first row pos (0ah)
	mov bl, dl
	sub bl, 0ah

	mov ah, dh 		; let ah = current col in desc
	sub ah, 02h

	cmp ah, ScrEventDetails_DescCharCount[bx]
	jbe @@justgoup 	; if current x is greater than what the row above has, do some stuff

	mov dh, ScrEventDetails_DescCharCount[bx]
	add dh, 02h

		@@justgoup:

	call GUI_SetCursorPos
	ret

	@@DownArrow:
	cmp dl, 13h
	jge @@Done_near2

	mov bh, 00h 			; let bx = current y pos - first row pos (0ah)
	mov bl, dl
	sub bl, 0ah

	mov ah, dh 				; let ah = current col in desc
	sub ah, 02h

	cmp ah, ScrEventDetails_DescCharCount[bx+1] 	; check the char count of next row
	jb @@justgodown

	cmp ScrEventDetails_DescCharCount[bx+1], 0 ; ok it's 0, but does it mean newline or no more data?
	jne @@justkindagodown

	cmp ScrEventDetails_DescCharCount[bx], 76d 	; first check if the current line ends "prematurely"
	mov dh, 02h 		; either way, set x=2, it's the same for both conditions
	je @@justgodown 	; the next line has to exist anyway if the current one is full

	mov ch, 00h
	mov cl, ScrEventDetails_DescCharCount[bx]

	call @@GetPos 		; we're assuming we're in the first pos in the current line now

	add bx, cx				; then add with current line's length to get end of the line

	cmp Buffer[bx], 00h 	; so is it end of the desc (00h) or a newline (0ah) ?
	je @@Done_near2 		; if end, then don't do stuff
	jmp @@justgodown 		; otherwise, go to x=2, y++

		@@justkindagodown:
		mov dh, ScrEventDetails_DescCharCount[bx+1]
		add dh, 02h

		@@justgodown:
		inc dl

	call GUI_SetCursorPos

	ret

	@@Done_near2:	jmp @@Done

	@@Backspace:
	cmp dx, 020ah 		; what are you trying to delete at index 0 ??
	je @@Done_near2

	call @@GetPos

		@@bsloop:
		cmp Buffer[bx], 00h
		je @@endbsloop

		cmp bx, 700d
		je @@endbsloop

		mov al, Buffer[bx] 		; move all character after cursor one byte ahead
		mov Buffer[bx-1], al
		inc bx
		jmp @@bsloop

		@@endbsloop:
		mov Buffer[bx-1], 00h

	call @@LeftArrow 	; simulate pressing left arrow

	lea si, Buffer
	call ScrEventDetails_PrintDesc

	call GUI_SetCursorPos 			; after printing title, cursor pos changes, so just set it back
	ret

	@@EscKey:
	call GUI_HideCursor

	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov cx, 700d
	lea si, Buffer
	lea di, EventList_0_Desc[bx]
	rep cmpsb
	je @@nochanges

	lea si, Str_YesNoKeys
	call GUI_SetHelpText

	mov bx, offset ScrEventDetails_DescEditConfirmKeyPressHandler
	mov cx, 1509h
	mov dx, 3b0fh
	lea si, Str_KeepChanges
	call GUI_MsgBox

	mov ah, 4bh 	; simulate left arrow press
	jmp ScrEventDetails_DescEditConfirmKeyPressHandler

		@@nochanges:
		jmp ScrEventDetails_Start

	@@EnterKey:
	mov al, 0eah 			; after +20h, it will be =0ah (overflow)

	@@PrintableChar:
	cmp Buffer[699], 00h 	; if there's a char at 700th position, title is full
	jne @@Done

	add al, 20h		; add back the al that we've subtracted when used for range comparison far above

	mov ch, al 		; keep this pressed value in ch

	call @@GetPos
	mov ax, bx 		; put the pointed index in ax

	mov bx, 699d 	; start shifting stuff from bx-1 to bx

		@@shiftloop:
		cmp bx, ax 				; if we hit the pointed index, time to stop
		je @@endshiftloop

		mov cl, Buffer[bx-1] 		; move all character after cursor one byte forward
		mov Buffer[bx], cl
		dec bx
		jmp @@shiftloop
		@@endshiftloop:
		mov Buffer[bx], ch 			; ch = ASCII of the pressed key according to BIOS
									; (not always accurate, but it'll do)

	call @@RightArrow 		; simulate pressing right arrow

	lea si, Buffer
	call ScrEventDetails_PrintDesc

	call GUI_SetCursorPos
	ret

	@@Done:
	ret

	@@GetPos: 	; puts the index of str pointed by current cursor pos (dx) into bx
	mov bh, 00h ; let bx = current cursor row index
	mov bl, dl
	sub bl, 0ah

	push ax 	; keep the original values and pop em back later
	push cx

	mov ax, 0 	; let ax be an accumulator for every row added up
	mov ch, 00h

		@@getposloop:
		dec bl 			; first loop: dec first, because the current line's one is calculated later
		cmp bl, 00h
		jl @@getposloopend

		mov cl, ScrEventDetails_DescCharCount[bx]
		add ax, cx

			; we have to account for the 0ah char, so if it's not full (76d), means a newline is
			; used, so we +1 the ax char counter
			cmp ScrEventDetails_DescCharCount[bx], 76d
			jae @@isfull
			inc ax
			@@isfull:

		jmp @@getposloop
		@@getposloopend:

	mov bl, dh 	; let bx = current cursor x pos
	sub bl, 02h ; subtract 02h to get the current index in the current row
	add bx, ax 	; add the accumulated value to bx

	pop cx
	pop ax
	ret
ScrEventDetails_DescEditKeyPressHandler endp

ScrEventDetails_DescEditConfirmKeyPressHandler proc
	cmp ah, 1ch
	je @@EnterKey

	jmp ScrEventDetails_TitleEditConfirmKeyPressHandler

	@@EnterKey:
	mov ah, 03h
	mov bh, 0
	int 10h
	xchg dh, dl

	cmp dh, 2eh
	je @@SaveChanges

	cmp dh, 36h
	je @@NoSaveChanges

		@@SaveChanges:
		mov al, SelectedEvent
		call Misc_GetEventListDisplacement

		mov cx, 700d				; copy from buffer back to data
		lea si, Buffer
		lea di, EventList_0_Desc[bx]
		cld
		rep movsb 					; intentional fallthrough here

		call Misc_SaveData

		@@NoSaveChanges:
		jmp ScrEventDetails_Start

	@@Done:
	ret
ScrEventDetails_DescEditConfirmKeyPressHandler endp

ScrEventDetails_BgColorMenuKeyPressHandler proc
	mov al, ah

	mov ah, 03h
	mov bh, 0
	int 10h

	xchg dh, dl 	; swap coordinate system because my program, my rules

	cmp al, 50h
	je @@DownArrow

	cmp al, 48h
	je @@UpArrow

	cmp al, 1ch
	je @@EnterKey

	cmp al, 01h
	je @@EscKey

	jmp @@Done

	@@DownArrow:
	cmp dl, 0fh
	jae @@Done

	mov bl, dl

	mov ah, 06h 	; clear the arrow row
	mov al, 0
	mov bh, 17h
	mov cx, 0821h
	mov dx, 0f21h
	int 10h

	mov dh, 21h
	mov dl, bl
	inc dl
	call GUI_SetCursorPos

	mov ah, 0eh
	mov al, 16d 	; ►
	int 10h

	ret

	@@UpArrow:
	cmp dl, 08h
	jbe @@Done

	mov bl, dl

	mov ah, 06h 	; clear the arrow row
	mov al, 0
	mov bh, 17h
	mov cx, 0821h
	mov dx, 0f21h
	int 10h

	mov dh, 21h
	mov dl, bl
	dec dl
	call GUI_SetCursorPos

	mov ah, 0eh
	mov al, 16d 	; ►
	int 10h

	ret

	@@EnterKey:
	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov ah, 00h
	mov al, EventList_0_Color[bx]
	mov cl, 10h
	div cl
	mov ch, ah 		; split al = bg colour (don't need it), ch = text color

	mov al, dl 		; take the y, minus by the first item pos (08h)
	sub al, 08h
	mov cl, 10h
	mul cl

	add al, ch
	mov EventList_0_Color[bx], al

	call Misc_SaveData

	jmp ScrEventDetails_Start

	@@EscKey:
	jmp ScrEventDetails_Start

	@@Done:
	ret
ScrEventDetails_BgColorMenuKeyPressHandler endp

ScrEventDetails_FgColorMenuKeyPressHandler proc
	cmp ah, 1ch
	je @@EnterKey

	jmp ScrEventDetails_BgColorMenuKeyPressHandler 	; inherit that function

	@@EnterKey:
	mov ah, 03h 	; get cursor pos
	mov bh, 0
	int 10h

	xchg dh, dl 	; swap coord system

	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov ah, 00h
	mov al, EventList_0_Color[bx]
	mov cl, 10h
	div cl
	mov ch, al 		; split al/ch = bg colour, ah = text color (don't need it)

	mov cl, 10h
	mul cl 			; al is already bg colour

	mov ah, dl 		; take the y, minus by the first item pos (08h)
	sub ah, 08h

	add al, ah
	mov EventList_0_Color[bx], al

	call Misc_SaveData

	jmp ScrEventDetails_Start

	@@Done:
	ret
ScrEventDetails_FgColorMenuKeyPressHandler endp

ScrEventDetails_LimitDialogKeyPressHandler proc
	mov bl, ah

	mov ah, 03h
	mov bh, 0
	int 10h

	cmp bl, 01h
	je @@EscKey

	cmp bl, 1ch
	je @@EnterKey

	cmp bl, 0eh
	je @@Backspace

	sub bl, 02h 	; bl >= 02h, bl <= 0bh 
	cmp bl, 09h
	jbe @@Number

	jmp @@Done

	@@EscKey:
	call GUI_HideCursor
	jmp ScrEventDetails_Start

	@@EnterKey:
	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov al, Buffer
	mov EventList_0_Limit[bx], al

	call Misc_SaveData

	call GUI_HideCursor

	jmp ScrEventDetails_Start

	@@Backspace:
	mov dx, 0b0ch
	call GUI_SetCursorPos

	mov ah, 0ah 	; print without moving cursor
	mov al, ' '
	mov cx, 3d 		; fill the three digits
	int 10h

	mov ah, 00h
	mov al, Buffer

	mov bl, 10d
	div bl 			; al = result, ah = remainder (useless here)

	mov Buffer, al

	cmp al, 0
	je @@Done

	call GUI_PrintNumber
	ret

	@@Number:
	sub al, 30h 	; get numeric value from ASCII code

	mov bl, al 		; put it to bl

	mov al, Buffer
	mov bh, 10d
	mul bh

	mov bh, 00h
	add ax, bx 		; we use 16-bits in case it's actually >255, that we can check it properly

	cmp ax, 100d
	ja @@Done

	mov Buffer, al

	mov dx, 0b0ch
	call GUI_SetCursorPos
	call GUI_PrintNumber
	ret

	@@Done:
	ret
ScrEventDetails_LimitDialogKeyPressHandler endp

ScrEventDetails_PriceDialogKeyPressHandler proc
	mov bl, ah

	mov ah, 03h
	mov bh, 0
	int 10h

	cmp bl, 01h
	je @@EscKey

	cmp bl, 1ch
	je @@EnterKey

	cmp bl, 0eh
	je @@Backspace

	sub bl, 02h 	; bl >= 02h, bl <= 0bh 
	cmp bl, 09h
	jbe @@Number

	jmp @@Done

	@@EscKey:
	call GUI_HideCursor
	jmp ScrEventDetails_Start

	@@EnterKey:
	mov al, SelectedEvent
	call Misc_GetEventListDisplacement

	mov al, Buffer
	mov EventList_0_Price[bx], al

	call Misc_SaveData

	call GUI_HideCursor

	jmp ScrEventDetails_Start

	@@Backspace:
	mov dx, 0c0ch
	call GUI_SetCursorPos

	mov ah, 0ah 	; print without moving cursor
	mov al, ' '
	mov cx, 3d 		; fill the three digits
	int 10h

	mov ah, 00h
	mov al, Buffer

	mov bl, 10d
	div bl 			; al = result, ah = remainder (useless here)

	mov Buffer, al

	cmp al, 0
	je @@Done

	call GUI_PrintNumber
	ret

	@@Number:
	sub al, 30h 	; get numeric value from ASCII code

	mov bl, al 		; put it to bl

	mov al, Buffer
	mov bh, 10d
	mul bh

	mov bh, 00h
	add ax, bx 		; we use 16-bits in case it's actually >255, that we can check it properly

	cmp ax, 255d
	ja @@Done

	mov Buffer, al

	mov dx, 0c0ch
	call GUI_SetCursorPos
	call GUI_PrintNumber
	ret

	@@Done:
	ret
ScrEventDetails_PriceDialogKeyPressHandler endp