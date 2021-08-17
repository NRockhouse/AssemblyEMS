; Stores the value of system timer for the next second, used by Update (MAIN.asm)
Update_NextSecond db 0

; A hook to set which function handles key presses now
GUI_KeyPressHandler dw 0000h

; Current session TP number (0ffh if it's staff)
Session db 3 dup(0)

; Used by both ScrEventList and ScrEventDetails
SelectedEvent db 0

; Used by ScrEventDetails as temporary buffer
Buffer db 700 dup(0)

; Used by ScrEventDetails description field for position counting
ScrEventDetails_DescCharCount db 10 dup(0)

; Size of one EventList item, placed here to prevent confusion as this is not being saved
EventList_Size equ EventList_1_Name - EventList_0_Name

; The size of the save file
SaveFile_Size equ 11051d

; =====================================================
;  !!! CONFIG FILE BACKUP RANGE BELOW !!!
;  1x EventList = 1105 bytes
;  Total		= 11051 bytes (11+ KB)
;
;  (Status)
;  0 = Closed
;  1 = Open
;  2 = Selected
; =====================================================

EventList_Count 			db 0

EventList_0_Name 			db 100 dup(0)
EventList_0_Desc 			db 700 dup(0)
EventList_0_Color			db 0
EventList_0_Limit			db 0
EventList_0_Price			db 0
EventList_0_Status			db 0
EventList_0_PtcpntsCount	db 0
EventList_0_Ptcpnts			db 300 dup(0)

EventList_1_Name 			db 100 dup(0)
EventList_1_Desc 			db 700 dup(0)
EventList_1_Color			db 0
EventList_1_Limit			db 0
EventList_1_Price			db 0
EventList_1_Status			db 0
EventList_1_PtcpntsCount	db 0
EventList_1_Ptcpnts			db 300 dup(0)

EventList_2_Name 			db 100 dup(0)
EventList_2_Desc 			db 700 dup(0)
EventList_2_Color			db 0
EventList_2_Limit			db 0
EventList_2_Price			db 0
EventList_2_Status			db 0
EventList_2_PtcpntsCount	db 0
EventList_2_Ptcpnts			db 300 dup(0)

EventList_3_Name 			db 100 dup(0)
EventList_3_Desc 			db 700 dup(0)
EventList_3_Color			db 0
EventList_3_Limit			db 0
EventList_3_Price			db 0
EventList_3_Status			db 0
EventList_3_PtcpntsCount	db 0
EventList_3_Ptcpnts			db 300 dup(0)

EventList_4_Name 			db 100 dup(0)
EventList_4_Desc 			db 700 dup(0)
EventList_4_Color			db 0
EventList_4_Limit			db 0
EventList_4_Price			db 0
EventList_4_Status			db 0
EventList_4_PtcpntsCount	db 0
EventList_4_Ptcpnts			db 300 dup(0)

EventList_5_Name 			db 100 dup(0)
EventList_5_Desc 			db 700 dup(0)
EventList_5_Color			db 0
EventList_5_Limit			db 0
EventList_5_Price			db 0
EventList_5_Status			db 0
EventList_5_PtcpntsCount	db 0
EventList_5_Ptcpnts			db 300 dup(0)

EventList_6_Name 			db 100 dup(0)
EventList_6_Desc 			db 700 dup(0)
EventList_6_Color			db 0
EventList_6_Limit			db 0
EventList_6_Price			db 0
EventList_6_Status			db 0
EventList_6_PtcpntsCount	db 0
EventList_6_Ptcpnts			db 300 dup(0)

EventList_7_Name 			db 100 dup(0)
EventList_7_Desc 			db 700 dup(0)
EventList_7_Color			db 0
EventList_7_Limit			db 0
EventList_7_Price			db 0
EventList_7_Status			db 0
EventList_7_PtcpntsCount	db 0
EventList_7_Ptcpnts			db 300 dup(0)

EventList_8_Name 			db 100 dup(0)
EventList_8_Desc 			db 700 dup(0)
EventList_8_Color			db 0
EventList_8_Limit			db 0
EventList_8_Price			db 0
EventList_8_Status			db 0
EventList_8_PtcpntsCount	db 0
EventList_8_Ptcpnts			db 300 dup(0)

EventList_9_Name 			db 100 dup(0)
EventList_9_Desc 			db 700 dup(0)
EventList_9_Color			db 0
EventList_9_Limit			db 0
EventList_9_Price			db 0
EventList_9_Status			db 0
EventList_9_PtcpntsCount	db 0
EventList_9_Ptcpnts			db 300 dup(0)