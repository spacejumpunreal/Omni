# -*- encoding: utf-8 -*-
import build_target


class Core(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(Core, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_DEFAULT_LIBRARY
        self.setup_build_files(build_target.default_runtime_rule)
        self.use_pch("CorePCH.h", "CorePCH.cpp")
        self.dependencies = ["Base", "Prelude"]


class CoreUnitTest(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(CoreUnitTest, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_CONSOLE_APP
        self.setup_build_files(build_target.default_unittest_rule)
        self.dependencies = ["Core", "GTest"]


BUILD_RULES = (Core, CoreUnitTest)


