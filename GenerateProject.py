# -*- encoding: utf-8 -*-
import sys
import os

sys.path.append(os.path.abspath("Engine/Tools/BuildTool"))  # add search path for BuildTool so we can import driver

if __name__ == "__main__":
    pwd = os.path.dirname(os.path.abspath(__file__))
    source_root = os.path.join(pwd, "Engine/Source")
    build_root = os.path.join(pwd, "Engine/Build")
    import build_tool
    target_platform = ""
    if len(sys.argv) < 2:
        # guess the platform
        target_platform = sys.platform
    else:
        target_platform = sys.argv[1]

    if target_platform == 'win32':
        solution_path = os.path.join(pwd, "Omni.2019.sln")
        build_tool.generate_vs_project(source_root, pwd, build_root, solution_path)
    elif target_platform == 'darwin':
        solution_path = os.path.join(pwd, "Omni.xcworkspace")
        build_tool.generate_ios_project(source_root, pwd, build_root, solution_path)