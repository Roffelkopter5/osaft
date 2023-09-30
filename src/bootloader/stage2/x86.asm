bits 16

section _TEXT class=CODE

; ------------------------------------------------------------------
; Credit: nanobyte
;
; void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* remainderOut);
;
global _x86_div64_32
_x86_div64_32:

    ; make new call frame
    push bp             ; save old call frame
    mov bp, sp          ; initialize new call frame

    push bx

    ; divide upper 32 bits
    mov eax, [bp + 8]   ; eax <- upper 32 bits of dividend
    mov ecx, [bp + 12]  ; ecx <- divisor
    xor edx, edx
    div ecx             ; eax - quot, edx - remainder

    ; store upper 32 bits of quotient
    mov bx, [bp + 16]
    mov [bx + 4], eax

    ; divide lower 32 bits
    mov eax, [bp + 4]   ; eax <- lower 32 bits of dividend
                        ; edx <- old remainder
    div ecx

    ; store results
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret

;
; U4D
;
; Operation:      Unsigned 4 byte divide
; Inputs:         DX;AX   Dividend
;                 CX;BX   Divisor
; Outputs:        DX;AX   Quotient
;                 CX;BX   Remainder
; Volatile:       none
;
global __U4D
__U4D:
    shl edx, 16         ; dx to upper half of edx
    mov dx, ax          ; edx - dividend
    mov eax, edx        ; eax - dividend
    xor edx, edx

    shl ecx, 16         ; cx to upper half of ecx
    mov cx, bx          ; ecx - divisor

    div ecx             ; eax - quot, edx - remainder
    mov ebx, edx
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16

    ret


;
; U4M
; Operation:      integer four byte multiply
; Inputs:         DX;AX   integer M1
;                 CX;BX   integer M2
; Outputs:        DX;AX   product
; Volatile:       CX, BX destroyed
;
global __U4M
__U4M:
    shl edx, 16         ; dx to upper half of edx
    mov dx, ax          ; m1 in edx
    mov eax, edx        ; m1 in eax

    shl ecx, 16         ; cx to upper half of ecx
    mov cx, bx          ; m2 in ecx

    mul ecx             ; result in edx:eax (we only need eax)
    mov edx, eax        ; move upper half to dx
    shr edx, 16

    ret

; ------------------------------------------------------------------


;
; void _cdecl x86_Video_WriteChar(char c, uint8_t page);
;
global _x86_Video_WriteChar
_x86_Video_WriteChar:
    push bp
    mov bp, sp

    push bx

    mov ah, 0x0E
    mov al, [bp + 4]
    mov bh, [bp + 6]
    int 10h

    pop bx

    mov sp, bp
    pop bp
    ret

;
; bool _cdecl x86_Disk_Reset(uint8_t drive);
;
global _x86_Disk_Reset
_x86_Disk_Reset:
    push bp
    mov bp, sp
    
    mov ah, 0
    mov dl, [bp + 4]
    stc
    int 13h

    mov ax, 1
    sbb ax, 0 ; carry = 1 (fail), carry = 0 (success)
    
    mov sp, bp
    pop bp
    ret

;
; bool _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t head, uint16_t sector, uint8_t count, uint8_t far *dataOut);
; 
global _x86_Disk_Read
_x86_Disk_Read:
    push bp
    mov bp, sp

    push bx
    push es

    mov dl, [bp + 4]

    mov ah, 2
    mov ch, [bp + 6]
    mov cl, [bp + 7]
    shl cl, 6

    mov dh, [bp + 8]

    mov al, [bp + 10]
    and al, 0x3f
    or cl, al

    mov al, [bp + 12]

    mov bx, [bp + 16]
    mov es, bx
    mov bx, [bp + 14]

    stc
    int 13h

    mov ax, 1
    sbb ax, 0 ; carry = 1 (fail), carry = 0 (success)

    pop es
    pop bx

    mov sp, bp
    pop bp
    ret

;
; bool _cdecl x86_Disk_GetParams(uint8_t drive, uint8_t *drveTypeOut, uint16_t *cylindersOut, uint16_t *headsOut, uint16_t *sectorsOut);
;
global _x86_Disk_GetParams
_x86_Disk_GetParams:
    push bp
    mov bp, sp

    push bx
    push si
    push es
    push di

    mov di, 0
    mov es, di

    mov ah, 8
    mov dl, [bp + 4]

    stc
    int 13h

    mov ax, 1
    sbb ax, 0 ; carry = 1 (fail), carry = 0 (success)

    mov si, [bp + 6]
    mov [si], bl

    mov bl, ch
    mov bh, cl
    shr bh, 6
    mov si, [bp + 8]
    mov [si], bx

    xor ch, ch
    and cl, 0x3f
    mov si, [bp + 12]
    mov [si], cx

    mov cl, dh
    mov si, [bp + 10]
    mov [si], cx

    pop di
    pop es
    pop si
    pop bx

    mov sp, bp
    pop bp
    ret