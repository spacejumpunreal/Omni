# -*- encoding: utf-8 -*-
import sys

TARGET_WINDOWS = 'TargetWindows'
TARGET_IOS = 'TargetIOS'

target_platform = 'TargetNone'


def init_target_platform(force_target):
    global target_platform
    if force_target is None:
        sp = sys.platform
        if sp == "darwin":
            target_platform = TARGET_IOS
        elif sp == "win32":
            target_platform = TARGET_WINDOWS
        else:
            raise NotImplementedError
    return target_platform
