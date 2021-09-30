# -*- encoding: utf-8 -*-
import sys
import os
import re

re_begin_with_include = re.compile(r'\s*#include\s*(.*)')


def convert_includes(lines, self_name, mp):
    for line_idx, line in enumerate(lines):
        r = re_begin_with_include.match(line)
        if r is not None:
            pth = r.group(1)
            if pth.startswith('"'):
                pth = pth[1:-1]
                base_name = os.path.basename(pth)
                if base_name in mp:
                    lines[line_idx] = '#include "%s"' % mp[base_name]
                else:
                    print "found unknown include at line:%d of %s, can't find included file:%s" % \
                          (line_idx, self_name, base_name)
                    assert(False)


def main():
    root_dir = os.path.normpath(sys.argv[1])
    mp = {}
    for root, dirs, files in os.walk(root_dir):
        for src in files:
            fp = os.path.join(root, src)
            rp = os.path.relpath(fp, root_dir)
            mp[src] = rp.replace('\\', '/')
    files = sys.argv[2:]
    for in_file in files:
        with open(in_file) as rf:
            try:
                content = rf.read().decode('utf-8')
            except Exception as e:
                print "got error(%s) decoding content of %s as utf-8, skip it" % (e, in_file)
                content = None
            if content is not None:
                lines = content.splitlines(False)
                convert_includes(lines, in_file, mp)
        with open(in_file, "w") as wf:
            u8lines = map(lambda line: line.encode('u8') + "\n", lines)
            wf.writelines(u8lines)


if __name__ == "__main__":
    main()
