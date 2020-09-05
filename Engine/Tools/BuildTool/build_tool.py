# -*- encoding: utf-8 -*-
import collector


def generate_vs_project(root_dir, debug_dir, build_dir, solution_path):
    import vs_generator
    c = collector.Collector(root_dir)
    c.run()
    g = vs_generator.VS2019Generator(c, debug_dir, build_dir, solution_path)
    g.run()
