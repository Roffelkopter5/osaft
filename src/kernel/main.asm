org 0x0
bits 16


%define ENDL 0x0D, 0x0A


start:
    ; print hello world message
    mov si, msg_hello
    call puts

.halt:
    cli
    hlt

;
; Prints a string to the screen
; Params:
;   - ds:si points to string
;
puts:
    ; save registers we will modify
    push si
    push ax
    push bx

.loop:
    lodsb               ; loads next character in al
    or al, al           ; verify if next character is null?
    jz .done

    mov ah, 0x0E        ; set bios interrupt code
    mov bh, 0           ; set page number to 0
    int 0x10

    jmp .loop

.done:
    pop bx
    pop ax
    pop si    
    ret


msg_hello: db '   ____         _____        __ _   ', ENDL
db '  / __ \       / ____|      / _| |  ', ENDL
db ' | |  | |_____| (___   __ _| |_| |_ ', ENDL
db ' | |  | |______\___ \ / _` |  _| __|', ENDL
db ' | |__| |      ____) | (_| | | | |_ ', ENDL
db '  \____/      |_____/ \__,_|_|  \__|', ENDL
db '                                    ', ENDL, 0
                                    

