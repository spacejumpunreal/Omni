# -*- encoding: utf-8 -*-
import build_target


class Runtime(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(Runtime, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_DUMMY
        self.add_target("Prelude")
        self.add_target("Base")
        #self.add_target("Core")
