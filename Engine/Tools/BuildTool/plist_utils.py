# -*- encoding: utf-8 -*-
import os
import collections
import xml.etree.ElementTree as ET

RESOURCE_PATH = os.path.join(os.path.dirname((os.path.abspath(__file__))), "ToolResources")


def load_ios_app_info_plist_template():
    path = os.path.join(RESOURCE_PATH, "XcodeAppTargetInfo.plist")
    tree = ET.parse(path)
    return tree


INFO_PLIST_HEADER = """<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
"""


def dump_to_file(path, tree):
    with open(path, "wb") as wf:
        lines = ET.tostringlist(tree.getroot(), encoding='UTF-8')
        lines.insert(1, INFO_PLIST_HEADER)
        wf.writelines(lines)


def test_main():
    test_tree = load_ios_app_info_plist_template()
    root = test_tree.getroot()
    armv7 = root.find("dict/array/string")
    armv7.text = "arm64"
    dump_to_file(r"c:/temp/newInfo.plist", test_tree)


if __name__ == "__main__":
    test_main()
