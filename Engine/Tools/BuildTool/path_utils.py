# -*- encoding: utf-8 -*-
import sys
import os
import assert_utils


def ensure_path(path):
    if path is None or path == "":
        assert_utils.die("can not ensure null path:%s" % path)
    if not os.path.exists(path):
        os.makedirs(path)


def is_parent_of(directory, path):
    return os.path.join(path, "").startswith(os.path.join(directory, ""))

