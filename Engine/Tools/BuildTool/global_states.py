# -*- encoding: utf-8 -*-
import os
import platform_utils
import assert_utils
import path_utils


project_name = ""
source_root = ""
build_root = ""
install_root = ""
project_root = ""
target_platform = platform_utils.PLATFORM_UNKNOWN
build_tool_res_path = ""
collector = None
default_library_type = ""


def setup(project_name_, source_root_, build_root_, project_root_, install_root_, force_platform_, default_library_type_):
    global project_name
    global source_root
    global build_root
    global install_root
    global project_root
    global target_platform
    global build_tool_res_path
    global default_library_type

    project_name = project_name_
    source_root = source_root_
    build_root = build_root_
    install_root = install_root_
    project_root = project_root_
    target_platform = platform_utils.calc_target_platform(force_platform_)
    default_library_type = default_library_type_
    build_tool_dir = os.path.dirname(os.path.abspath(__file__))
    build_tool_res_path = os.path.join(build_tool_dir, "ToolResources")

    assert_utils.check_path_exists(source_root)
    path_utils.ensure_path(build_root)
    path_utils.ensure_path(install_root)

