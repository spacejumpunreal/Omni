# -*- encoding: utf-8 -*-
import os
import uuid
import global_states

BUILD_RULE_SUFFIX = ".Build.py"
SOURCE_CXX_SUFFIXES = {".c", ".cpp", ".cxx", ".cc"}
HEADER_CXX_SUFFIXES = {".h", ".hpp", ".hxx", ".hh"}

FILE_TYPE_SOURCE = "FILE_TYPE_SOURCE"
FILE_TYPE_HEADER = "FILE_TYPE_HEADER"
FILE_TYPE_RESOURCE = "FILE_TYPE_RESOURCE"
FILE_TYPE_OTHER = "FILE_TYPE_OTHER"
FILE_TYPE_IGNORED = None

TARGET_TYPE_DUMMY = "Dummy"
TARGET_TYPE_NO_BUILD = "NoBuild"
TARGET_TYPE_STATIC_LIBRARY = "StaticLibrary"
TARGET_TYPE_DYNAMIC_LIBRARY = "DynamicLibrary"
TARGET_TYPE_DEFAULT_LIBRARY = "Library"

TARGET_TYPE_CONSOLE_APP = "ConsoleApp"
TARGET_TYPE_WINDOW_APP = "WindowApp"

ALL_LIBRARY_TYPES = {TARGET_TYPE_DYNAMIC_LIBRARY, TARGET_TYPE_STATIC_LIBRARY}

BUILD_RULES_NAME = "BUILD_RULES"

PUBLIC_ITEMS = 1
PRIVATE_ITEMS = 2

PCH_SUFFIX = ".pch"


def enumerate_input_files(base_dir, rule_func):
    ret = {FILE_TYPE_SOURCE: [], FILE_TYPE_HEADER: [], FILE_TYPE_RESOURCE: []}
    for root, _, files in os.walk(base_dir):
        for filename in files:
            full_path = os.path.join(root, filename)
            rpath = os.path.relpath(full_path, base_dir)
            tp = rule_func(rpath)
            if tp is not FILE_TYPE_IGNORED:
                file_list = ret.get(tp)
                if file_list:
                    file_list.append(full_path)
                else:
                    ret[tp] = [full_path]
    return ret


def exclude_regex(path, reg):
    return reg.match(path) is not None


def path_ends_with_any_of(filepath, suffixes):
    splits = os.path.splitext(filepath)
    return splits[1] in suffixes


def default_cpp_rule(relative_path):
    splits = os.path.splitext(relative_path)
    ext = splits[1]
    if ext in SOURCE_CXX_SUFFIXES:
        return FILE_TYPE_SOURCE
    if ext in HEADER_CXX_SUFFIXES:
        return FILE_TYPE_HEADER
    return FILE_TYPE_IGNORED


def default_runtime_rule(relative_path):
    if relative_path.find("_UnitTest") != -1:
        return FILE_TYPE_IGNORED
    return default_cpp_rule(relative_path)


def default_unittest_rule(relative_path):
    if relative_path.find("_UnitTest") != -1:
        return default_cpp_rule(relative_path)
    else:
        return FILE_TYPE_IGNORED


class BuildTarget(object):
    def __init__(self, build_file_path):
        self.build_file_path = build_file_path
        self.target_type = TARGET_TYPE_DUMMY
        self.group = ""
        self.base_dir = os.path.dirname(build_file_path)
        self.dependencies = []
        self.public_includes = []
        self.private_includes = [global_states.source_root]
        self.files = []
        self.public_defines = []
        self.private_defines = []
        self.custom_export_define = None
        self.custom_import_define = None
        self.pch = None
        self.guid = uuid.uuid5(uuid.NAMESPACE_URL, build_file_path + self.__class__.__name__)

    def complete(self):
        self_name = self.get_name().upper()
        if self.target_type == TARGET_TYPE_DEFAULT_LIBRARY:
            self.target_type = global_states.default_library_type
        if self.target_type in ALL_LIBRARY_TYPES:
            if self.custom_export_define is None:
                self.custom_export_define = "EXPORT_" + self_name
            if self.custom_import_define is None:
                self.custom_import_define = "IMPORT_" + self_name

            is_dynamic = 0 if self.target_type == TARGET_TYPE_STATIC_LIBRARY else 1
            if is_dynamic:
                self._set_api_export_import_defines(self.custom_export_define, self.custom_import_define)
            self.add_macro_define("%s_IS_DYNAMIC_LIBRARY=%d" % (self_name, is_dynamic))

    def get_relative_path(self, p):
        return os.path.relpath(p, self.base_dir)

    def get_name(self):
        return self.__class__.__name__

    def add_target(self, relative_path):
        global_states.collector.add_target(os.path.join(self.base_dir, relative_path))

    def get_lib_name(self):
        return "lib" + self.get_name()

    def get_lib_path(self, root):
        relative_path = os.path.relpath(self.base_dir + self.get_lib_name(), global_states.source_root)
        return os.path.join(root, relative_path)

    def setup_build_files(self, rule_func):
        self.files = enumerate_input_files(self.base_dir, rule_func)

    def add_include_path(self, path, acl=PUBLIC_ITEMS | PRIVATE_ITEMS):
        if acl & PUBLIC_ITEMS:
            self.public_includes.append(path)
        if acl & PRIVATE_ITEMS:
            self.private_includes.append(path)

    def add_macro_define(self, kv_pair, acl=PUBLIC_ITEMS | PRIVATE_ITEMS):
        if acl & PUBLIC_ITEMS:
            self.public_defines.append(kv_pair)
        if acl & PRIVATE_ITEMS:
            self.private_defines.append(kv_pair)

    def set_custom_export_import_defines(self, export_def, import_def):
        self.custom_export_define = export_def
        self.custom_import_define = import_def

    def _set_api_export_import_defines(self, export_def, import_def):
        if global_states.default_library_type == TARGET_TYPE_DYNAMIC_LIBRARY:
            self.add_macro_define(export_def, PRIVATE_ITEMS)
            self.add_macro_define(import_def, PUBLIC_ITEMS)

    def use_pch(self, header, source):
        def handle_local_file(file_path):
            if os.path.isabs(file_path):
                return file_path
            else:
                return os.path.join(self.base_dir, file_path)

        def get_source_root_path(file_path):
            jp = os.path.join(self.base_dir, file_path)
            return os.path.relpath(jp, global_states.source_root)
        self.pch = get_source_root_path(header), handle_local_file(source)




