# -*- encoding: utf-8 -*-
import collections


class XmlNode(object):
    def __init__(self, tag, content=(), attrib=None):
        self.tag = tag
        self.content = content
        if type(content) not in {str, bytes}:
            self.content = list(content)
        self.attrib = attrib if attrib else {}

    def append(self, c):
        self.content.append(c)

    def format(self, indent_char, indent_level):
        ctx = {"level": indent_level, "indent_char": indent_char, "frags": collections.deque(), "newline": "" if indent_char == "" else "\n"}
        self._do_format(ctx)
        return "".join(ctx["frags"])

    def _do_format(self, ctx):
        frags = ctx["frags"]
        level = ctx["level"]
        indent_char = ctx["indent_char"]
        newline = ctx["newline"]

        frags.append(indent_char * level)
        frags.append("<%s" % self.tag)
        for k, v in self.attrib.items():
            frags.append(' %s="%s"' % (k, v))
        frags.append(">")

        if type(self.content) is list:  # with children
            frags.append(newline)
            ctx["level"] += 1
            for c in self.content:
                c._do_format(ctx)
            ctx["level"] -= 1
            frags.append(indent_char * level)
        else:  # only text
            frags.append(self.content)
        frags.append("</%s>" % self.tag)
        frags.append(newline)


if __name__ == "__main__":
    t = XmlNode("a", (
            XmlNode("b0", "b0text"),
            XmlNode("b1", (
                XmlNode("c0", "c0text"),
                XmlNode("c1", "c1text"),
            )),
            XmlNode("b2", "b2text"),
    ), {"attr0": 1, "attr2": 2})
    body = t.format(indent_char=" ", indent_level=2)
    print(body)


