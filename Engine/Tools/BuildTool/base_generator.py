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
        ret = list(all_deps)
        ret.sort()
        return ret

    def get_dependent_include_paths(self, target):
        paths = set()

        def handle_target_pub_paths(atarget):
            for pub_inc in atarget.public_includes:
                if not os.path.isabs(pub_inc):
                    paths.add(os.path.join(atarget.base_dir, pub_inc))
        handle_target_pub_paths(target)
        for priv_inc in target.private_includes:
            if not os.path.isabs(priv_inc):
                paths.add(os.path.join(target.base_dir, priv_inc))
        for p in target.dependencies:
            handle_target_pub_paths(self._targets[p])
        return paths

    @staticmethod
    def format_guid(guid):
        return "{%s}" % guid
