# -*- encoding: utf-8 -*-
import os
import target_collector
import platform_utils
import global_states
import build_target


def generate_project():
    c = target_collector.TargetCollector(global_states.source_root)
    global_states.collector = c
    top_file = os.path.join(global_states.source_root, global_states.project_name + build_target.BUILD_RULE_SUFFIX)
    c.run(top_file)
    if global_states.target_platform == platform_utils.PLATFORM_WINDOWS:
        import vs_generator
        generator = vs_generator.VS2019Generator(c)
        generator.run()
    elif global_states.target_platform == platform_utils.PLATFORM_IOS:
        import xcode_generator
        generator = xcode_generator.XcodeGenerator(c)
        generator.run()

