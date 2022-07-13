# -*- encoding: utf-8 -*-
import os
import uuid
import winreg
import base_generator
from xml_utils import XmlNode
import global_states
import build_target
import functools


VCXPROJ_TEMPLATE = """<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
{ProjectConfiguration}
  <PropertyGroup Label="Globals">
    <VCProjectVersion>15.0</VCProjectVersion>
    <ProjectGuid>{TargetGUID}</ProjectGuid>
    <RootNamespace>{TargetName}</RootNamespace>
    <WindowsTargetPlatformVersion>{WindowsTargetPlatformVersion}</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
{PropertyGroupConfiguration}
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
{ImportGroupPropertySheet}
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup />
{ItemDefinitionGroup}
{ItemGroupCLCompile}
{ItemGroupCLInclude}
{ItemGroupNone}
{ItemGroupProjectReference}
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>

"""

FILTER_TEMPLATE = """<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
{ItemGroupSources}
{ItemGroupFilters}
{ItemGroupIncludes}
{ItemGroupBuildFile}
</Project>

"""

USER_TEMPLATE = """<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
{PropertyGroups}
</Project>

"""

SLN_TEMPLATE = """
Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio 16
VisualStudioVersion = 16.0.29306.81
MinimumVisualStudioVersion = 10.0.40219.1
{ProjectsBlock}
Global
    GlobalSection(SolutionConfigurationPlatforms) = preSolution
{SolutionConfigurationPlatforms}
    EndGlobalSection
    GlobalSection(ProjectConfigurationPlatforms) = postSolution
{ProjectConfigurationPlatforms}
    EndGlobalSection
    GlobalSection(SolutionProperties) = preSolution
        HideSolutionNode = FALSE
    EndGlobalSection
    GlobalSection(NestedProjects) = preSolution
{NestedProjects}
    EndGlobalSection
    GlobalSection(ExtensibilityGlobals) = postSolution
        SolutionGuid = {SolutionGUID}
    EndGlobalSection
EndGlobal

"""

Configurations = ("Debug", "Release")
Platforms = ("x64",)
PlatformToolset = "v143"
Indent = "  "


def get_latest_windows_sdk_version():
    def cmp_version(a, b):
        a = a.split('.')
        b = b.split('.')
        la, lb = len(a), len(b)
        cmp_length = min(la, lb)
        for c in range(cmp_length):
            if a[c] < b[c]:
                return -1
            elif a[c] > b[c]:
                return 1
        return -1
    root = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\Microsoft\Windows Kits\Installed Roots")
    idx = 0
    subs = []
    while True:
        try:
            subs.append(winreg.EnumKey(root, idx))
            idx += 1
        except EnvironmentError:
            break
    return sorted(subs, key=functools.cmp_to_key(cmp_version))[-1]


class VS2022Generator(base_generator.BaseGenerator):
    def __init__(self, collector):
        super(VS2022Generator, self).__init__(collector)
        self._solution_path = os.path.join(global_states.project_root, global_states.project_name + ".vs2022.sln")
        self._intermediate_dir = os.path.join(global_states.build_root, "Intermediate")

    def run(self):
        generated_vcxproj_files = []
        for _, target in self._targets.items():
            vcxproj_file = os.path.join(global_states.build_root, self._get_vcxproj_file_name(target))
            generated_vcxproj_files.append((target, vcxproj_file))
            self._generate_vcxproj_file(target, vcxproj_file)
            self._generate_filter_file(target)
            self._generate_user_file(target)
        self._generate_sln(generated_vcxproj_files)

    def _generate_vcxproj_file(self, target, vcxproj_file_path):
        prj_config = XmlNode("ItemGroup", (), {"Label": "ProjectConfigurations"})
        property_group_configs = []
        import_group_infos = []
        item_definition_group_configs = []
        target_type = target.target_type \
            if target.target_type != build_target.TARGET_TYPE_DEFAULT_LIBRARY \
            else global_states.default_library_type

        config_type = {
            build_target.TARGET_TYPE_NO_BUILD: "Utility",
            build_target.TARGET_TYPE_STATIC_LIBRARY: "StaticLibrary",
            build_target.TARGET_TYPE_DYNAMIC_LIBRARY: "DynamicLibrary",
            build_target.TARGET_TYPE_CONSOLE_APP: "Application",
            build_target.TARGET_TYPE_WINDOW_APP: "Application",
        }[target_type]

        all_deps = self.get_all_dependencies(target)

        for c in Configurations:
            for p in Platforms:
                # ItemGroup Label="ProjectConfigurations"
                prj_config.append(
                    XmlNode(
                        "ProjectConfiguration",
                        (
                            XmlNode("Configuration", c),
                            XmlNode("Platform", p),
                        ),
                        {"Include": "%s|%s" % (c, p)}))
                # PropertyGroup-setting: ConfigurationType, UseDebugLibraries, PlatformToolset...
                is_debug, is_not_debug = ("true", "false") if c == "Debug" else ("false", "true")
                property_group_configs.append(
                    XmlNode(
                        "PropertyGroup",
                        (
                            XmlNode("ConfigurationType", config_type),
                            XmlNode("UseDebugLibraries", is_debug),
                            XmlNode("PlatformToolset", PlatformToolset),
                            XmlNode("WholeProgramOptimization", is_not_debug),
                            XmlNode("CharacterSet", "Unicode"),
                            XmlNode("OutDir", global_states.install_root + "\\"),
                            XmlNode("IntDir", os.path.join("Intermediate", "$(ProjectName)",
                                                           "$(Platform)", "$(Configuration)") + "\\"),
                            XmlNode("TargetName", "%s.%s.%s" % (target.get_name(), c, p))
                        ),
                        {"Condition": "'$(Configuration)|$(Platform)'=='%s|%s'" % (c, p)}))

                # ImportGroup
                d = {"Label": "PropertySheets", "Condition": "'$(Configuration)|$(Platform)'=='%s|%s'" % (c, p)}
                dd = {
                    "Project": r'$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props',
                    "Condition": r"exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')",
                    "Label": "LocalAppDataPlatform",
                }
                import_group_infos.append(XmlNode("ImportGroup", (XmlNode("Import", (), dd),), d))

                # ItemDefinitionGroup
                is_debug = c == "Debug"
                cp = {"Condition": "'$(Configuration)|$(Platform)'=='%s|%s'" % (c, p)}
                macros = self.get_dependent_properties(target, "defines", build_target.PRIVATE_ITEMS) + [
                    "_DEBUG" if is_debug else "NDEBUG",
                    "%(PreprocessorDefinitions)"]

                abs_inc_dirs = self.get_dependent_properties(target, "includes", build_target.PRIVATE_ITEMS)
                additional_include_directories = [os.path.relpath(d, global_states.build_root) for d in abs_inc_dirs]
                additional_include_directories.append("%(AdditionalIncludeDirectories)")

                abs_prebuilt_libs = self.get_dependent_properties(target, "prebuilt_libs", build_target.PRIVATE_ITEMS)
                system_libs = self.get_dependent_properties(target, "system_libs", build_target.PRIVATE_ITEMS)
                additional_link_libs = [os.path.relpath(d, global_states.build_root) for d in abs_prebuilt_libs]
                additional_link_libs += system_libs
                additional_link_libs.append("%(AdditionalDependencies)")

                pch_path = "" if target.pch is None else target.pch[0]
                clcompile = XmlNode("ClCompile", (
                    XmlNode("WarningLevel", "Level4"),
                    XmlNode("Optimization", "Disabled" if is_debug else "MaxSpeed"),
                    XmlNode("SDLCheck", "true"),
                    XmlNode("PreprocessorDefinitions", ";".join(macros)),
                    XmlNode("ConformanceMode", "true"),
                    XmlNode("MultiProcessorCompilation", "true"),
                    XmlNode("AdditionalIncludeDirectories", ";".join(additional_include_directories)),
                    XmlNode("LanguageStandard", "stdcpplatest"),
                    XmlNode("EnableEnhancedInstructionSet", "AdvancedVectorExtensions2"),
                    XmlNode("TreatWarningAsError", "true"),
                    XmlNode("PrecompiledHeader", "Use" if target.pch else "NotUsing"),
                    XmlNode("PrecompiledHeaderFile", pch_path),
                    #XmlNode("ExceptionHandling", "false"),  TODO:make ExceptionHandling a flag, things like GTest still needs it
                ))

                subsystem = {
                    build_target.TARGET_TYPE_CONSOLE_APP: "Console",
                    build_target.TARGET_TYPE_WINDOW_APP: "Window",
                }.get(target_type, "NotSet")

                link = XmlNode("Link", (
                    XmlNode("EnableCOMDATFolding", "false" if is_debug else "true"),
                    XmlNode("OptimizeReferences", "false" if is_debug else "true"),
                    XmlNode("AdditionalDependencies", ";".join(additional_link_libs)),
                    XmlNode("SubSystem", subsystem),
                ))
                item_definition_group_configs.append(XmlNode("ItemDefinitionGroup", (clcompile, link), cp))

        def create_item_group(tag, files):
            root = XmlNode("ItemGroup", ())
            for group_file in files:
                inner_value = ()
                if target.pch is not None and group_file.endswith(target.pch[1]):
                    inner_value = XmlNode("PrecompiledHeader", "Create"), XmlNode("PrecompiledHeaderFile", "")
                root.append(XmlNode(tag, inner_value,
                                    {"Include": os.path.relpath(group_file, global_states.build_root)}))
            return root

        item_group_clcompile = create_item_group("ClCompile", target.files[build_target.FILE_TYPE_SOURCE])
        item_group_clinclude = create_item_group("ClInclude", target.files[build_target.FILE_TYPE_HEADER])
        # TODO: add resources

        nones = [target.build_file_path]
        item_group_none = create_item_group("None", nones)

        item_group_project_reference = XmlNode("ItemGroup", ())
        for d in all_deps:
            if d is target:
                continue
            pnode = XmlNode(
                "ProjectReference",
                content=(XmlNode("Project", self.format_guid(d.guid)),),
                attrib={"Include": self._get_vcxproj_file_name(d)})
            item_group_project_reference.append(pnode)

        sdk_version = get_latest_windows_sdk_version()
        content = VCXPROJ_TEMPLATE.format(
            ProjectConfiguration=prj_config.format(Indent, 1),
            TargetGUID="{%s}" % target.guid,
            TargetName=self.get_target_name(target),
            WindowsTargetPlatformVersion=sdk_version,
            PropertyGroupConfiguration="".join(map(lambda x: x.format(Indent, 1), property_group_configs)),
            ImportGroupPropertySheet="".join(map(lambda x: x.format(Indent, 1), import_group_infos)),
            ItemDefinitionGroup="".join(map(lambda x: x.format(Indent, 1), item_definition_group_configs)),
            ItemGroupCLCompile=item_group_clcompile.format(Indent, 1),
            ItemGroupCLInclude=item_group_clinclude.format(Indent, 1),
            ItemGroupProjectReference=item_group_project_reference.format(Indent, 1),
            ItemGroupNone=item_group_none.format(Indent, 1),
        )
        with open(vcxproj_file_path, "wb") as wf:
            wf.write(content.encode('utf8'))

    def _generate_filter_file(self, target):
        used_filters = set()

        def add_all_parents_for_path(pth, st):
            while pth:
                st.add(pth)
                pth = os.path.dirname(pth)
            return st

        def create_item_group_and_collect_filter(tag, files):
            children = []
            for f in files:
                rp = os.path.dirname(target.get_relative_path(f))
                if rp:
                    add_all_parents_for_path(rp, used_filters)
                children.append(XmlNode(tag, (
                    XmlNode("Filter", rp),
                ), {"Include": os.path.relpath(f, global_states.build_root)}))
            return XmlNode("ItemGroup", children)
        item_group_sources = create_item_group_and_collect_filter("ClCompile", target.files[build_target.FILE_TYPE_SOURCE])
        item_group_filters = create_item_group_and_collect_filter("ClInclude", target.files[build_target.FILE_TYPE_HEADER])

        def create_item_group_filters():
            children = []
            for f in used_filters:
                child = XmlNode("Filter", (
                    XmlNode("UniqueIdentifier", "{%s}" % uuid.uuid5(uuid.NAMESPACE_URL, f)),
                ), {"Include": f})
                children.append(child)
            return XmlNode("ItemGroup", children)
        item_group_includes = create_item_group_filters()
        item_group_build_file = XmlNode("ItemGroup", (
                XmlNode("None", "", {"Include": os.path.relpath(global_states.build_root, target.build_file_path)}),
            ))

        content = FILTER_TEMPLATE.format(
            ItemGroupSources=item_group_sources.format(Indent, 1),
            ItemGroupFilters=item_group_filters.format(Indent, 1),
            ItemGroupIncludes=item_group_includes.format(Indent, 1),
            ItemGroupBuildFile=item_group_build_file.format(Indent, 1),
        )
        filter_file = os.path.join(global_states.build_root, self._get_filter_file_name(target))
        with open(filter_file, "wb") as wf:
            wf.write(content.encode('utf8'))

    def _generate_user_file(self, target):
        def create_item_group_users():
            frags = []
            for config in Configurations:
                for platform in Platforms:
                    cond_str = "'$(Configuration)|$(Platform)'=='%s|%s'" % (config, platform)
                    node = XmlNode(
                        "PropertyGroup",
                        (
                            XmlNode("DebuggerFlavor", "WindowsLocalDebugger"),
                            XmlNode("LocalDebuggerWorkingDirectory", global_states.install_root),
                        ),
                        {"Condition": cond_str},
                    )
                    frags.append(node.format(Indent, 1))
            return "".join(frags)

        content = USER_TEMPLATE.format(
            PropertyGroups=create_item_group_users()
        )
        user_file_name = os.path.join(global_states.build_root, self.get_user_file_name(target))
        with open(user_file_name, "wb") as wf:
            wf.write(content.encode('utf8'))

    def _generate_sln(self, generated_projects):
        # SolutionConfigurationPlatforms
        frags = []
        for c in Configurations:
            for p in Platforms:
                line = "\t\t%s|%s = %s|%s\n" % (c, p, c, p)
                frags.append(line)
        solution_config_platform_block = "".join(frags)

        # ProjectConfigurationPlatforms
        frags = []
        for t in self._targets.values():
            for c in Configurations:
                for p in Platforms:
                    line = "\t\t{%s}.%s|%s.ActiveCfg = %s|%s\n" % (str(t.guid).upper(), c, p, c, p)
                    frags.append(line)
                    line = ("\t\t{%s}.%s|%s.Build.0 = %s|%s\n" % (str(t.guid).upper(), c, p, c, p))
                    frags.append(line)
        project_config_platform_block = "".join(frags)

        # NestedProjects
        path2guid = {}
        nested_projects = []
        for t in self._targets.values():
            if t.group == "":
                t.group = os.path.relpath(os.path.dirname(t.base_dir), global_states.source_root)
                t.group = t.group.replace('/', '\\')
            target_dir_rp = t.group
            assert(target_dir_rp and target_dir_rp != '.')
            while True:
                guid = path2guid.get(target_dir_rp, None)
                if guid is None:
                    guid = uuid.uuid5(uuid.NAMESPACE_URL, target_dir_rp)  # a directory, assign guid for it
                    path2guid[target_dir_rp] = guid
                splits = target_dir_rp.rsplit('\\', 1)
                if len(splits) == 1:
                    break
                target_dir_rp = splits[0]

        for path, guid in path2guid.items():
            splits = path.rsplit('\\', 1)
            if len(splits) == 1:
                continue
            parent_guid = path2guid[splits[0]]
            nested_projects.append("\t\t{%s} = {%s}\n" % (str(guid).upper(), str(parent_guid).upper()))

        for t in self._targets.values():
            parent_guid = path2guid[t.group]
            nested_projects.append("\t\t{%s} = {%s}\n" % (str(t.guid).upper(), str(parent_guid).upper()))

        frags = []
        for path, guid in path2guid.items():
            base_name = path.rsplit('\\')[-1]
            s = 'Project("{%s}") = "%s", "%s", "{%s}"\nEndProject\n' % (
                "2150E333-8FDC-42A3-9474-1A3956D46DE8", base_name, base_name, str(guid).upper())
            frags.append(s)
        sorted_prj_files = sorted(generated_projects, key=lambda x: x[1])
        for target, vcxproj_file in sorted_prj_files:
            s = 'Project("{ProjectType}") = "{TargetName}", "{VCXProjPath}", "{PrjGUID}"\nEndProject\n'.format(
                ProjectType="{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}",
                TargetName=self.get_target_name(target),
                VCXProjPath=os.path.relpath(vcxproj_file, global_states.project_root),
                PrjGUID="{%s}" % str(target.guid).upper()
            )
            frags.append(s)
        project_block = "".join(frags)

        content = SLN_TEMPLATE.format(
            ProjectsBlock=project_block,
            SolutionConfigurationPlatforms=solution_config_platform_block,
            ProjectConfigurationPlatforms=project_config_platform_block,
            NestedProjects="".join(nested_projects),
            SolutionGUID="{%s}" % uuid.uuid5(uuid.NAMESPACE_URL, global_states.project_name),
        )
        with open(self._solution_path, "wb") as wf:
            wf.write(content.encode('utf8'))



    @staticmethod
    def _get_vcxproj_file_name(target):
        return target.get_name() + ".vs2022.vcxproj"

    @staticmethod
    def _get_filter_file_name(target):
        return target.get_name() + ".vs2022.vcxproj.filters"

    @staticmethod
    def get_user_file_name(target):
        return target.get_name() + ".vs2022.vcxproj.user"

    @staticmethod
    def _get_target_name(target):
        return target.get_name() + ".vs2022"

    @staticmethod
    def get_target_name(target):
        return target.get_name() + ".vs2022"
