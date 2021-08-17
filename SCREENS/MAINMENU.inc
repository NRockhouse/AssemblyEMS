ScrMainMenu_Start proc
	call GUI_ClearScreen

	lea si, Str_MainMenu
	call GUI_SetTitle

	mov dx, 190bh
	lea si, Str_AssemblyEMS
	call GUI_WriteAtPos

	mov dx, 0d0dh
	lea si, Str_EMS
	call GUI_WriteAtPos

	mov cx, 17d 	; loop 17 times
	mov dx, 0427h
	mov bh, 0
	mov al, 179d 	; vertical single border character

	@@drawmidpipe:
	mov ah, 2		
	int 10h
	mov ah, 0eh
	int 10h
	inc dh
	loop @@drawmidpipe

	mov ah, 2		; put cursor beside Students box
	mov dx, 0629h
	int 10h

	mov ah, 06h 	; draw blue boxes (though only used as border)
	mov al, 0
	mov bh, 17		; blue bg, white text
	mov cx, 042bh 	; Students border
	mov dx, 084ch
	int 10h

	mov ch, 0ah 	; Staff border
	mov dh, 0eh
	int 10h

	mov ch, 10h 	; Exit border
	mov dh, 14h
	int 10h

	call ScrMainMenu_UpdateBoxes

	lea si, Str_MainMenuKeys
	call GUI_SetHelpText

	mov GUI_KeyPressHandler, offset ScrMainMenu_KeyPressHandler

	ret
ScrMainMenu_Start endp

ScrMainMenu_UpdateBoxes proc
	mov ah, 06h 	; clean the row where the play symbol appears
	mov al, 0
	mov bh, 01h 	; black bg, blue text
	mov cx, 0629h
	mov dx, 1229h
	int 10h

	mov bh, 07h 	; black bg, white text
	mov cx, 052ch	; Students internal box
	mov dx, 074bh
	int 10h 		; applying black box inside the blue box (so to make the blue just a border)

	mov ch, 0bh 	; Staff internal box
	mov dh, 0dh
	int 10h

	mov ch, 11h 	; Exit internal box
	mov dh, 13h
	int 10h

	mov ah, 0eh
	mov al, 16d		; play symbol / right arrow pointing thingy
	int 10h

	mov ah, 03h 	; get cursor position (dx = y,x)
	mov bh, 0
	int 10h

	mov ah, 06h 	; make the selected box blue
	mov al, 0
	mov bh, 17h 	; blue bg, white text
	mov cx, dx 		; from the cursor position, calculate the cx and dx
	dec ch
	add cl, 2d
	inc dh
	add dl, 33d
	int 10h

	mov dx, 2d06h 			; write text in the boxes
	lea si, Str_Students
	call GUI_WriteAtPos

	mov dl, 0ch
	lea si, Str_Staff
	call GUI_WriteAtPos

	mov dl, 12h
	lea si, Str_Exit
	call GUI_WriteAtPos

	ret
ScrMainMenu_UpdateBoxes endp

ScrMainMenu_KeyPressHandler proc
	mov al, ah

	mov ah, 03h
	mov bh, 0
	int 10h

	cmp al, 50h
	je @@DownArrow

	cmp al, 48h
	je @@UpArrow

	cmp al, 1ch
	je @@EnterKey

	jmp @@Done

	@@DownArrow:
	cmp dh, 12h		; don't do anything if already at lowest option
	jge @@Done

	mov ah, 02h
	add dh, 6d
	mov dl, 29h
	int 10h

	jmp ScrMainMenu_UpdateBoxes ; we'll just use the ret at UpdateBoxes so don't have to call

	@@UpArrow:
	cmp dh, 06h		; don't do anything if already at highest option
	jle @@Done

	mov ah, 02h
	sub dh, 6d
	mov dl, 29h
	int 10h
	
	jmp ScrMainMenu_UpdateBoxes

	@@EnterKey:
	cmp dh, 06h
	je @@StudentsSelected

	cmp dh, 0ch
	je @@StaffSelected

	jmp Quit

	@@StudentsSelected:
	lea bx, ScrMainMenu_TPDialogKeyPressHandler
	mov cx, 1108h
	mov dx, 3d10h
	lea si, Str_EnterTPNumber
	call GUI_MsgBox

	mov ah, 06h 	; make obvious input box (unlike CIMB bank and their ATM kiosk)
	mov al, 0
	mov bh, 47h		; red bg, white text
	mov cx, 0c14h
	mov dx, 0c3ah
	int 10h

	mov dx, 140ch
	lea si, Str_TP
	call GUI_WriteAtPos

	mov ah, 02h 	; put cursor behind "TP"
	mov bh, 0
	mov dx, 0c16h
	int 10h

	call GUI_ShowCursor

	lea si, Str_TPDialogKeys
	jmp GUI_SetHelpText

	@@StaffSelected:
	mov Session, 0ffh 		; = ffh, but TASM needs a 0 to understand that
	call Misc_ReadData
	jmp ScrEventList_Start

	@@Done:
	ret
ScrMainMenu_KeyPressHandler endp

ScrMainMenu_TPDialogKeyPressHandler proc
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
	jmp ScrMainMenu_Start

	@@EnterKey:
	cmp dl, 1ch
	jne @@ShowError

	call GUI_HideCursor
	call Misc_ReadData
	jmp ScrEventList_Start

		@@ShowError:
		call ScrMainMenu_Start

		lea bx, ScrMainMenu_MsgBoxKeyPressHandler
		mov cx, 1c09h
		mov dx, 330fh
		lea si, Str_InvalidTPNumber
		call GUI_MsgBox
		jmp GUI_HideCursor

	@@Backspace:
	cmp dl, 16h
	jle @@Done

	mov ah, 02h 	; set cursor position to x-1 , y
	dec dl
	int 10h

	mov ah, 0ah 	; print without moving cursor
	mov al, ' '
	mov cx, 1d 		; print once
	int 10h
	ret

	@@Number:
	cmp dl, 1ch
	jge @@Done

	mov ah, 0eh 	; al is already the numeric ASCII value
	int 10h 		; print it

	sub al, 30h 	; get numeric value from ASCII code
	mov dh, al 		; need the numeric value for later use, put it to dh first

	mov al, dl 		; make ax = numeric position in the TP number
	sub al, 16h
	mov ah, 00h
	mov bl, 2d 		; ax / 2
	div bl 			; al = session byte position, ah = high or low 4 bits

	mov bh, 00h 	; use bx as session byte position / indexing cuz Intel
	mov bl, al

	cmp ah, 1
	je @@LowBits

	mov al, dh 		; get the numeric value from dh
	mov ah, 10d		; multiply it with 10d
	mul ah
	mov Session[bx], al 	; put that into the Session
	ret

		@@LowBits:
		add Session[bx], dh 	; just add the numeric value onto the existing byte

	@@Done:
	ret
ScrMainMenu_TPDialogKeyPressHandler endp

ScrMainMenu_MsgBoxKeyPressHandler proc
	cmp ah, 01h 	; Esc
	je @@Dismiss

	cmp ah, 1ch 	; Enter
	je @@Dismiss

	ret

	@@Dismiss:
	call GUI_HideCursor
	jmp ScrMainMenu_Start
ScrMainMenu_MsgBoxKeyPressHandler endp