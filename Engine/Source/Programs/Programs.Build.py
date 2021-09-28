# -*- encoding: utf-8 -*-
import build_target


class Programs(build_target.BuildTarget):
    def __init__(self, build_file_path):
        super(Programs, self).__init__(build_file_path)
        self.target_type = build_target.TARGET_TYPE_DUMMY
        self.add_target("PlayGround")


