#! /usr/bin/python3
# coding: utf-8

import JTAGTAP


class XSVFDecoder(object):
    """
    Decoder
    """

    # Instruction op codes
    XCOMPLETE = 0
    XTDOMASK = 1
    XSIR = 2
    XSDR = 3
    XRUNTEST = 4
    XRESERVED_5 = 5
    XRESERVED_6 = 6
    XREPEAT = 7
    XSDRSIZE = 8
    XSDRTDO = 9
    XSETSDRMASKS = 10
    XSDRINC = 11
    XSDRB = 12
    XSDRC = 13
    XSDRE = 14
    XSDRTDOB = 15
    XSDRTDOC = 16
    XSDRTDOE = 17
    XSTATE = 18
    XENDIR = 19
    XENDDR = 20
    XSIR2 = 21
    XCOMMENT = 22
    XWAIT = 23

    @staticmethod
    def num_bytes(n):
        return (n + 7) >> 3

    def __init__(self, args):
        self._args = args
        self._error_code = 0
        self._iter_file = None
        #
        self._next_state = 0
        self._sirsize_bits = 0
        self._sirsize_bytes = 0
        self._sdrsize_bits = 0
        self._sdrsize_bytes = 0
        self._length2_bits = 0
        self._length2_bytes = 0
        self._repeat = 0
        self._runtest = 0
        self._endir_state = 0
        self._enddr_state = 0
        self._wait_start_state = 0
        self._wait_end_state = 0
        self._wait_time_usecs = 0
        self._xcomplete = 0

        #
        # uint32_t S_MAX_CHAIN_SIZE_BYTES = 40;
        # uint32_t S_MAX_CHAIN_SIZE_BITS = S_MAX_CHAIN_SIZE_BYTES * 8;
        self._tdi = []
        self._tdo = []
        self._tdo_mask = []
        self._tdo_expected = []
        self._address_mask = []
        self._data_mask = []
        self._xsdrinc_start_address = []
        self._xsdrinc_num_times = 0
        self._xsdrinc_data_list = []
        self._comment = ""
        self._instruction_counter = 0
        self._current_instruction_string = ""

    @property
    def error_code(self):
        return self._error_code

    @error_code.setter
    def error_code(self, value):
        self._error_code = value

    @property
    def next_state(self):
        """The next state to go to."""
        return self._next_state

    @next_state.setter
    def next_state(self, value):
        self._next_state = value

    @property
    def sirsize_bits(self):
        """The number of bits of the next SIR instruction."""
        return self._sirsize_bits

    @sirsize_bits.setter
    def sirsize_bits(self, value):
        self._sirsize_bits = value
        self._sirsize_bytes = XSVFDecoder.num_bytes(value)

    @property
    def sirsize_bytes(self):
        return self._sirsize_bytes

    @property
    def sdrsize_bits(self):
        """The number of bits of the next XSDR instruction."""
        return self._sdrsize_bits

    @sdrsize_bits.setter
    def sdrsize_bits(self, value):
        self._sdrsize_bits = value
        self._sdrsize_bytes = XSVFDecoder.num_bytes(value)

    @property
    def sdrsize_bytes(self):
        return self._sdrsize_bytes

    @property
    def length2_bits(self):
        """The number of bits of the length2 parameter."""
        return self._length2_bits

    @length2_bits.setter
    def length2_bits(self, value):
        self._length2_bits = value
        self._length2_bytes = XSVFDecoder.num_bytes(value)

    @property
    def length2_bytes(self):
        return self._length2_bytes

    @property
    def repeat(self):
        """The number of repetitions of the XREPEAT instruction."""
        return self._repeat

    @repeat.setter
    def repeat(self, value):
        self._repeat = value

    @property
    def runtest(self):
        """The number of repetitions of the XRUNTEST instruction."""
        return self._runtest

    @runtest.setter
    def runtest(self, value):
        self._runtest = value

    @property
    def endir_state(self):
        """Final state of XENDIR instruction."""
        return self._endir_state

    @endir_state.setter
    def endir_state(self, value):
        self._endir_state = value

    @property
    def enddr_state(self):
        """Final state of XENDDR instruction."""
        return self._enddr_state

    @enddr_state.setter
    def enddr_state(self, value):
        self._enddr_state = value

    @property
    def wait_start_state(self):
        """Start state of XWAIT instruction."""
        return self._wait_start_state

    @wait_start_state.setter
    def wait_start_state(self, value):
        self._wait_start_state = value

    @property
    def wait_end_state(self):
        """End state of XWAIT instruction."""
        return self._wait_end_state

    @wait_end_state.setter
    def wait_end_state(self, value):
        self._wait_end_state = value

    @property
    def wait_time_usecs(self):
        """Duration of the XWAIT instruction in microseconds."""
        return self._wait_time_usecs

    @wait_time_usecs.setter
    def wait_time_usecs(self, value):
        self._wait_time_usecs = value

    @property
    def xcomplete(self):
        """True if the XCOMPLETE instruction been executed."""
        return self._xcomplete

    @xcomplete.setter
    def xcomplete(self, value):
        self._xcomplete = value

    @property
    def tdi(self):
        """The TDI sequence to be written"""
        return self._tdi

    @tdi.setter
    def tdi(self, value):
        self._tdi = value

    @tdi.deleter
    def tdi(self):
        del self._tdi

    @property
    def tdo(self):
        """The TDO sequence to be read"""
        return self._tdo

    @tdo.setter
    def tdo(self, value):
        self._tdo = value

    @tdo.deleter
    def tdo(self):
        del self._tdo

    @property
    def tdo_mask(self):
        """The TDO sequence mask"""
        return self._tdo_mask

    @tdo_mask.setter
    def tdo_mask(self, value):
        self._tdo_mask = value

    @tdo_mask.deleter
    def tdo_mask(self):
        del self._tdo_mask

    @property
    def tdo_expected(self):
        """The TDO sequence to be read"""
        return self._tdo_expected

    @tdo_expected.setter
    def tdo_expected(self, value):
        self._tdo_expected = value

    @tdo_expected.deleter
    def tdo_expected(self):
        del self._tdo_expected

    @property
    def address_mask(self):
        """The XSETSDRMASKS address mask"""
        return self._address_mask

    @address_mask.setter
    def address_mask(self, value):
        self._address_mask = value

    @address_mask.deleter
    def address_mask(self):
        del self._address_mask

    @property
    def data_mask(self):
        """The XSETSDRMASKS data mask"""
        return self._data_mask

    @data_mask.setter
    def data_mask(self, value):
        self._data_mask = value

    @data_mask.deleter
    def data_mask(self):
        del self._data_mask

    @property
    def xsdrinc_start_address(self):
        """XSDRINC start address"""
        return self._xsdrinc_start_address

    @xsdrinc_start_address.setter
    def xsdrinc_start_address(self, value):
        self._xsdrinc_start_address = value

    @xsdrinc_start_address.deleter
    def xsdrinc_start_address(self):
        del self._xsdrinc_start_address

    @property
    def xsdrinc_num_times(self):
        """The length of the XSDRINC data list"""
        return self._xsdrinc_num_times

    @xsdrinc_num_times.setter
    def xsdrinc_num_times(self, value):
        self._xsdrinc_num_times = value

    @xsdrinc_num_times.deleter
    def xsdrinc_num_times(self):
        del self._xsdrinc_num_times

    @property
    def xsdrinc_data_list(self):
        return self._xsdrinc_data_list

    @xsdrinc_data_list.setter
    def xsdrinc_data_list(self, value):
        self._xsdrinc_data_list = value

    @xsdrinc_data_list.deleter
    def xsdrinc_data_list(self):
        del self._xsdrinc_data_list

    @property
    def comment(self):
        """The last XCOMMENT read"""
        return self._comment

    @comment.setter
    def comment(self, value):
        self._comment = value

    @comment.deleter
    def comment(self):
        del self._comment

    @property
    def instruction_counter(self):
        """The number of instructions processed."""
        return self._instruction_counter

    @instruction_counter.setter
    def instruction_counter(self, value):
        self._instruction_counter = value

    @property
    def current_instruction_string(self):
        """A string containing the bytes of the current instruction"""
        return self._current_instruction_string

    @current_instruction_string.setter
    def current_instruction_string(self, value):
        self._current_instruction_string = value

    @current_instruction_string.deleter
    def current_instruction_string(self):
        del self._current_instruction_string

    #
    # Decoders
    #

    def decode_xcomplete(self):
        self._xcomplete = True
        return True

    def decode_xtdomask(self):
        self.tdo_mask = self.get_next_bytes(self.sdrsize_bytes)
        return True

    def decode_xsir(self):
        self.sirsize_bits = self.get_next_byte()
        self.tdi = self.get_next_bytes(self.sirsize_bytes)
        return True

    def decode_xsdr(self):
        self.tdi = self.get_next_bytes(self.sdrsize_bytes)
        return True

    def decode_xruntest(self):
        self.runtest = self.get_next_long()
        return True

    def decode_xreserved_5(self):
        return True

    def decode_xreserved_6(self):
        return True

    def decode_xrepeat(self):
        self.repeat = self.get_next_byte()
        return True

    def decode_xsdrsize(self):
        self.sdrsize_bits = self.get_next_long()
        return True

    def decode_xsdrtdo(self):
        self.tdi = self.get_next_bytes(self.sdrsize_bytes)
        self.tdo_expected = self.get_next_bytes(self.sdrsize_bytes)
        return True

    def decode_xsetsdrmasks(self):
        self.address_mask = self.get_next_bytes(self.sdrsize_bytes)
        self.data_mask = self.get_next_bytes(self.sdrsize_bytes)
        # Count the number of bits '1' in data_mask
        n = 0
        for j in range(self.sdrsize_bytes):
            b = self.data_mask[j]
            for k in range(8):
                if b & 1:
                    n += 1
                b >>= 1
        self.length2_bits = n
        return True

    def decode_xsdrinc(self):
        self.xsdrinc_start_address = self.get_next_bytes(self.sdrsize_bytes)
        self.xsdrinc_num_times = self.get_next_byte()
        n = self.xsdrinc_num_times
        self.xsdrinc_data_list = []
        while n:
            self.xsdrinc_data_list.append(
                self.get_next_bytes(self.length2_bytes))
            n -= 1
        return True

    def decode_xsdrb(self):
        self.tdi = self.get_next_bytes(self.sdrsize_bytes)
        return True

    def decode_xsdrc(self):
        self.tdi = self.get_next_bytes(self.sdrsize_bytes)
        return True

    def decode_xsdre(self):
        self.tdi = self.get_next_bytes(self.sdrsize_bytes)
        return True

    def decode_xsdrtdob(self):
        self.tdi = self.get_next_bytes(self.sdrsize_bytes)
        self.tdo_expected = self.get_next_bytes(self.sdrsize_bytes)
        return True

    def decode_xsdrtdoc(self):
        self.tdi = self.get_next_bytes(self.sdrsize_bytes)
        self.tdo_expected = self.get_next_bytes(self.sdrsize_bytes)
        return True

    def decode_xsdrtdoe(self):
        self.tdi = self.get_next_bytes(self.sdrsize_bytes)
        self.tdo_expected = self.get_next_bytes(self.sdrsize_bytes)
        return True

    def decode_xstate(self):
        self.next_state = self.get_next_byte()
        return True

    def decode_xendir(self):
        ret = True
        s = self.get_next_byte()
        if s == 0:
            self.endir_state = JTAGTAP.JTAGTAP.RUN_TEST_IDLE
        elif s == 1:
            self.endir_state = JTAGTAP.JTAGTAP.PAUSE_IR
        else:
            self.error_code = -2
            ret = False
            print("Invalid XENDIR parameter %d" % s)
        return ret

    def decode_xenddr(self):
        ret = True
        s = self.get_next_byte()
        if s == 0:
            self.enddr_state = JTAGTAP.JTAGTAP.RUN_TEST_IDLE
        elif s == 1:
            self.enddr_state = JTAGTAP.JTAGTAP.PAUSE_DR
        else:
            self.error_code = -3
            ret = False
            print("Invalid XENDDR parameter %d" % s)
        return ret

    def decode_xsir2(self):
        self.sirsize_bits = self.get_next_word()
        self.tdi = self.get_next_bytes(self.sirsize_bytes)
        return True

    def decode_xcomment(self):
        while True:
            c = self.get_next_byte()
            if not c:
                break
            self.comment += chr(c)
        return True

    def decode_xwait(self):
        self.wait_start_state = self.get_next_byte()
        self.wait_end_state = self.get_next_byte()
        self.wait_time_usecs = self.get_next_long()
        return True

    #
    _NAME = 0
    _DECODER = 1
    _instruction_data = (
        ('XCOMPLETE',    decode_xcomplete),
        ('XTDOMASK',     decode_xtdomask),
        ('XSIR',         decode_xsir),
        ('XSDR',         decode_xsdr),
        ('XRUNTEST',     decode_xruntest),
        ('XRESERVED_5',  decode_xreserved_5),
        ('XRESERVED_6',  decode_xreserved_6),
        ('XREPEAT',      decode_xrepeat),
        ('XSDRSIZE',     decode_xsdrsize),
        ('XSDRTDO',      decode_xsdrtdo),
        ('XSETSDRMASKS', decode_xsetsdrmasks),
        ('XSDRINC',      decode_xsdrinc),
        ('XSDRB',        decode_xsdrb),
        ('XSDRC',        decode_xsdrc),
        ('XSDRE',        decode_xsdre),
        ('XSDRTDOB',     decode_xsdrtdob),
        ('XSDRTDOC',     decode_xsdrtdoc),
        ('XSDRTDOE',     decode_xsdrtdoe),
        ('XSTATE',       decode_xstate),
        ('XENDIR',       decode_xendir),
        ('XENDDR',       decode_xenddr),
        ('XSIR2',        decode_xsir2),
        ('XCOMMENT',     decode_xcomment),
        ('XWAIT',        decode_xwait),
    )

    @staticmethod
    def instruction_name(instruction):
        return XSVFDecoder._instruction_data[instruction][XSVFDecoder._NAME]

    def instruction_decoder(self, instruction):
        return self._instruction_data[instruction][XSVFDecoder._DECODER](self)

    def instruction_handler(self, instruction):
        pass

    @staticmethod
    def bytes_from_file(fd, chunksize=8192):
        with fd:
            while True:
                chunk = fd.read(chunksize)
                if chunk:
                    for b in chunk:
                        yield b
                else:
                    break

    def private_next_byte(self):
        # return next(self._iter_file)
        # return XSVFDecoder.bytes_from_file(self._current_file)
        b = ord(self._current_file.read(1))
        self.current_instruction_string += '{0:02X} '.format(b)
        return b

    def get_next_byte(self):
        n = self.private_next_byte()
        return n

    def get_next_word(self):
        n = self.private_next_byte() << 8
        n |= self.private_next_byte()
        return n

    def get_next_long(self):
        n = self.private_next_byte() << 24
        n |= self.private_next_byte() << 16
        n |= self.private_next_byte() << 8
        n |= self.private_next_byte()
        return n

    def get_next_bytes(self, n):
        s = []
        while n:
            s.append(self.private_next_byte())
            n -= 1
        return s

    def decode_one_file(self, fd):
        self._xcomplete = False
        self._instruction_counter = 0
        self._iter_file = iter(fd)
        self._current_file = fd
        while not self._xcomplete:
            self.current_instruction_string = ""
            instruction = self.get_next_byte()
            if instruction > self.XWAIT:
                print("Invalid instruction: 0x%02x" % instruction)
                return False
            ok = self.instruction_decoder(instruction)
            if ok:
                self.instruction_handler(instruction)
        return True

    def decode_all_files(self, fd_list):
        ok = True
        for fd in fd_list:
            with fd:
                ok = self.decode_one_file(fd)
                if not ok:
                    break
        return ok
