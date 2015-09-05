; XSVF Assembly language example file
;
; All XSVF instructions are used here

XSDRSIZE 48 ; XSDRSIZE must be set for the decoder/disassembler to work
            ; properly.
XTDOMASK    ; Comment test
        00 00 00 00 00 00
XSIR 8 00   ; Comment test
XSDR        ; Comment test
        00 00 00 00 00 00 ; Comment test
XRUNTEST 70
XRESERVED_5
XRESERVED_6
XREPEAT 0x20
XSDRTDO
        FF FF FF FF FF FF ; Comment test
        ,                 ; Comment test
        00 00 00 00 00 00 ; Comment test
XSETSDRMASKS
        00 00 00 00 00 00,
        FF FF FF FF FF FF
XSDRINC                     ; Comment test
        01010101            ; Comment test in the middle of byte_squence
        FF                  ; Comment test in the middle of byte_squence
        10101010 FF 01010101 FF ; Comment test
        ,                   ; Comment test after comma
        5,                  ; Comment test
        00 00 00 00 00 00,  ; Comment test
        FF FF FF FF FF FF,  ; Comment test
        00 00 00 00 00 00,  ; Comment test
        FF FF FF FF FF FF,  ; Comment test
        00 00 00 00 00 00   ; Comment test
XSDRB
        FF FF FF FF FF FF
XSDRC
        FF FF FF FF FF FF
XSDRE
        FF FF FF FF FF FF
XSDRTDOB
        FF FF FF FF FF FF,
        00 00 00 00 00 00
XSDRTDOC
        FF FF FF FF FF FF,
        00 00 00 00 00 00
XSDRTDOE
        FF FF FF FF FF FF,
        00 00 00 00 00 00
XSTATE ; Comment test
       TEST_LOGIC_RESET
XSTATE RUN_TEST_IDLE
XSTATE SELECT_DR_SCAN
XSTATE CAPTURE_DR
XSTATE SHIFT_DR
XSTATE EXIT1_DR
XSTATE PAUSE_DR
XSTATE EXIT2_DR
XSTATE UPDATE_DR
XSTATE SELECT_IR_SCAN
XSTATE CAPTURE_IR
XSTATE SHIFT_IR
XSTATE EXIT1_IR
XSTATE PAUSE_IR
XSTATE EXIT2_IR
XSTATE UPDATE_IR
XENDIR RUN_TEST_IDLE
XENDIR PAUSE_IR
XENDDR RUN_TEST_IDLE
XENDDR PAUSE_DR
XSIR2 48
        00 00 00 00 00 00
XCOMMENT ; Comment test
    "adsf f sda asdf sdaf asdf sadf
    qwer qwer qwer qwer
    zxcv zvcx zxcv c"   ; Comment test
; Comment test
XWAIT                   ; Comment test
    TEST_LOGIC_RESET    ; Comment test
    RUN_TEST_IDLE       ; Comment test
    100                 ; Comment test
XCOMPLETE
