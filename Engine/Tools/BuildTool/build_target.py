# -*- encoding: utf-8 -*-
import os
import re
import uuid


BUILD_RULE_SUFFIX = ".Build.py"
DEFAULT_SOURCE_SUFFIXES = re.compile(r".*\.cpp"),
DEFAULT_HEADER_SUFFIXES = re.compile(r".*\.h"),


def is_parent_of(directory, path):
    return os.path.join(path, "").startswith(os.path.join(directory, ""))


class BuildTarget(object):
    def __init__(self, base_dir):
        self.is_library = True
        self.include_source_dir = True
        self.base_dir = base_dir
        self.dependencies = []
        self.exported_dirs = []
        self.excluded_paths = []
        self.guid = uuid.uuid4()
        self.order = 0
        self.source_suffixes = list(DEFAULT_SOURCE_SUFFIXES)
        self.header_suffixes = list(DEFAULT_HEADER_SUFFIXES)

    def collect_source_files(self):
        headers = []
        source = []
        for root, _, files in os.walk(self.base_dir):
            for fn in files:
                full_path = os.path.join(root, fn)
                skip = False
                for epath in self.excluded_paths:
                    if is_parent_of(epath, full_path):
                        skip = True
                        break
                if skip:
                    continue

                ok = False
                for pat in self.source_suffixes:
                    if pat.match(fn):
                        source.append(full_path)
                        ok = True
                        break
                if ok:
                    continue

                for pat in self.header_suffixes:
                    if pat.match(fn):
                        headers.append(full_path)
                        break
        return source, headers

    def get_relative_path(self, p):
        return os.path.relpath(p, self.base_dir)

    def get_name(self):
        return self.__class__.__name__

    def get_build_file_path(self):
        return os.path.join(self.base_dir, ("%s." + BUILD_RULE_SUFFIX) % self.get_name())
