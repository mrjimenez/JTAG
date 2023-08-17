#! /usr/bin/python3
# coding: utf-8

#
# example:
# $ ./xsvf -c disasm ../xsvf/XC2C64A/idcode_simpler.xsvf
#

import JTAGTAP
import XSVFDecoder


class XSVFDisassembler(XSVFDecoder.XSVFDecoder):
    """
    XSVF Disassembler
    """
    @staticmethod
    def add_arguments(p):
        """Adds the necessary arguments to the parser."""
        p.add_argument(
            '-n', '--no_bytes',
            action='store_true',
            # type=bool,
            help='Do not output bytes'
                 ' (default=%(default)s)')

    def __init__(self, args):
        XSVFDecoder.XSVFDecoder.__init__(self, args)
        self._args = args
        self._current_instruction = 0
        self._instruction_handlers = (
            self.disasm_xcomplete,
            self.disasm_xtdomask,
            self.disasm_xsir,
            self.disasm_xsdr,
            self.disasm_xruntest,
            self.disasm_xreserved_5,
            self.disasm_xreserved_6,
            self.disasm_xrepeat,
            self.disasm_xsdrsize,
            self.disasm_xsdrtdo,
            self.disasm_xsetsdrmasks,
            self.disasm_xsdrinc,
            self.disasm_xsdrb,
            self.disasm_xsdrc,
            self.disasm_xsdre,
            self.disasm_xsdrtdob,
            self.disasm_xsdrtdoc,
            self.disasm_xsdrtdoe,
            self.disasm_xstate,
            self.disasm_xendir,
            self.disasm_xenddr,
            self.disasm_xsir2,
            self.disasm_xcomment,
            self.disasm_xwait,
        )

    @property
    def current_instruction(self):
        return self._current_instruction

    @current_instruction.setter
    def current_instruction(self, value):
        self._current_instruction = value

    def format_first_part(self, s):
        """
        Breaks the instruction's bytes in lines of 8 bytes.

        :param s: string of bytes

        :return: list of 8 byte strings
        """
        l = []
        while s:
            l.append(s[:24])
            s = s[24:]
        return l

    def return_zeroeth(self, l):
        """
        Returns the zeroeth element of the list, right whitespace stripped,
        but avoid returning None if the result is empty.
        """
        if l:
            l_0 = l[0].rstrip()
        else:
            l_0 = ""
        return l_0

    def format_byte_list(self, l):
        return '    {:s}'.format(' '.join('{0:02X}'.format(x) for x in l))

    def pretty_disasm(self, pars=("",)):
        l1 = self.format_first_part(self.current_instruction_string)
        l2 = list(pars)
        l2[0] = '{:s}{:s}'.format(
            self.instruction_name(self.current_instruction),
            l2[0])
        while l1 or l2:
            l1_0 = self.return_zeroeth(l1)
            l2_0 = self.return_zeroeth(l2)
            if self._args.no_bytes:
                if l2_0:
                    print('{0:s}'.format(l2_0))
            else:
                if l2_0:
                    print('{0:<24}    {1:s}'.format(l1_0, l2_0))
                else:
                    print('{0:s}'.format(l1_0))
            l1 = l1[1:]
            l2 = l2[1:]

    def disasm_xcomplete(self):
        self.pretty_disasm()

    def disasm_xtdomask(self):
        p = (
            '',
            self.format_byte_list(self.tdo_mask)
        )
        self.pretty_disasm(p)

    def disasm_xsir(self):
        p = (
            ' {:d} {:s}'.format(
                self.sirsize_bits,
                self.format_byte_list(self.tdi).strip()),
        )
        self.pretty_disasm(p)

    def disasm_xsdr(self):
        p = (
            '',
            self.format_byte_list(self.tdi)
        )
        self.pretty_disasm(p)

    def disasm_xruntest(self):
        p = (
            ' {:d}'.format(self.runtest),
        )
        self.pretty_disasm(p)

    def disasm_xreserved_5(self):
        self.pretty_disasm()

    def disasm_xreserved_6(self):
        self.pretty_disasm()

    def disasm_xrepeat(self):
        p = (
            ' {:d}'.format(self.repeat),
        )
        self.pretty_disasm(p)

    def disasm_xsdrsize(self):
        p = (
            ' {0}'.format(self.sdrsize_bits),
        )
        self.pretty_disasm(p)

    def disasm_xsdrtdo(self):
        p = (
            '',
            self.format_byte_list(self.tdi) + ',',
            self.format_byte_list(self.tdo_expected)
        )
        self.pretty_disasm(p)

    def disasm_xsetsdrmasks(self):
        p = (
            '',
            self.format_byte_list(self.address_mask) + ',',
            self.format_byte_list(self.data_mask)
        )
        self.pretty_disasm(p)

    def disasm_xsdrinc(self):
        p = [
            '',
            self.format_byte_list(self.xsdrinc_start_address) + ',',
            '    {:d},'.format(self.xsdrinc_num_times)
        ]
        n = self.xsdrinc_num_times
        for l in self.xsdrinc_data_list:
            s = self.format_byte_list(l)
            n -= 1
            # Adds a comma, unless it's the last one
            if n:
                s += ','
            p.append(s)
        self.pretty_disasm(p)

    def disasm_xsdrb(self):
        p = (
            '',
            self.format_byte_list(self.tdi)
        )
        self.pretty_disasm(p)

    def disasm_xsdrc(self):
        p = (
            '',
            self.format_byte_list(self.tdi)
        )
        self.pretty_disasm(p)

    def disasm_xsdre(self):
        p = (
            '',
            self.format_byte_list(self.tdi)
        )
        self.pretty_disasm(p)

    def disasm_xsdrtdob(self):
        p = (
            '',
            self.format_byte_list(self.tdi) + ',',
            self.format_byte_list(self.tdo_expected)
        )
        self.pretty_disasm(p)

    def disasm_xsdrtdoc(self):
        p = (
            '',
            self.format_byte_list(self.tdi) + ',',
            self.format_byte_list(self.tdo_expected)
        )
        self.pretty_disasm(p)

    def disasm_xsdrtdoe(self):
        p = (
            '',
            self.format_byte_list(self.tdi) + ',',
            self.format_byte_list(self.tdo_expected)
        )
        self.pretty_disasm(p)

    def disasm_xstate(self):
        p = (
            ' {:s}'.format(JTAGTAP.JTAGTAP.state_name(self.next_state)),
        )
        self.pretty_disasm(p)

    def disasm_xendir(self):
        p = (
            ' {:s}'.format(JTAGTAP.JTAGTAP.state_name(self.endir_state)),
        )
        self.pretty_disasm(p)

    def disasm_xenddr(self):
        p = (
            ' {:s}'.format(JTAGTAP.JTAGTAP.state_name(self.enddr_state)),
        )
        self.pretty_disasm(p)

    def disasm_xsir2(self):
        p = (
            ' {:d}'.format(self.sirsize_bits),
            self.format_byte_list(self.tdi)
        )
        self.pretty_disasm(p)

    def disasm_xcomment(self):
        p = (
            ' "{:s}"'.format(self.comment),
        )
        self.pretty_disasm(p)

    def disasm_xwait(self):
        p = (
            ' {:s} {:s} {:d}'.format(
                JTAGTAP.JTAGTAP.state_name(self.wait_start_state),
                JTAGTAP.JTAGTAP.state_name(self.wait_end_state),
                self.wait_time_usecs),
        )
        self.pretty_disasm(p)

    #
    def instruction_handler(self, instruction):
        self.current_instruction = instruction
        self._instruction_handlers[instruction]()

    def disasm_all_files(self, fd_list):
        return self.decode_all_files(fd_list)
