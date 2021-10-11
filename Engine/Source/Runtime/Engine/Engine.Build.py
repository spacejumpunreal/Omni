# -*- encoding: utf-8 -*-
import build_target


class Engine(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(Engine, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_DEFAULT_LIBRARY
        self.setup_build_files(build_target.default_runtime_rule)
        self.use_pch("EnginePCH.h", "EnginePCH.cpp")
        self.dependencies = ["Core", "Prelude"]


class EngineUnitTest(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(EngineUnitTest, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_CONSOLE_APP
        self.setup_build_files(build_target.default_unittest_rule)
        self.dependencies = ["Engine", "GTest"]


BUILD_RULES = (Engine, EngineUnitTest)


