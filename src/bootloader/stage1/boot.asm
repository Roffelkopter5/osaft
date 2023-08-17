org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

jmp short start
nop

; FAT12 header
bdb_oem_identifier: db 'MSWIN4.1'
bdb_bytes_per_sector: dw 512
bdb_sectors_per_cluster: db 1
bdb_reserved_sectors: dw 1
bdb_fat_count: db 2
bdb_dir_entries_count: dw 0E0h
bdb_total_sectors: dw 2880
bdb_media_descriptor_type: db 0F0h
bdb_sectors_per_fat: dw 9
bdb_sectors_per_track: dw 18
bdb_heads: dw 2
bdb_hidden_sectors: dd 0
bdb_large_sector_count: dd 0

; extended boot record
ebr_drive_number: db 0
db 0
ebr_signature: db 29h
ebr_volume_id: db 04h, 20h, 23h, 08h
ebr_volume_label: db 'O-Saft OS  '
ebr_system_id: db 'FAT12   '


start:
    mov ax, 0
    mov ds, ax
    mov es, ax

    push es
    push word .after
    retf

.after:
    mov ss, ax
    mov sp, 0x7C00

    mov si, str_hello_msg
    call puts

    mov [ebr_drive_number], dl ; Bios puts drive number in dl
    mov ah, 08h
    push es
    int 13h
    jc floppy_error
    pop es

    and cl, 0x3F
    xor ch, ch
    mov [bdb_sectors_per_track], cx
    inc dh 
    mov [bdb_heads], dh

.read_root_dir:
    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bl
    add ax, [bdb_reserved_sectors]

    push ax
    mov ax, [bdb_dir_entries_count]
    shl ax, 5
    xor dx, dx
    div word [bdb_bytes_per_sector]
    test dx, dx
    jc .read_root_dir_after
    inc ax
 
.read_root_dir_after:
    mov cl, al
    mov dl, [ebr_drive_number]
    mov bx, buffer
    pop ax
    call disk_read

    xor bx, bx
    mov di, buffer

.search_kernel:
    mov si, kernel_file_name
    mov cx, 11
    push di
    repe cmpsb 
    pop di
    jz .found_kernel
    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_kernel
    call kernel_error

.found_kernel:
    mov ax, [di + 26] ; first cluster
    push ax
    mov ax, [bdb_reserved_sectors]
    mov bx, buffer
    mov cl, [bdb_sectors_per_fat]
    mov dl, [ebr_drive_number]
    call disk_read

    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx
    mov bx, KERNEL_LOAD_OFFSET
    
    pop ax

.load_kernel_loop:
    push ax
    add ax, 31
    
    mov cl, 1
    mov dl, [ebr_drive_number]
    call disk_read

    add bx, [bdb_bytes_per_sector]
    pop ax
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz .even

.odd:
    shr ax, 4
    jmp .load_after

.even:
    and ax, 0x0FFF

.load_after:
    cmp ax, 0x0FF8
    jl .load_kernel_loop

    mov dl, [ebr_drive_number]
    mov ax, KERNEL_LOAD_SEGMENT
    mov ds, ax
    mov es, ax

    jmp KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET

    jmp halt

floppy_error:
    mov si, str_floppy_error
    call puts
    jmp wait_and_reboot

kernel_error:
    mov si, str_kernel_error
    call puts
    jmp wait_and_reboot

wait_and_reboot:
    mov ah, 0
    int 16h
    jmp 0FFFFh:0

halt:
    cli
    hlt

; Prints string
; Params:
;   - ds:si: Adress of string
puts:
    push ax
    push si

.loop:
    lodsb ; load char at ds:si into al and increment si
    or al, al
    jz .done

    mov ah, 0x0E
    int 0x10

    jmp .loop

.done:
    pop si
    pop ax
    ret

; disk op

; Converts LBA to CHS adress
; Params:
;   - ax: LBA adress
; Returns:
;   - cx [0-5]: sector number
;   - cx [6-15]: cylinder
;   - dh: head
lba_to_chs:
    push ax
    push dx

    xor dx, dx
    div word [bdb_sectors_per_track] ; ax = LBA / sectors_per_track
                                     ; dx = LBA % sectors_per_track
    inc dx                           ; dx = LBA % sectors_per_track + 1 = sector
    mov cx, dx                       ; cx = sector
    xor dx, dx
    div word [bdb_heads]             ; ax = LBA / sectors_per_track / heads = cylinder
                                     ; dx = LBA / sectors_per_track % heads = head
    mov dh, dl
    mov ch, al
    shl ah, 6
    or cl, ah

    pop ax
    mov dl, al
    pop ax
    ret

; read sectors from disk
; Params:
;   - ax: LBA adress
;   - cl: number of sectors to read
;   - dl: drive number
;   - es:bx: memory addres to store data
disk_read:
    push ax
    push bx
    push cx
    push dx
    push di

    push cx
    call lba_to_chs
    pop ax

    mov ah, 02h
    mov di, 3

.retry:
    pusha
    stc 
    int 13h
    jnc .done
    popa
    call disk_reset
    dec di
    test di, di
    jnz .retry

.fail:
    jmp floppy_error

.done:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; resets disk
; Params:
;   - dl: drive number
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc floppy_error
    popa
    ret

; data
str_hello_msg: db 'Loading O-Saft...', ENDL, 0
str_floppy_error: db 'Error: Unable to read from disk', ENDL, 0
str_kernel_error: db 'Error: Unable to load kernel', ENDL, 0

kernel_file_name: db 'KERNEL  BIN'

KERNEL_LOAD_SEGMENT equ 0x2000
KERNEL_LOAD_OFFSET equ 0

times 510-($-$$) db 0
dw 0AA55h

buffer: