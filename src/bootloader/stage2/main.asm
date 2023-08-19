bits 16

section _ENTRY class=CODE

extern _cstart_
global entry

entry:
    cli
    ; setup stack
    mov ax, ds
    mov ss, ax
    mov sp, 0
    mov bp, sp
    sti

    xor dh, dh
    push dx

    ; mov ax, 0x0600 ; AH = 06 (scroll up function), AL = 0 (attribute to fill with)
    ; mov bh, 0x07   ; Background color (here, light gray)
    ; mov cx, 0      ; Upper left corner (row 0, column 0)
    ; mov dx, 0x184F ; Lower right corner (row 24, column 79)
    ; int 0x10       ; Call BIOS video interrupt to scroll window up and clear the screen

    ; mov ah, 0x02
    ; mov bh, 0
    ; mov dx, 0
    ; int 0x10 

    call _cstart_

    cli
    hlt