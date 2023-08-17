#! /usr/bin/python3
# coding: utf-8

class JTAGTAP(object):
    """

    """

    # JTAG TAP states
    TEST_LOGIC_RESET = 0
    RUN_TEST_IDLE = 1
    SELECT_DR_SCAN = 2
    CAPTURE_DR = 3
    SHIFT_DR = 4
    EXIT1_DR = 5
    PAUSE_DR = 6
    EXIT2_DR = 7
    UPDATE_DR = 8
    SELECT_IR_SCAN = 9
    CAPTURE_IR = 10
    SHIFT_IR = 11
    EXIT1_IR = 12
    PAUSE_IR = 13
    EXIT2_IR = 14
    UPDATE_IR = 15

    _NAME = 0
    _state_data = (
        ('TEST_LOGIC_RESET',),
        ('RUN_TEST_IDLE',),
        ('SELECT_DR_SCAN',),
        ('CAPTURE_DR',),
        ('SHIFT_DR',),
        ('EXIT1_DR',),
        ('PAUSE_DR',),
        ('EXIT2_DR',),
        ('UPDATE_DR',),
        ('SELECT_IR_SCAN',),
        ('CAPTURE_IR',),
        ('SHIFT_IR',),
        ('EXIT1_IR',),
        ('PAUSE_IR',),
        ('EXIT2_IR',),
        ('UPDATE_IR',),
    )

    @staticmethod
    def state_name(n):
        return JTAGTAP._state_data[n][JTAGTAP._NAME]
