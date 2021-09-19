# -*- encoding: utf-8 -*-
import sys
import os

sys.path.append(os.path.abspath("Engine/Tools/BuildTool"))  # add search path for BuildTool so we can import driver

import build_tool
import build_target
import global_states


def main():
    pwd = os.path.dirname(os.path.abspath(__file__))
    source_root = os.path.join(pwd, "Engine/Source")
    build_root = os.path.join(pwd, "Build")
    install_root = os.path.join(pwd, "Engine/Binaries")
    force_platform = None
    project_name = "Omni"
    default_library_type = build_target.TARGET_TYPE_STATIC_LIBRARY
    global_states.setup(project_name, source_root, build_root, pwd, install_root, force_platform, default_library_type)
    build_tool.generate_project()


if __name__ == "__main__":
    main()
