; =============================================
;	Misc_PushAllRegs
;	Input:
;		ax, bx, cx, dx
;	Output:
;		N/A
;	Notes:
;		Refer Excel sheet "Misc_P#AllRegs Diagram".
; =============================================
Misc_PushAllRegs proc
	push bx 	; push original bx value onto the stack first
	add sp, 2d	; move stack pointer back down (imaginary pop)
	pop bx 		; use bx to store the return address

	push ax
	sub sp, 2d

	push cx
	push dx

	push bx
	
	add sp, 6d	; retrieve otiginal bx from stack
	pop bx
	push bx
	sub sp, 6d
	ret
Misc_PushAllRegs endp

; =============================================
;	Misc_PopAllRegs
;	Input:
;		N/A
;	Output:
;		ax, bx, cx, dx
;	Notes:
;		Refer Excel sheet "Misc_P#AllRegs Diagram".
; =============================================
Misc_PopAllRegs proc
	pop bx

	pop dx
	pop cx

	add sp, 2d
	pop ax

	push bx
	sub sp, 2d
	pop bx
	ret
Misc_PopAllRegs endp

; =============================================
;	Misc_GetEventListDisplacement
;	Input:
;		al - EventList item index (start from 0)
;	Output:
;		bx - Displacement in memory / data segment
; =============================================
Misc_GetEventListDisplacement proc
	push ax 		; we can't use Misc_P*AllRegs because it would override the bx output
	push dx

	mov ah, 00h 			; make ax = al only
	mov bx, EventList_Size 	; calc the base address (the address of first appearing EventList's name)
	mul bx
	mov bx, ax 		; we have to use bx because Intel doesn't allow addition / addressing
					; mode using other general-purpose registers
			; refer: https://www.ic.unicamp.br/~celio/mc404s2-03/addr_modes/intel_addr.html (1.2.5)

	pop dx
	pop ax

	ret
Misc_GetEventListDisplacement endp

; =============================================
;	Misc_ClearMemory
;	Input:
;		cx - Length
;		si - Source index
;	Output:
;		N/A
; =============================================
Misc_ClearMemory proc
	mov byte ptr [si], 00h
	inc si
	loop Misc_ClearMemory
	ret
Misc_ClearMemory endp

; =============================================
;	Misc_SaveData
;	Input:
;		N/A
;	Output:
;		N/A
; =============================================
Misc_SaveData proc
	mov ah, 3ch
	mov cx, 0 		; create normal file - no read-only attributes etc.
	lea dx, Str_SaveFileName
	int 21h 		; if this fails, we're dying silently - maybe add an error handler here

	mov bx, ax 		; put the returned file handle to bx

	mov ah, 40h
	mov cx, SaveFile_Size
	lea dx, EventList_Count 	; the starting point
	int 21h

	mov ah, 3eh 	; close file handle
	int 21h
	ret
Misc_SaveData endp

; =============================================
;	Misc_ReadData
;	Input:
;		N/A
;	Output:
;		N/A
; =============================================
Misc_ReadData proc
	mov ah, 3dh
	mov al, 0 		; access mode: read
	lea dx, Str_SaveFileName
	int 21h

	jc @@Done 		; if error, maybe file doesn't exist, just die silently
	mov bx, ax 		; put the returned file handle to bx

	mov ah, 3fh
	mov cx, SaveFile_Size
	lea dx, EventList_Count
	int 21h

	mov ah, 3eh 	; close file handle
	int 21h

	@@Done:
	ret
Misc_ReadData endp