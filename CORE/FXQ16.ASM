; Q16.16 helpers for Borland PowerPack (32-bit)
.386
.model flat, C
.code

; i32 fx_mul_q16(i32 a, i32 b);  -> (a*b)>>16 (signed)
public fx_mul_q16
fx_mul_q16 proc a:dword, b:dword
    mov     eax, [a]
    imul    dword ptr [b]      ; EDX:EAX = EAX * [b]
    shrd    eax, edx, 16       ; logical right of 64-bit by 16 into EAX
    sar     edx, 16            ; arithmetic to keep sign for completeness
    ret
fx_mul_q16 endp

; i32 fx_div_q16(i32 num_q16, i32 den_q16); -> ((num<<16)/den) (signed)
public fx_div_q16
fx_div_q16 proc num:dword, den:dword
    mov     eax, [num]         ; EDX:EAX = sign-extended num
    cdq
    shld    edx, eax, 16       ; build 64-bit (num << 16)
    shl     eax, 16
    idiv    dword ptr [den]    ; EAX = ((num<<16)/den)
    ret
fx_div_q16 endp

end
