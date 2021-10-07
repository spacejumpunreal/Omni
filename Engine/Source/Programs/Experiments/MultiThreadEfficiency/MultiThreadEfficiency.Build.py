# -*- encoding: utf-8 -*-
import global_states
import build_target


class MultiThreadEfficiency(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(MultiThreadEfficiency, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_CONSOLE_APP
        self.setup_build_files(build_target.default_runtime_rule)
        self.dependencies.append("Core")


BUILD_RULES = (MultiThreadEfficiency,)

