# -*- encoding: utf-8 -*-
import build_target


class GTest(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(GTest, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_DEFAULT_LIBRARY

        def custom_rule(relative_path):
            tp = build_target.default_cpp_rule(relative_path)
            if tp == build_target.FILE_TYPE_SOURCE and relative_path.find("gtest-all.cc") == -1:
                return build_target.FILE_TYPE_HEADER
            return tp
        self.setup_build_files(custom_rule)
        self.add_include_path("include")
        self.add_include_path(".", build_target.PRIVATE_ITEMS)
        self.set_custom_export_import_defines(
            "GTEST_CREATE_SHARED_LIBRARY",
            "GTEST_LINKED_AS_SHARED_LIBRARY=1")

        

