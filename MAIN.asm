.model small
.stack 100h
.data
	locals @@			; enable local labels (https://stackoverflow.com/a/43823619)

	include VARS.inc
	include STRINGS.inc
	
.code

; ========== APIs ==========
include API/MISC.inc
include API/GUI.inc

; ========== Screens ==========
include SCREENS/MAINMENU.inc
include SCREENS/EVENTS.inc
include SCREENS/DETAILS.inc
include SCREENS/PTCPLIST.inc

Start proc
	mov ax, @data
	mov ds, ax
	mov es, ax

	mov ah, 0 		; the Update_NextSecond incremental function copied from Update proc,
	int 1ah 		; we need to do it once first so the OnSecond can fire normally

	mov dh, 0
	add dx, 18d
	mov Update_NextSecond, dl

	call GUI_Start
	call ScrMainMenu_Start
Start endp

Update proc
	; ===== KeyPress check =====
	; return values: https://www.fountainware.com/EXPL/bios_key_codes.htm
	; (high = ah, low = al)
	mov ah, 01h		; check if any key pressed in keyboard buffer and set ZF
	int 16h
	jz @@OnSecond

	mov ah, 00h 	; clear the keyboard buffer
	int 16h 		; https://stackoverflow.com/a/61486868

	call [GUI_KeyPressHandler]
	; ==========================

	@@OnSecond:
	; ===== OnSecond check =====
	mov ah, 0		; system timer that increments 18.2 times every second
	int 1ah			; https://stackoverflow.com/a/9973442
					; timer value = cx:dx (will only use dl because otherwise integer overflow)

	cmp dl, [Update_NextSecond]
	jne Update

	mov dh, 0
	add dx, 18d
	mov Update_NextSecond, dl

	call GUI_UpdateClock
	; ==========================

	jmp Update
Update endp

Quit proc
	mov ah, 06h 	; clear screen
	mov al, 00h
	mov bh, 07h		; black bg, white text
	xor cx, cx
	mov dx, 184fh
	int 10h

	mov ah, 02h		; move cursor back to 0,0
	mov bh, 0
	xor dx, dx
	int 10h

	mov ah, 01h 	; make cursor back to two-bottom-lined default
	mov cx, 0607h
	int 10h

	mov ah, 4ch
	int 21h			; Quit to DOS
Quit endp

end Start 			; Set the entry point to "Start" procedure
					; https://docs.microsoft.com/en-us/cpp/assembler/masm/end-masm