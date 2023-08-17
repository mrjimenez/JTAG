#! /usr/bin/python3
# coding: utf-8

# ./XSVFAssembler.py > test.xsvf
# ./xsvf -c disasm -n test.xsvf > test.xsvf.s
# ./xsvf -c asm test.xsvf.s > test2.xsvf

import pyparsing as pp
import sys

__all__ = ['parse_xsvf_file']

# Comment
asm_comment = pp.Suppress(pp.Group(';' + pp.restOfLine))
optional_asm_comment = pp.Optional(asm_comment)
xcomment_argument = pp.QuotedString(
    quoteChar='"', escQuote='""', multiline=True)

# Hexadecimal integers
zero_ex = pp.Literal('0x') | pp.Literal('0X')
hex_body = pp.Word(pp.hexnums)
hex_number = pp.Combine(zero_ex + hex_body)

hex_byte = pp.Word(pp.hexnums, exact=2)
bin_byte = pp.Word('01', exact=8)
# The order is important here. We must try to match a bin_byte first, otherwise
# '01010101' becomes '01' '01' '01' '01'.
byte_number = bin_byte | hex_byte

# Byte sequences and byte sequence lists
comma = pp.Suppress(pp.Literal(','))
byte_sequence = pp.Group(pp.OneOrMore(byte_number + optional_asm_comment))
byte_sequence_list = \
    byte_sequence + \
    pp.ZeroOrMore(comma + optional_asm_comment +
                  byte_sequence)
# The order is important here. We must try to match '0x1ABCD' to hex first
# otherwise it will become '0' 'x'.
integer = hex_number | pp.Word(pp.nums)

# JTAG TAP States
test_logic_reset = pp.Keyword('TEST_LOGIC_RESET')
run_test_idle = pp.Keyword('RUN_TEST_IDLE')
select_dr_scan = pp.Keyword('SELECT_DR_SCAN')
capture_dr = pp.Keyword('CAPTURE_DR')
shift_dr = pp.Keyword('SHIFT_DR')
exit1_dr = pp.Keyword('EXIT1_DR')
pause_dr = pp.Keyword('PAUSE_DR')
exit2_dr = pp.Keyword('EXIT2_DR')
update_dr = pp.Keyword('UPDATE_DR')
select_ir_scan = pp.Keyword('SELECT_IR_SCAN')
capture_ir = pp.Keyword('CAPTURE_IR')
shift_ir = pp.Keyword('SHIFT_IR')
exit1_ir = pp.Keyword('EXIT1_IR')
pause_ir = pp.Keyword('PAUSE_IR')
exit2_ir = pp.Keyword('EXIT2_IR')
update_ir = pp.Keyword('UPDATE_IR')

state = (
    test_logic_reset |
    run_test_idle |
    select_dr_scan |
    capture_dr |
    shift_dr |
    exit1_dr |
    pause_dr |
    exit2_dr |
    update_dr |
    select_ir_scan |
    capture_ir |
    shift_ir |
    exit1_ir |
    pause_ir |
    exit2_ir |
    update_ir
)

# Instructions
xcomplete = pp.Group(pp.Keyword('XCOMPLETE'))
xtdomask = pp.Group(pp.Keyword('XTDOMASK') + optional_asm_comment +
                    byte_sequence)
xsir = pp.Group(pp.Keyword('XSIR') + optional_asm_comment +
                integer + optional_asm_comment +
                byte_sequence)
xsdr = pp.Group(pp.Keyword('XSDR') + optional_asm_comment +
                byte_sequence)
xruntest = pp.Group(pp.Keyword('XRUNTEST') + optional_asm_comment +
                    integer)
xreserved_5 = pp.Group(pp.Keyword('XRESERVED_5'))
xreserved_6 = pp.Group(pp.Keyword('XRESERVED_6'))
xrepeat = pp.Group(pp.Keyword('XREPEAT') + optional_asm_comment +
                   integer)
xsdrsize = pp.Group(pp.Keyword('XSDRSIZE') + optional_asm_comment +
                    integer)
xsdrtdo = pp.Group(pp.Keyword('XSDRTDO') +
                   byte_sequence + optional_asm_comment +
                   comma + optional_asm_comment +
                   byte_sequence)
xsetsdrmasks = pp.Group(pp.Keyword('XSETSDRMASKS') + optional_asm_comment +
                        byte_sequence + optional_asm_comment +
                        comma + optional_asm_comment +
                        byte_sequence)
xsdrinc = pp.Group(pp.Keyword('XSDRINC') + optional_asm_comment +
                   byte_sequence + optional_asm_comment +
                   comma + optional_asm_comment +
                   integer + optional_asm_comment +
                   comma + optional_asm_comment +
                   byte_sequence_list)
xsdrb = pp.Group(pp.Keyword('XSDRB') + optional_asm_comment +
                 byte_sequence)
xsdrc = pp.Group(pp.Keyword('XSDRC') + optional_asm_comment +
                 byte_sequence)
xsdre = pp.Group(pp.Keyword('XSDRE') + optional_asm_comment +
                 byte_sequence)
xsdrtdob = pp.Group(pp.Keyword('XSDRTDOB') + optional_asm_comment +
                    byte_sequence + optional_asm_comment +
                    comma + optional_asm_comment +
                    byte_sequence)
xsdrtdoc = pp.Group(pp.Keyword('XSDRTDOC') + optional_asm_comment +
                    byte_sequence + optional_asm_comment +
                    comma + optional_asm_comment +
                    byte_sequence)
xsdrtdoe = pp.Group(pp.Keyword('XSDRTDOE') + optional_asm_comment +
                    byte_sequence + optional_asm_comment +
                    comma + optional_asm_comment +
                    byte_sequence)
xstate = pp.Group(pp.Keyword('XSTATE') + optional_asm_comment +
                  state)
xendir = pp.Group(pp.Keyword('XENDIR') + optional_asm_comment +
                  (run_test_idle | pause_ir))
xenddr = pp.Group(pp.Keyword('XENDDR') + optional_asm_comment +
                  (run_test_idle | pause_dr))
xsir2 = pp.Group(pp.Keyword('XSIR2') + optional_asm_comment +
                 integer + optional_asm_comment +
                 byte_sequence)
xcomment = pp.Group(pp.Keyword('XCOMMENT') + optional_asm_comment +
                    xcomment_argument)
xwait = pp.Group(pp.Keyword('XWAIT') + optional_asm_comment +
                 state + optional_asm_comment +
                 state + optional_asm_comment +
                 integer)

instruction = (
    xcomplete |
    xtdomask |
    xsir |
    xsdr |
    xruntest |
    xreserved_5 |
    xreserved_6 |
    xrepeat |
    xsdrsize |
    xsdrtdo |
    xsetsdrmasks |
    xsdrinc |
    xsdrb |
    xsdrc |
    xsdre |
    xsdrtdob |
    xsdrtdoc |
    xsdrtdoe |
    xstate |
    xendir |
    xenddr |
    xsir2 |
    xcomment |
    xwait
)

# Line
line = asm_comment | (instruction + optional_asm_comment)

# File
xsvf_parser = pp.OneOrMore(line)


def parse_xsvf_file(f):
    return xsvf_parser.parseFile(f, parseAll=True)


xsvf_example = """
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
XCOMPLETE"""


def main():
    print(xsvf_parser.parseString(xsvf_example, parseAll=True))
    sys.exit(0)


if __name__ == '__main__':
    main()
