# -*- encoding: utf-8 -*-
import sys
import os


def die(msg):
    print msg
    sys.exit(-1)


def check(cond, msg):
    if not cond:
        die(msg)


def check_path_exists(path, msg=None):
    if not os.path.exists(path):
        die(("path does not exist:" + path) if msg is None else msg)


