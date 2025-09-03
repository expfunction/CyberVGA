; cv_vga.asm Borland PowerPack 32-bit, TASM32-friendly VGA routines
; Build:  tasm32 /ml /m3 cv_vga.asm

.386
.model flat, C

public  vga_set_palette256
public  vga_blit_8
public  vsync_wait

.code
; void vga_set_palette256(unsigned char* pDac);
; Sets the 256-color VGA palette from a 768-byte RGB array at pDac.
vga_set_palette256 proc pDac:dword
    pushad
    mov     esi, [pDac]
    mov     dx, 03C8h
    xor     al, al
    out     dx, al
    inc     dx            ; 03C9h
    mov     ecx, 256*3
L: lodsb
    out     dx, al
    loop    L
    popad
    ret
vga_set_palette256 endp

; void vsync_wait(void);
; Polls VGA status (port 3DAh bit 3) wait not-in-retrace, then retrace start.
vsync_wait proc
    push edx
    mov  dx,03DAh
; wait for end of current retrace
nr: 
     in al,dx
     test al,08h
     jnz  nr
; wait for next retrace to begin
r:  
     in   al,dx
     test al,08h
     jz   r
    pop  edx
    ret
vsync_wait endp

; void vga_blit_8(const void* src, unsigned long nbytes);
vga_blit_8 proc src:dword, nbytes:dword
    pushad
    push    es

    mov     esi, [src]          ; source ptr
    mov     ecx, [nbytes]       ; bytes to copy (e.g., 64000)

    ; make an alias selector for RM seg A000h
    mov     bx, 0A000h
    mov     ax, 0002h
    int     31h
    jc      short .done         ; if it fails, exit quietly
    mov     dx, ax

    mov     ax, dx
    mov     es, ax              ; ES = VGA selector
    xor     edi, edi            ; ES:EDI = A000:0000

    ; copy ECX bytes (rep movsd/movsb split)
    mov     edx, ecx
    shr     ecx, 2
    rep     movsd
    mov     ecx, edx
    and     ecx, 3
    rep     movsb

    ; free selector
    mov     ax, 0001h
    mov     cx, dx
    int     31h

.done:
    pop     es
    popad
    ret
vga_blit_8 endp

end