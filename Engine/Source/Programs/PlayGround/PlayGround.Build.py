# -*- encoding: utf-8 -*-
import global_states
import build_target


class PlayGround(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(PlayGround, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_DEFAULT_LIBRARY
        self.group = "Program"
        self.dependencies.append("Runtime")
        self.setup_build_files()

