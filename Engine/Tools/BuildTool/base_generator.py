# -*- encoding: utf-8 -*-
import os


class BaseGenerator(object):
    def __init__(self, collector):
        self._targets = collector.targets

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
        paths = set()
        for p in target.dependencies:
            dependent_target = self._targets[p]
            for pinclude in dependent_target.public_includes:
                if not os.path.isabs(pinclude):
                    paths.add(os.path.join(target.base_dir, pinclude))
        return paths

    @staticmethod
    def format_guid(guid):
        return "{%s}" % guid
