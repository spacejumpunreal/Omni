# -*- encoding: utf-8 -*-
import os
import build_target


class TargetCollector(object):
    def __init__(self, root_dir):
        self.root_dir = os.path.abspath(root_dir)
        self.targets = {}
        self._targets_to_handle = []

    def _handle_build_file(self, file_path):
        if not file_path.endswith(build_target.BUILD_RULE_SUFFIX):
            raise BadBuildRuleFilePathError(file_path)
        gd = {}
        execfile(file_path, gd)
        build_rules = gd.get(build_target.BUILD_RULES_NAME)
        if build_rules is None:
            module_name = os.path.split(file_path)[1][:-len(build_target.BUILD_RULE_SUFFIX)]
            if module_name not in gd:
                raise BuildRuleClassNotFoundError(module_name, file_path)
            build_rules = [gd[module_name]]
        for clz in build_rules:
            module_name = clz.__name__
            if module_name in self.targets:
                raise DuplicatedTargetError(module_name)
            rule_instance = clz(file_path)
            rule_instance.complete()
            if rule_instance.target_type != build_target.TARGET_TYPE_DUMMY:
                self.targets[module_name] = rule_instance

    def add_target(self, sub_target_relative_path):
        joined_path = os.path.join(self.root_dir, sub_target_relative_path)
        last_component = os.path.basename(joined_path)
        if not last_component.endswith(build_target.BUILD_RULE_SUFFIX):
            joined_path = os.path.join(joined_path, last_component + build_target.BUILD_RULE_SUFFIX)
        self._handle_build_file(joined_path)

    def run(self, top_file):
        self._handle_build_file(top_file)

        # check all dependencies exists for every BuildTarget
        for k, v in self.targets.iteritems():
            for d in v.dependencies:
                if d not in self.targets:
                    raise DependencyMissingError(k, d)


class BadBuildRuleFilePathError(Exception):
    def __init__(self, file_path):
        super(BadBuildRuleFilePathError, self).__init__()
        self.file_path = file_path

    def __str__(self):
        return "BadBuildRuleFilePathError(%s)" % self.file_path


class DependencyMissingError(Exception):
    def __init__(self, owner, missed):
        super(DependencyMissingError, self).__init__()
        self.owner = owner
        self.missed = missed

    def __str__(self):
        return "DependencyMissingError(%s, %s)" % (self.owner, self.missed)


class BuildRuleClassNotFoundError(Exception):
    def __init__(self, build_rule_class_name, file_path):
        self.build_rule_class_name = build_rule_class_name
        self.file_path = file_path

    def __str__(self):
        return "BuildRuleClassNotFoundError(%s, %s)" % (self.build_rule_class_name, self.file_path)


class DuplicatedTargetError(Exception):
    def __init__(self, name):
        self.name = name

    def __str__(self):
        return "DuplicatedTargetError(%s)" % self.name