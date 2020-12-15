# -*- encoding: utf-8 -*-
import os
import collector


def generate_vs_project(root_dir, debug_dir, build_dir, solution_path):
    import vs_generator
    c = collector.Collector(root_dir)
    c.run()
    g = vs_generator.VS2019Generator(c, debug_dir, build_dir, solution_path)
    g.run()


def generate_xcode_project(root_dir, debug_dir, build_dir, solution_path):
    import xcode_generator
    c = collector.Collector(root_dir)
    c.run()
    tool_dir = os.path.dirname(os.path.abspath(__file__))
    g = xcode_generator.XcodeGenerator(c, debug_dir, build_dir, solution_path, os.path.join(tool_dir, "ToolResources"))
    g.run()

