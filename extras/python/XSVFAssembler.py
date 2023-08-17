# !/usr/bin/python3
# coding: utf-8

#
# example:
# $ ./XSVFAssembler.py > test.xsvf
# $ ./xsvf -c disasm -n test.xsvf > test.xsvf.s
# $ ./xsvf -c asm ../xsvf/XC2C64A/idcode_simpler.xsvf.s
# $

import XSVFParser
import sys


class XSVFAssembler(object):
    """
    Assembler
    """
    OPCODE = 0
    HANDLER = 1

    def __init__(self, args):
        self._args = args
        self._instruction_info = {
            'XCOMPLETE': (0, self.asm_xcomplete),
            'XTDOMASK': (1, self.asm_xtdomask),
            'XSIR': (2, self.asm_xsir),
            'XSDR': (3, self.asm_xsdr),
            'XRUNTEST': (4, self.asm_xruntest),
            'XRESERVED_5': (5, self.asm_xreserved_5),
            'XRESERVED_6': (6, self.asm_xreserved_6),
            'XREPEAT': (7, self.asm_xrepeat),
            'XSDRSIZE': (8, self.asm_xsdrsize),
            'XSDRTDO': (9, self.asm_xsdrtdo),
            'XSETSDRMASKS': (10, self.asm_xsetsdrmasks),
            'XSDRINC': (11, self.asm_xsdrinc),
            'XSDRB': (12, self.asm_xsdrb),
            'XSDRC': (13, self.asm_xsdrc),
            'XSDRE': (14, self.asm_xsdre),
            'XSDRTDOB': (15, self.asm_xsdrtdob),
            'XSDRTDOC': (16, self.asm_xsdrtdoc),
            'XSDRTDOE': (17, self.asm_xsdrtdoe),
            'XSTATE': (18, self.asm_xstate),
            'XENDIR': (19, self.asm_xendir),
            'XENDDR': (20, self.asm_xenddr),
            'XSIR2': (21, self.asm_xsir2),
            'XCOMMENT': (22, self.asm_xcomment),
            'XWAIT': (23, self.asm_xwait)
        }
        self._state_info = {
            'TEST_LOGIC_RESET': 0,
            'RUN_TEST_IDLE': 1,
            'SELECT_DR_SCAN': 2,
            'CAPTURE_DR': 3,
            'SHIFT_DR': 4,
            'EXIT1_DR': 5,
            'PAUSE_DR': 6,
            'EXIT2_DR': 7,
            'UPDATE_DR': 8,
            'SELECT_IR_SCAN': 9,
            'CAPTURE_IR': 10,
            'SHIFT_IR': 11,
            'EXIT1_IR': 12,
            'PAUSE_IR': 13,
            'EXIT2_IR': 14,
            'UPDATE_IR': 15
        }
        self._arguments = []

    def asm_byte_sequence(self, l):
        for c in l:
            if len(c) == 2:
                b = int(c, base=16)
            else:  # len(c) == 8:
                b = int(c, base=2)
            sys.stdout.write(chr(b))

    def asm_integer(self, s, n):
        # Deal with '0x' prefixed integers
        if len(s) > 2 and s[0] == '0' and (s[1] == 'x' or s[1] == 'X'):
            i = int(s[2:], base=16)
        else:
            i = int(s)
        ret = i
        # Build a list of bytes
        l = []
        n1 = n
        while n1:
            j = i % 256
            l.append(j)
            i >>= 8
            n1 -= 1
        # Output the list reversed (Big Endian)
        n1 = n
        while n1:
            j = l.pop()
            sys.stdout.write(chr(j))
            n1 -= 1
        return ret

    def asm_state(self, state):
        i = self._state_info[state]
        sys.stdout.write(chr(i))

    def asm_xcomplete(self):
        pass

    def asm_xtdomask(self):
        self.asm_byte_sequence(self._arguments[0])

    def asm_xsir(self):
        self.asm_integer(self._arguments[0], 1)
        self.asm_byte_sequence(self._arguments[1])

    def asm_xsdr(self):
        self.asm_byte_sequence(self._arguments[0])

    def asm_xruntest(self):
        self.asm_integer(self._arguments[0], 4)

    def asm_xreserved_5(self):
        pass

    def asm_xreserved_6(self):
        pass

    def asm_xrepeat(self):
        self.asm_integer(self._arguments[0], 1)

    def asm_xsdrsize(self):
        self.asm_integer(self._arguments[0], 4)

    def asm_xsdrtdo(self):
        self.asm_byte_sequence(self._arguments[0])
        self.asm_byte_sequence(self._arguments[1])

    def asm_xsetsdrmasks(self):
        self.asm_byte_sequence(self._arguments[0])
        self.asm_byte_sequence(self._arguments[1])

    def asm_xsdrinc(self):
        self.asm_byte_sequence(self._arguments[0])
        n = self.asm_integer(self._arguments[1], 1)
        j = n
        while j:
            i = n - j
            self.asm_byte_sequence(self._arguments[i+2])
            j -= 1

    def asm_xsdrb(self):
        self.asm_byte_sequence(self._arguments[0])

    def asm_xsdrc(self):
        self.asm_byte_sequence(self._arguments[0])

    def asm_xsdre(self):
        self.asm_byte_sequence(self._arguments[0])

    def asm_xsdrtdob(self):
        self.asm_byte_sequence(self._arguments[0])
        self.asm_byte_sequence(self._arguments[1])

    def asm_xsdrtdoc(self):
        self.asm_byte_sequence(self._arguments[0])
        self.asm_byte_sequence(self._arguments[1])

    def asm_xsdrtdoe(self):
        self.asm_byte_sequence(self._arguments[0])
        self.asm_byte_sequence(self._arguments[1])

    def asm_xstate(self):
        state = str(self._state_info[self._arguments[0]])
        self.asm_integer(state, 1)

    def asm_xendir(self):
        state = self._arguments[0]
        self.asm_integer('0' if state == 'RUN_TEST_IDLE' else '1', 1)

    def asm_xenddr(self):
        state = self._arguments[0]
        self.asm_integer('0' if state == 'RUN_TEST_IDLE' else '1', 1)

    def asm_xsir2(self):
        self.asm_integer(self._arguments[0], 2)
        self.asm_byte_sequence(self._arguments[1])

    def asm_xcomment(self):
        sys.stdout.write(self._arguments[0])
        sys.stdout.write(chr(0))

    def asm_xwait(self):
        self.asm_state(self._arguments[0])
        self.asm_state(self._arguments[1])
        self.asm_integer(self._arguments[2], 4)

    def assemble_tree(self, tree):
        for l in tree:
            instruction = l[0]
            info = self._instruction_info[instruction]
            opcode = info[self.OPCODE]
            asm_funcion = info[self.HANDLER]
            self._arguments = l[1:]
            sys.stdout.write(chr(opcode))
            asm_funcion()
        return True

    def assemble_one_file(self, fd):
        tree = XSVFParser.xsvf_parser.parseFile(fd, parseAll=True)
        return self.assemble_tree(tree)

    def assemble_all_files(self, fd_list):
        ok = True
        for fd in fd_list:
            with fd:
                ok = self.assemble_one_file(fd)
                if not ok:
                    break
        return ok


def main():
    a = XSVFAssembler(None)
    tree = XSVFParser.xsvf_parser.parseString(
        XSVFParser.xsvf_example, parseAll=True)
    a.assemble_tree(tree)
    sys.exit(0)

if __name__ == '__main__':
    main()
