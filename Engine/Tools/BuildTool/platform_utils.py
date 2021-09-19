# -*- encoding: utf-8 -*-
import sys
import assert_utils

PLATFORM_WINDOWS = 'PlatformWindows'
PLATFORM_IOS = 'PlatformIOS'
PLATFORM_UNKNOWN = 'PlatformUnknown'

ALL_SUPPORTED_PLATFORMS = {PLATFORM_WINDOWS, PLATFORM_IOS}


def calc_target_platform(force_target):
    if force_target is None:
        sp = sys.platform
        if sp == "darwin":
            target_platform = PLATFORM_IOS
        elif sp == "win32":
            target_platform = PLATFORM_WINDOWS
        else:
            raise NotImplementedError
        return target_platform
    else:
        assert_utils.check(
            force_target in ALL_SUPPORTED_PLATFORMS,
            "force_target(%s) is not supported" % force_target)

