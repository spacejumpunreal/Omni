# -*- encoding: utf-8 -*-
import build_target


class Omni(build_target.BuildTarget):
    def __init__(self, base_dir):
        super(Omni, self).__init__(base_dir)
        self.target_type = build_target.TARGET_TYPE_DUMMY
        #self.add_target("Programs/PlayGround")
        self.add_target("Runtime")

