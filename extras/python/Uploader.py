#! /usr/bin/python3
# coding: utf-8

import os
import string
import sys
import time

try:
    import serial
except ImportError:
    print("Error importing pyserial. Please check if it is installed.")
    sys.exit(1)


class Uploader(object):
    """Uploads a XSVF file to the arduino board."""

    # Create a translation array of printable characters
    _translate_str_1 = "".join(
        [(chr(x) in string.printable) and chr(x) or "." for x in range(256)]
    )
    _translate_str = bytes(_translate_str_1, "ascii")

    @staticmethod
    def add_arguments(p):
        """Adds the necessary arguments to the parser."""
        p.add_argument(
            "-p",
            "--port",
            default="/dev/ttyACM0",
            help="Serial port device name" " (default=%(default)s)",
        )
        p.add_argument(
            "-b",
            "--baud",
            default=115200,
            type=int,
            help="BAUD rate" " (type %(type)s, default=%(default)s)",
        )

    def __init__(self, args):
        self._args = args
        self._serial = serial.Serial(port=args.port, baudrate=args.baud)
        # Help printing new lines
        self._need_lf = False
        #
        self._file_size = 0
        # Hashes
        self._sum = 0
        # To compute the elapsed time
        self._start_time = 0
        # Error code
        self._error_code = 0

    @property
    def error_code(self):
        return self._error_code

    @error_code.setter
    def error_code(self, value):
        self._error_code = value

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
            print

    def initialize_hashes(self):
        self._sum = 0

    def update_hashes(self, s):
        for c in s:
            self._sum += c

    def print_hashes(self):
        cksum = (-self._sum) & 0xFF
        if self._args.debug > 1:
            print("  Expected checksum:  0x%02X/%lu." % (cksum, self._file_size))
            print("  Expected sum: 0x%08lX/%lu." % (self._sum, self._file_size))
        if self._start_time > 0:
            print("Elapsed time: %.02f seconds." % (time.time() - self._start_time))

    def upload_one_file(self, fd):
        self.reset_arduino()
        self._file_size = os.fstat(fd.fileno()).st_size
        bytes_written = 0
        while True:
            line = self._serial.readline().strip()
            if not line:
                continue
            command = chr(line[0])
            argument = line[1:].decode("ascii")
            if command == "S":
                num_bytes = int(argument)
                xsvf_data = fd.read(num_bytes)
                bytes_written += len(xsvf_data)
                self.update_hashes(xsvf_data)
                xsvf_data += b"\xFF" * (num_bytes - len(xsvf_data))
                self._serial.write(xsvf_data)
                if self._args.debug > 1:
                    print(
                        "\rSent: %8d bytes, %8d remaining"
                        % (bytes_written, self._file_size - bytes_written),
                        end="",
                    )
                    sys.stdout.flush()
                    self._need_lf = True
            elif command == "R":
                self.initialize_hashes()
                if self._args.debug > 1:
                    print("File: %s" % os.path.realpath(fd.name))
                    print("Ready to send %d bytes." % self._file_size)
                self._start_time = time.time()
            elif command == "Q":
                self.print_lf()
                # Split the argument. The first field is the error code,
                # the next field is the error message.
                args = argument.split(",")
                self.error_code = int(args[0])
                if self._args.debug > 1:
                    print("Quit: {1:s} ({0:d}).".format(self.error_code, args[1]))
                self.print_hashes()
                return self.error_code == 0
            elif command == "D":
                if self._args.debug > 0:
                    self.print_lf()
                    print("Device:", argument)
            elif command == "!":
                if self._args.debug > 0:
                    self.print_lf()
                    print("IMPORTANT:", argument)
            else:
                self.print_lf()
                print("Unrecognized line:", line.translate(Uploader._translate_str))

    def upload_all_files(self, fd_list):
        ok = True
        for fd in fd_list:
            with fd:
                ok = self.upload_one_file(fd)
                if not ok:
                    break
        return ok
