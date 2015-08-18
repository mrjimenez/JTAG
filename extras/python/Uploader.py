#!/usr/bin/python
# coding: utf-8

import os
import string
import sys
import time

try:
    import serial
except ImportError:
    print "Error importing pyserial. Please check if it is installed."
    sys.exit(1)

class Uploader(object):
    """Uploads a XSVF file to the arduino board.
    """

    # Create a translation array of printable characters
    _printable_chars = string.digits + string.letters + string.punctuation + ' '
    _translate_str = ''.join(
        [(chr(x) in _printable_chars) and chr(x) or '.' for x in range(256)])


    @staticmethod
    def add_arguments(p):
        """Adds the necessary arguments to the parser."""
        p.add_argument(
            '-p', '--port',
            default='/dev/ttyACM0',
            help='Serial port device name'
                 ' (default=%(default)s)')
        p.add_argument(
            '-b', '--baud',
            default=115200,
            type=int,
            help='BAUD rate'
                 ' (type %(type)s, default=%(default)s)')

    def __init__(self, port, baud):
        self._serial = serial.Serial(port=port, baudrate=baud)
        # Help printing new lines
        self._need_lf = False
        #
        self._file_size = 0
        # Hashes
        self._sum = 0
        # To compute the elapsed time
        self._start_time = 0

    def reset_arduino(self):
        """Resets the arduino and clear any garbage on the serial port."""
        self._serial.setDTR(False)
        time.sleep(1)
        self._serial.flushInput()
        self._serial.flushOutput()
        self._serial.setDTR(True)
        self._start_time = 0

    def print_lf(self):
        if self._need_lf:
            self._need_lf = False
            print()

    def initialize_hashes(self):
        self._sum = 0

    def update_hashes(self, s):
        for c in s:
            self._sum += ord(c)

    def print_hashes(self):
        cksum = (-self._sum) & 0xFF
        print '  Expected checksum:  0x%02X/%lu.' % (cksum, self._file_size)
        print '  Expected sum: 0x%08lX/%lu.' % (self._sum, self._file_size)
        if self._start_time > 0:
            print 'Elapsed time: %.02f seconds.' % \
                  (time.time() - self._start_time)

    def upload_one_file(self, fd):
        self.reset_arduino()
        self._file_size = os.fstat(fd.fileno()).st_size
        bytes_written = 0
        self._serial.write('xsvf\n')
        while True:
            line = self._serial.readline().strip()
            if not line:
                continue
            command = line[0]
            argument = line[1:]
            if command == 'S':
                num_bytes = int(argument)
                xsvf_data = fd.read(num_bytes)
                bytes_written += len(xsvf_data)
                self.update_hashes(xsvf_data)
                xsvf_data += chr(0xff) * (num_bytes - len(xsvf_data))
                self._serial.write(xsvf_data)
                print '\rSent: %8d bytes, %8d remaining' % \
                      (bytes_written, self._file_size - bytes_written),
                sys.stdout.flush()
                self._need_lf = True
            elif command == 'R':
                self.initialize_hashes()
                print 'File: %s' % os.path.realpath(fd.name)
                print 'Ready to send %d bytes.' % self._file_size
                self._start_time = time.time()
            elif command == 'Q':
                self.print_lf()
                print 'Received device quit:', argument
                self.print_hashes()
                return True
            elif command == 'D':
                self.print_lf()
                print 'Device:', argument
            elif command == '!':
                self.print_lf()
                print 'IMPORTANT:', argument
            else:
                self.print_lf()
                print 'Unrecognized line:',\
                    line.translate(Uploader._translate_str)

    def upload_all_files(self, fd_list):
        ok = True
        for fd in fd_list:
            with fd:
                ok = self.upload_one_file(fd)
                if not ok:
                    break
        return ok
