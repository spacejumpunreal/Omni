# -*- encoding: utf-8 -*-
import global_states
import build_target
import re


class PlayGround(build_target.BuildTarget):
    def __init__(self, base_dir):
        super(PlayGround, self).__init__(base_dir)
        self.is_library = False
        self.dependencies.append("Runtime")
        self.order = -1
        if global_states.target_platform == global_states.TARGET_IOS:
            self.source_suffixes.append(build_target.OBJC_SOURCE_SUFFIXES)
