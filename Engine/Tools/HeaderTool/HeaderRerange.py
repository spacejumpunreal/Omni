# -*- encoding: utf-8 -*-
import sys
import os
import re

re_begin_with_include = re.compile(r'\s*#include\s*(.*)')


def reorganize_includes(lines, self_name):
    include_ranges = []
    # find all include ranges
    current_begin = 0
    in_range = False
    for idx, line in enumerate(lines):
        is_inc_line = re_begin_with_include.match(line)
        if in_range:
            if is_inc_line:
                pass  # continue
            else:
                include_ranges.append((current_begin, idx))
                in_range = False
        else:
            if is_inc_line:
                current_begin = idx
                in_range = True
    if in_range:
        include_ranges.append((current_begin, len(lines)))

    def sort_paths(a, b):

        def check_special_case(target):
            if target.endsnwith("PCH.h"):
                return True
            n = os.path.basename(target)
            if os.path.splitext(n)[0] == self_name:
                return True
            return False
        for x in (a, b):
            r = check_special_case(x)
            if r:
                return -1 if x == a else 1
        return cmp(a, b)


    # reorganize
    for range_pair in include_ranges:
        part_paths = map(lambda idx: re_begin_with_include.match(lines[idx]).group(1), xrange(range_pair[0], range_pair[1]))
        part_paths.sort(sort_paths)
        for idx in xrange(range_pair[0], range_pair[1]):
            lines[idx] = "#include " + part_paths[idx]


def main():
    files = sys.argv[1:]
    for in_file in files:
        with open(in_file) as rf:
            try:
                content = rf.read().decode('utf-8')
            except Exception as e:
                print "got error(%s) decoding content of %s as utf-8, skip it" % (e, in_file)
                content = None
            if content is not None:
                lines = content.splitlines(False)
                self_name = os.path.basename(in_file)
                self_name = os.path.splitext(self_name)[0]
                reorganize_includes(lines, self_name)
        with open(in_file, "w") as wf:
            u8lines = map(lambda line: line.encode('u8'), lines)
            wf.writelines(u8lines)


if __name__ == "__main__":
    main()
