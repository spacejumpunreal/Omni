# -*- encoding: utf-8 -*-
import build_target


class Base(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(Base, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_DEFAULT_LIBRARY
        self.setup_build_files(build_target.default_runtime_rule)
        self.use_pch("BasePCH.h", "BasePCH.cpp")
        self.dependencies = ["Prelude"]


class BaseUnitTest(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(BaseUnitTest, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_CONSOLE_APP
        self.setup_build_files(build_target.default_unittest_rule)
        self.dependencies = ["Base", "GTest"]

BUILD_RULES = (Base, BaseUnitTest)
