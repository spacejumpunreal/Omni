# -*- encoding: utf-8 -*-
import os
import build_target


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

    def get_dependent_properties(self, target, prop_name, self_option):
        ret = set()

        def handle_target_items(pub_priv, atarget):
            for x_item in getattr(atarget, pub_priv + prop_name):
                if prop_name == "includes" and not os.path.isabs(x_item):
                    ret.add(os.path.join(atarget.base_dir, x_item))
                else:
                    ret.add(x_item)
        if self_option & build_target.PUBLIC_ITEMS:
            handle_target_items("public_", target)
        if self_option & build_target.PRIVATE_ITEMS:
            handle_target_items("private_", target)

        all_deps = self.get_all_dependencies(target)
        all_deps.remove(target)
        for p in all_deps:
            handle_target_items("public_", p)
        ret = list(ret)
        ret.sort()
        return ret

    @staticmethod
    def format_guid(guid):
        return "{%s}" % guid
