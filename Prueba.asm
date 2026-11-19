;Prueba del archivo
BITS 32

SECTION .text
GLOBAL _start

_start:
    MOV EAX, 10             ; numero decimal simple
    MOV EBX, 0x1F           ; numero hexadecimal estilo C
    MOV ECX, 1Ah            ; numero hexadecimal estilo MASM
    MOV EDX, 0FFh           ; hexadecimal que empieza con digito

    ADD EAX, EBX            ; registro + registro
    SUB ECX, 4              ; registro - numero
    ADD EDX, [EAX + 4]      ; acceso a memoria con desplazamiento
    SUB ESI, [EBX]          ; acceso a memoria simple

    MOV EDI, [ESP + EBP]    ; dos registros dentro de corchetes
    MOV ESP, [EBP - 8]      ; desplazamiento negativo

    CMP EAX, 100            ; comparacion con decimal
    CMP EBX, 0x64           ; comparacion con hex

l1:
    JMP l1          ; salto a etiqueta local

l2:
    MOV EAX, [EBX + ECX*4]  ; direccionamiento indexado
    ADD EAX, 1

fin:
    CMP EAX, EAX            ; instruccion final
    JMP fin                 ; bucle infinito
