# -*- encoding: utf-8 -*-
import os
import shutil


class BaseGenerator(object):
    def __init__(self, collector, launch_dir, build_dir, solution_path):
        self._targets = collector.targets
        self._source_root_dir = collector.root_dir
        self._launch_dir = launch_dir
        self._build_dir = build_dir
        self._solution_path = solution_path

    def get_all_dependencies(self, target):
        q = [target]
        all_deps = set()
        while q:
            x = q.pop()
            if x in all_deps:
                continue
            all_deps.add(x)
            for d in x.dependencies:
                dd = self._targets[d]
                if dd not in all_deps:
                    q.append(dd)
        return all_deps

    def get_dependent_include_paths(self, target):
        paths = []
        for p in target.dependencies:
            paths += self._targets[p].exported_dirs
        if target.include_source_dir:
            paths.append(self._source_root_dir)
        return paths

    @staticmethod
    def format_guid(guid):
        return "{%s}" % guid
