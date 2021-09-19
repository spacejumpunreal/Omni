# -*- encoding: utf-8 -*-
import build_target


class Core(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(Core, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_DEFAULT_LIBRARY
        self.setup_build_files()
        

