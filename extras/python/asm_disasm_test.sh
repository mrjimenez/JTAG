#! /bin/bash
./XSVFAssembler.py > test.xsvf
./xsvf -c disasm -n test.xsvf > test.xsvf.s
./xsvf -c asm test.xsvf.s > test2.xsvf
sha1sum test*.xsvf

