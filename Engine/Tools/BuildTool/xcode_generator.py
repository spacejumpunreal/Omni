# -*- encoding: utf-8 -*-
import os
import shutil
import plist_utils
import pbxproj_utils
import base_generator
import xml.etree.ElementTree as ET


class XcodeGenerator(base_generator.BaseGenerator):
    # constants
    INFO_PLIST = "Info.plist"
    EXT_XCODEPROJ = ".xcodeproj"
    app_storyboard_files = ["LaunchScreen.storyboard", "Main.storyboard"]

    def __init__(self, collector, launch_dir, build_dir, solution_path, ios_resource_dir):
        super(XcodeGenerator, self).__init__(collector, launch_dir, build_dir, solution_path)
        self._ios_resource_dir = ios_resource_dir

    def run(self):
        if os.path.exists(self._build_dir):
            shutil.rmtree(self._build_dir)
        os.makedirs(self._build_dir)
        generated_target_pbxproj_pairs = []
        for _, target in self._targets.iteritems():
            pbxproj_file = self._get_pbxproj_file_path(target)
            self._ensure_parents_exists(pbxproj_file)
            generated_target_pbxproj_pairs.append((target, pbxproj_file))
            source, headers = target.collect_source_files()
            if not target.is_library:
                self._generate_app_folder(target)
            self._generate_pbxproj_files(target, source, headers, pbxproj_file)
        self._generate_workspace(generated_target_pbxproj_pairs)

    @staticmethod
    def _ensure_path_exists(p):
        if not os.path.exists(p):
            os.makedirs(p, mode=0755)

    @staticmethod
    def _ensure_parents_exists(file_path):
        p = os.path.dirname(file_path)
        if not os.path.exists(p):
            os.makedirs(p, mode=0755)

    def _get_pbxproj_file_path(self, target):
        return os.path.join(self._build_dir, target.get_name() + self.EXT_XCODEPROJ, "project.pbxproj")

    def _get_target_app_dir(self, target):
        return os.path.join(self._build_dir, target.get_name() + "_App")

    @staticmethod
    def _get_target_name(target):
        tn = target.get_name()
        return "lib%s.a" % tn if target.is_library else "%s.app" % tn

    def _generate_pbxproj_files(self, target, source, headers, pbxproj_file):
        pbx_objects = []
        source_refs, resource_refs, framework_refs, product_ref, main_group_ref, products_group_ref = \
            self._handle_pbx_file_and_group(pbx_objects, target, source, headers, pbxproj_file)

        phase_refs = self._handle_pbx_build_phases(pbx_objects, source_refs, resource_refs, framework_refs)
        target_ref, project_ref = self._handle_pbx_target_project(
            target, pbx_objects, product_ref, phase_refs, main_group_ref, products_group_ref)
        self._handle_pbxproj(target, pbx_objects, project_ref, pbxproj_file)

    def _handle_pbx_file_and_group(self, pbx_objects, target, source, headers, pbxproj_file):
        # discover source tree structure from paths
        path2children = {}

        # discover tree from sources
        def add_all_parents_for_path(path_rel2source):
            path2children[path_rel2source] = None
            last_child = path_rel2source
            cpath = os.path.dirname(path_rel2source)
            while True:
                my_children = path2children.get(cpath)
                should_continue = False
                if my_children is None:  # haven't been added yet
                    my_children = path2children[cpath] = []
                    should_continue = True
                my_children.append(last_child)
                last_child = cpath
                should_continue = should_continue and len(cpath) != 0
                cpath = os.path.dirname(cpath)
                if not should_continue:
                    break
        all_code_files = source + headers
        prj_path = os.path.dirname(pbxproj_file)
        for pth in all_code_files:
            path_rel2source = os.path.relpath(pth, self._source_root_dir)
            add_all_parents_for_path(path_rel2source)

        # generate PBXFileReference and PBXGroup and PBXBuildFile
        # build pbx object from the tree
        prj2source = os.path.relpath(self._source_root_dir, prj_path)
        source_refs = []

        def build_pbx_rec(path):
            path_children_children = path2children[path]
            base_name = os.path.basename(path)
            if path_children_children is None:
                tp = pbxproj_utils.guess_file_type(path)
                file_ref = pbxproj_utils.build_PBXFileReference(tp, base_name, pbxproj_utils.SOURCETREE_GROUP)
                pbx_objects.append(file_ref)
                # also add PBXBuildFile
                build_file = pbxproj_utils.build_PBXBuildFile(file_ref)
                source_refs.append(build_file)
                pbx_objects.append(build_file)
                ret = file_ref
            else:
                child_nodes = []
                for c in path_children_children:
                    child_nodes.append(build_pbx_rec(c))
                ret = pbxproj_utils.build_PBXGroup(
                    child_nodes, base_name, sourceTree=pbxproj_utils.SOURCETREE_SOURCE_ROOT,
                    path=os.path.join(prj2source, path))
                pbx_objects.append(ret)
            return ret
        main_group_ref = build_pbx_rec("")

        # Resources
        file_refs = []
        resource_refs = []
        if not target.is_library:  # if app, need to add storyboard
            storyboard_files = self.app_storyboard_files
            app_dir = self._get_target_app_dir(target)
            prj2app = os.path.relpath(app_dir, prj_path)
            for storyboard_file in storyboard_files:
                file_ref = pbxproj_utils.build_PBXFileReference(pbxproj_utils.FILE_TYPE_STORYBOARD, storyboard_file)
                variant_group = pbxproj_utils.build_PBXVariantGroup([file_ref], storyboard_file)
                file_refs.append(variant_group)
                build_file = pbxproj_utils.build_PBXBuildFile(variant_group)
                pbx_objects.append(file_ref)
                pbx_objects.append(variant_group)
                pbx_objects.append(build_file)
                resource_refs.append(build_file)
            app_group = pbxproj_utils.build_PBXGroup(
                file_refs, os.path.basename(app_dir),
                sourceTree=pbxproj_utils.SOURCETREE_GROUP, path=prj2app)
            pbxproj_utils.add_to_PBXGRoup(main_group_ref, app_group)
            pbx_objects.append(app_group)

        # Products
        product_type = pbxproj_utils.FILE_TYPE_ARCHIVE if target.is_library else pbxproj_utils.FILE_TYPE_APP
        product_name = target.get_name() + ".a" if target.is_library else ".app"
        product_ref = pbxproj_utils.build_PBXFileReference(
            product_type, product_name, pbxproj_utils.SOURCETREE_PRODUCTS)
        # add Products group
        products_group_ref = pbxproj_utils.build_PBXGroup([product_ref], "Products")
        pbx_objects.append(product_ref)
        pbx_objects.append(products_group_ref)
        pbxproj_utils.add_to_PBXGRoup(main_group_ref, products_group_ref)

        # Frameworks
        deps = self.get_all_dependencies(target)
        deps.remove(target)
        framework_refs = []
        file_refs = []

        if len(deps) > 0:
            for dep in deps:
                libname = self._get_target_name(dep)
                file_ref = pbxproj_utils.build_PBXFileReference(
                    pbxproj_utils.FILE_TYPE_ARCHIVE, libname, sourceTree=pbxproj_utils.SOURCETREE_PRODUCTS)
                file_refs.append(file_ref)
                build_ref = pbxproj_utils.build_PBXBuildFile(file_ref)
                pbx_objects.append(file_ref)
                pbx_objects.append(build_ref)
                framework_refs.append(build_ref)
            framework_group_ref = pbxproj_utils.build_PBXGroup(file_refs, "Frameworks")
            pbx_objects.append(framework_group_ref)
            pbxproj_utils.add_to_PBXGRoup(main_group_ref, framework_group_ref)
        return source_refs, resource_refs, framework_refs, product_ref, main_group_ref, products_group_ref

    def _handle_pbx_build_phases(self, pbx_objects, source_refs, resource_refs, framework_refs):
        source_build_phase = pbxproj_utils.build_PBXSourcesBuildPhase(files=source_refs)
        pbx_objects.append(source_build_phase)

        resource_build_phase = pbxproj_utils.build_PBXResourcesBuildPhase(files=resource_refs)
        pbx_objects.append(resource_build_phase)

        framework_build_phase = pbxproj_utils.build_PBXFrameworksBuildPhase(files=framework_refs)
        pbx_objects.append(framework_build_phase)

        phase_refs = [source_build_phase, resource_build_phase, framework_build_phase]
        return phase_refs

    def _handle_pbx_target_project(self, target, pbx_objects, product_ref, phase_refs,
                                   main_group_ref, products_group_ref):
        # XCBuildConfiguration
        is_app = not target.is_library
        inc_paths = self.get_dependent_include_paths(target)
        prj_dir = os.path.dirname(self._get_pbxproj_file_path(target))
        fields2update = [("HEADER_SEARCH_PATHS", map(lambda x: os.path.relpath(x, prj_dir), inc_paths))]
        target_debug = pbxproj_utils.build_XCBuildConfiguration(
            pbxproj_utils.get_build_setting(True, is_app, False, fields2update),
            "Debug")
        target_release = pbxproj_utils.build_XCBuildConfiguration(
            pbxproj_utils.get_build_setting(False, is_app, False, fields2update),
            "Release")
        project_debug = pbxproj_utils.build_XCBuildConfiguration(
            pbxproj_utils.get_build_setting(True, is_app, True),
            "Debug")
        project_release = pbxproj_utils.build_XCBuildConfiguration(
            pbxproj_utils.get_build_setting(False, is_app, True),
            "Release")

        target_config_list = pbxproj_utils.build_XCConfigurationList([target_debug, target_release], "Release")
        project_config_list = pbxproj_utils.build_XCConfigurationList([project_debug, project_release], "Release")
        pbx_objects.append(target_config_list)
        pbx_objects.append(project_config_list)
        pbx_objects += [target_debug, target_release, project_debug, project_release]
        # PBXNativeTarget
        target_name = target.get_name()
        product_type = pbxproj_utils.PRODUCT_TYPE_STATIC_LIBRARY \
            if target.is_library else pbxproj_utils.PRODUCT_TYPE_APP
        target_ref = pbxproj_utils.build_PBXNativeTarget(
            buildConfigurationList=target_config_list,
            buildPhases=phase_refs,
            buildRules=(),
            dependencies=(),
            name=target_name,
            productName=target_name,
            productReference=product_ref,
            productType=product_type)
        project_ref = pbxproj_utils.build_PBXProject(
            buildConfigurationList=project_config_list,
            mainGroup=main_group_ref,
            productRefGroup=products_group_ref,
            targets=(target_ref,))
        # PBXProject
        pbx_objects.append(target_ref)
        pbx_objects.append(project_ref)
        return target_ref, project_ref

    @staticmethod
    def _handle_pbxproj(target, pbx_objects, project_ref, pbxproj_file):
        txt = pbxproj_utils.dump_pbxproj(pbx_objects, target.get_name(), project_ref)
        with open(pbxproj_file, "wb") as wf:
            wf.write(txt)

    def _generate_app_folder(self, target):
        app_dir = self._get_target_app_dir(target)
        self._ensure_path_exists(app_dir)
        tree = plist_utils.load_ios_app_info_plist_template()
        plist_utils.dump_to_file(os.path.join(app_dir, self.INFO_PLIST), tree)
        for storyboard_file in self.app_storyboard_files:
            dst_path = os.path.join(app_dir, storyboard_file)
            src_path = os.path.join(self._ios_resource_dir, storyboard_file)
            shutil.copyfile(src_path, dst_path)


    WORKSPACE_CONTENT = """<?xml version="1.0" encoding="UTF-8"?>
<Workspace version = "1.0">
</Workspace>"""

    def _generate_workspace(self, generated_target_pbxproj_pairs):
        if not os.path.exists(self._solution_path):
            os.makedirs(self._solution_path, 0755)
            os.path.join(self._solution_path, "")
        root = ET.fromstring(self.WORKSPACE_CONTENT)
        root_path = os.path.dirname(self._solution_path)
        for target, prj_path in generated_target_pbxproj_pairs:
            group_string = "group:%s" % os.path.relpath(prj_path, root_path)
            ET.SubElement(root, "FileRef", attrib={"location": group_string})
        with open(os.path.join(self._solution_path, "contents.xcworkspacedata"), "wb") as wf:
            wf.write(ET.tostring(root, encoding='UTF-8'))
