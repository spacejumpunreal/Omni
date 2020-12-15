# -*- encoding: utf-8 -*-
import collections
import os


class DumpContext(object):
    __slots__ = ["indent_level", "ptr_base", "frags", "indent_str", "indent_char", "id2index", "gindex"]

    def init_context(self, string_seed, indent_char="    ", init_level=0):
        self.indent_level = init_level
        state = 0
        for s in string_seed:
            state = ord(s) | (state << 8)
        state = 0xffffffffffffffff & state
        self.ptr_base = "%08X" + ("%016X" % state)
        self.frags = []
        self.indent_str = init_level * indent_char
        self.indent_char = indent_char
        self.id2index = {}
        self.gindex = [0]

    def next(self):
        d = DumpContext()
        d.indent_level = self.indent_level + 1
        d.ptr_base = self.ptr_base
        d.frags = self.frags
        d.indent_str = self.indent_str + self.indent_char
        d.indent_char = self.indent_char
        d.id2index = self.id2index
        d.gindex = self.gindex
        return d

    def get_indent(self):
        return self.indent_str

    def format_ptr(self, obj):
        assert type(obj) is collections.OrderedDict
        name = obj.get("name", None)
        id_obj = id(obj)
        idx = self.id2index.get(id_obj, None)
        if idx is None:
            idx = self.gindex[0]
            self.gindex[0] = idx + 1
            self.id2index[id_obj] = idx
        s = self.ptr_base % idx
        if name is not None:
            s = s + "/*%s*/" % name
        return s


def next_context(context):
    return context[0] + 1, context[1], context[2]  # (indent_level, string_fragments, obj2id)


def quote_string(s):
    return '"%s"' % s


class ObjectRef(object):
    def __init__(self, obj):
        self.object = obj

    def __hash__(self):
        return id(self.object)


def ensure_ref(o):
    if type(o) is not ObjectRef:
        return ObjectRef(o)
    else:
        return o


def build_PBXBuildFile(fileRef):
    return collections.OrderedDict([
        ("isa", "PBXBuildFile"),
        ("fileRef", ensure_ref(fileRef)),
    ])


DEFAULT_BUILD_ACTION_MASK = 0x7fffffff


def build_PBXCopyFilesBuildPhase(buildActionMask=DEFAULT_BUILD_ACTION_MASK, dstPath="", dstSubfolderSpec=16, files=(), runOnlyForDeploymentPostprocessing=0):
    return collections.OrderedDict([
        ("isa", "PBXCopyFilesBuildPhase"),
        ("buildActionMask", buildActionMask),
        ("dstPath", dstPath),
        ("dstSubfolderSpec", dstSubfolderSpec),
        ("files", files),
        ("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing),
    ])

SOURCETREE_GROUP = quote_string("<group>")
SOURCETREE_SOURCE_ROOT = "SOURCE_ROOT"
SOURCETREE_PRODUCTS = "BUILT_PRODUCTS_DIR"

FILE_TYPE_ARCHIVE = 'archive.ar'
FILE_TYPE_APP = 'wrapper.application'
FILE_TYPE_HEADER = 'sourcecode.c.h'
FILE_TYPE_CPP = 'sourcecode.cpp.cpp'
FILE_TYPE_MM = 'sourcecode.cpp.objcpp'
FILE_TYPE_M = 'sourcecode.c.objc'
FILE_TYPE_C = 'sourcecode.c.c'
FILE_TYPE_STORYBOARD = 'file.storyboard'


def build_PBXFileReference(explicitFileType, path, sourceTree=SOURCETREE_GROUP):
    return collections.OrderedDict([
        ("isa", "PBXFileReference"),
        ("explicitFileType", explicitFileType),
        ("path", path),
        ("sourceTree", sourceTree),
    ])


def build_PBXFrameworksBuildPhase(buildActionMask=DEFAULT_BUILD_ACTION_MASK, files=(), runOnlyForDeploymentPostprocessing=0):
    return collections.OrderedDict([
        ("isa", "PBXFrameworksBuildPhase"),
        ("buildActionMask", buildActionMask),
        ("files", map(ensure_ref, files)),
        ("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing),
    ])


def mac_format_path(pth):
    p = pth.replace('\\', '/')
    return p.rstrip('/')


def build_PBXGroup(children, name, sourceTree=SOURCETREE_GROUP, path=None):
    if path is not None:
        path = path.strip()
    d = collections.OrderedDict([
        ("isa", "PBXGroup"),
        ("children", map(ensure_ref, children)),
        ("name", name),
        ("sourceTree", sourceTree),
    ])
    if path is not None and len(path) != 0:
        d["path"] = mac_format_path(path)
    return d


def add_to_PBXGRoup(group, child):
    children = group["children"]
    children.append(ensure_ref(child))


PRODUCT_TYPE_STATIC_LIBRARY = "com.apple.product-type.library.static"
PRODUCT_TYPE_APP = "com.apple.product-type.application"


def build_PBXNativeTarget(buildConfigurationList, buildPhases, buildRules, dependencies, name,
                            productName, productReference, productType):
    return collections.OrderedDict([
        ("isa", "PBXNativeTarget"),
        ("buildConfigurationList", ensure_ref(buildConfigurationList)),
        ("buildPhases", map(ensure_ref, buildPhases)),
        ("buildRules", map(ensure_ref, buildRules)),
        ("dependencies", map(ensure_ref, dependencies)),
        ("name", name),
        ("productName", productName),
        ("productReference", ensure_ref(productReference)),
        ("productType", productType),
    ])


def build_PBXProject(buildConfigurationList, mainGroup, productRefGroup, targets):
    target_attributes = collections.OrderedDict()
    for t in targets:
        target_attributes[ensure_ref(t)] = "{CreatedOnToolsVersion = 11.3.1; }"
    return collections.OrderedDict([
        ("isa", "PBXProject"),
        ("attributes", collections.OrderedDict([
            ("LastUpdateCheck", 1130),
            ("ORGANIZATIONNAME", "qsmdev"),
            ("TargetAttributes", target_attributes),
            ("", 0),
            ("", 0),
        ])),
        ("buildConfigurationList", ensure_ref(buildConfigurationList)),
        ("compatibilityVersion", "Xcode 9.3"),
        ("developmentRegion", "en"),
        ("hasScannedForEncodings", 0),
        ("knownRegions", ["en", "Base"]),
        ("mainGroup", ensure_ref(mainGroup)),
        ("productRefGroup", ensure_ref(productRefGroup)),
        ("projectDirPath", quote_string("")),
        ("projectRoot", quote_string("")),
        ("targets", map(ensure_ref, targets)),
    ])


def build_PBXResourcesBuildPhase(buildActionMask=DEFAULT_BUILD_ACTION_MASK, files=(), runOnlyForDeploymentPostprocessing=0):
    return collections.OrderedDict([
        ("isa", "PBXResourcesBuildPhase"),
        ("buildActionMask", buildActionMask),
        ("files", map(ensure_ref, files)),
        ("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing),
    ])


def build_PBXSourcesBuildPhase(buildActionMask=DEFAULT_BUILD_ACTION_MASK, files=(), runOnlyForDeploymentPostprocessing=0):
    return collections.OrderedDict([
        ("isa", "PBXSourcesBuildPhase"),
        ("buildActionMask", buildActionMask),
        ("files", map(ensure_ref, files)),
        ("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing),
    ])


def build_PBXVariantGroup(children, name, sourceTree=SOURCETREE_GROUP):
    return collections.OrderedDict([
        ("isa", "PBXVariantGroup"),
        ("children", map(ensure_ref, children)),
        ("name", name),
        ("sourceTree", sourceTree)
    ])


def build_XCBuildConfiguration(buildSettings, name):
    return collections.OrderedDict([
        ("isa", "XCBuildConfiguration"),
        ("buildSettings", buildSettings),
        ("name", name),
    ])

#####>>>> these are used for adding code
def parse_dict_body(file):
    lines = None
    with open(file) as rf:
        lines = rf.readlines()
    for line in lines:
        k, v = line.split(' = ')
        print "%s," %((k.strip(), v[:-2]),)
# parse_dict_body

#####<<<< these are used for adding code

DEFAULT_PROJECT_BUILD_SETTING = collections.OrderedDict([
    ('ALWAYS_SEARCH_USER_PATHS', 'NO'),
    ('CLANG_ANALYZER_NONNULL', 'YES'),
    ('CLANG_ANALYZER_NUMBER_OBJECT_CONVERSION', 'YES_AGGRESSIVE'),
    ('CLANG_CXX_LANGUAGE_STANDARD', '"c++17"'),
    ('CLANG_CXX_LIBRARY', '"libc++"'),
    ('CLANG_ENABLE_MODULES', 'YES'),
    ('CLANG_ENABLE_OBJC_ARC', 'YES'),
    ('CLANG_ENABLE_OBJC_WEAK', 'YES'),
    ('CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING', 'YES'),
    ('CLANG_WARN_BOOL_CONVERSION', 'YES'),
    ('CLANG_WARN_COMMA', 'YES'),
    ('CLANG_WARN_CONSTANT_CONVERSION', 'YES'),
    ('CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS', 'YES'),
    ('CLANG_WARN_DIRECT_OBJC_ISA_USAGE', 'YES_ERROR'),
    ('CLANG_WARN_DOCUMENTATION_COMMENTS', 'YES'),
    ('CLANG_WARN_EMPTY_BODY', 'YES'),
    ('CLANG_WARN_ENUM_CONVERSION', 'YES'),
    ('CLANG_WARN_INFINITE_RECURSION', 'YES'),
    ('CLANG_WARN_INT_CONVERSION', 'YES'),
    ('CLANG_WARN_NON_LITERAL_NULL_CONVERSION', 'YES'),
    ('CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF', 'YES'),
    ('CLANG_WARN_OBJC_LITERAL_CONVERSION', 'YES'),
    ('CLANG_WARN_OBJC_ROOT_CLASS', 'YES_ERROR'),
    ('CLANG_WARN_RANGE_LOOP_ANALYSIS', 'YES'),
    ('CLANG_WARN_STRICT_PROTOTYPES', 'YES'),
    ('CLANG_WARN_SUSPICIOUS_MOVE', 'YES'),
    ('CLANG_WARN_UNGUARDED_AVAILABILITY', 'YES_AGGRESSIVE'),
    ('CLANG_WARN_UNREACHABLE_CODE', 'YES'),
    ('CLANG_WARN__DUPLICATE_METHOD_MATCH', 'YES'),
    ('COPY_PHASE_STRIP', 'YES'),
    ('ENABLE_STRICT_OBJC_MSGSEND', 'YES'),
    ('GCC_C_LANGUAGE_STANDARD', 'gnu11'),
    ('GCC_NO_COMMON_BLOCKS', 'YES'),
    ('GCC_WARN_64_TO_32_BIT_CONVERSION', 'YES'),
    ('GCC_WARN_ABOUT_RETURN_TYPE', 'YES_ERROR'),
    ('GCC_WARN_UNDECLARED_SELECTOR', 'YES'),
    ('GCC_WARN_UNINITIALIZED_AUTOS', 'YES_AGGRESSIVE'),
    ('GCC_WARN_UNUSED_FUNCTION', 'YES'),
    ('GCC_WARN_UNUSED_VARIABLE', 'YES'),
    ('IPHONEOS_DEPLOYMENT_TARGET', '13.2'),
    ('ONLY_ACTIVE_ARCH', 'YES'),
    ('MTL_ENABLE_DEBUG_INFO', 'YES'),
    ('MTL_FAST_MATH', 'YES'),
    ('SDKROOT', 'iphoneos'),
    ('ONLY_ACTIVE_ARCH', 'YES'),
    ('VALIDATE_PRODUCT', 'YES'),
])

DEBUG_PROJECT_EXTRA_BUILD_SETTING = collections.OrderedDict([
    ('DEBUG_INFORMATION_FORMAT', 'dwarf'),
    ('ENABLE_TESTABILITY', 'YES'),
    ('GCC_DYNAMIC_NO_PIC', 'NO'),
    ('GCC_OPTIMIZATION_LEVEL', 0),
    ('GCC_PREPROCESSOR_DEFINITIONS', '("DEBUG=1", "$(inherited)",)')
])

RELEASE_PROJECT_EXTRA_BUILD_SETTING = collections.OrderedDict([
    ('DEBUG_INFORMATION_FORMAT', '"dwarf-with-dsym"'),
    ('ENABLE_NS_ASSERTIONS', 'NO'),
])

DEFAULT_TARGET_BUILD_SETTING = collections.OrderedDict([
    ('CODE_SIGN_STYLE', 'Automatic'),
    ('DEVELOPMENT_TEAM', '8K6L6A7M7P'),
    ('GCC_ENABLE_CPP_EXCEPTIONS', 'NO;'),
    ('HEADER_SEARCH_PATHS', []),
    ('PRODUCT_NAME', '$(TARGET_NAME)'),
    ('TARGETED_DEVICE_FAMILY', '"1,2"'),
])

LIBRARY_EXTRA_BUILD_SETTING = collections.OrderedDict([

    ('OTHER_LDFLAGS', '"-ObjC"'),
    ('PRODUCT_NAME', '"$(TARGET_NAME)"'),
    ('SKIP_INSTALL', 'YES'),
    ('TARGETED_DEVICE_FAMILY', '"1,2'),
])

APP_EXTRA_BUILD_SETTING = collections.OrderedDict([
    ("ASSETCATALOG_COMPILER_APPICON_NAME", "AppIcon"),
    ("CLANG_CXX_LANGUAGE_STANDARD", "c++17"),
    ('INFOPLIST_FILE', '""'),
    ('LD_RUNPATH_SEARCH_PATHS', []),
    ('PRODUCT_BUNDLE_IDENTIFIER', ""),
])


def build_XCConfigurationList(buildConfigurations, defaultConfigurationName):
    return collections.OrderedDict([
        ("isa", "XCConfigurationList"),
        ("buildConfigurations", map(ensure_ref, buildConfigurations)),
        ("defaultConfigurationIsVisible", 0),
        ("defaultConfigurationName", defaultConfigurationName),
    ])


def cmp_struct(left, right):
    ret = cmp(left["isa"], right["isa"])
    if ret != 0:
        return ret
    return int(id(left) - id(right))


atomic_types = {int, str}


def do_dump(ctx, record):
    record_type = type(record)
    if record_type == collections.OrderedDict:
        ctx.frags.append('{\n')
        nctx = ctx.next()
        for key, value in record.iteritems():
            indent_str = nctx.indent_str
            nctx.frags.append(indent_str)
            do_dump(ctx, key)
            nctx.frags.append(" = ")
            do_dump(nctx, value)
            nctx.frags.append(';\n')
        ctx.frags.append(ctx.indent_str)
        ctx.frags.append('}')
    elif record_type is list or record_type is tuple:
        ctx.frags.append('(\n')
        nctx = ctx.next()
        for v in record:
            indent_str = nctx.indent_str
            nctx.frags.append(indent_str)
            do_dump(nctx, v)
            ctx.frags.append(',\n')
        ctx.frags.append(ctx.indent_str)
        ctx.frags.append(')')
    elif record_type is ObjectRef:
        ctx.frags.append(ctx.format_ptr(record.object))
    elif record_type in atomic_types:
        ctx.frags.append(str(record))
    else:
        raise NotImplementedError


def dump_pbxproj(objects, str_seed, project_ref):
    ctx = DumpContext()
    ctx.init_context(str_seed)  # we can just use python builtin id
    objects.sort(cmp_struct)
    if project_ref is None:
        for obj in objects:
            if obj["isa"] == "PBXProject":
                root_obj = obj
        if project_ref is None:
            raise AssertionError("root object with PBXProject not specified in argument(objects)")

    ctx.frags.append("// !$*UTF8*$!\n")
    top_record = collections.OrderedDict([
        ("archiveVersion", 1),
        ("objectVersion", 50),
        ("objects", collections.OrderedDict(map(lambda x: (ctx.format_ptr(x), x), objects))),
        ("rootObject", ensure_ref(project_ref))
    ])
    do_dump(ctx, top_record)
    return "".join(ctx.frags)


def get_build_setting(is_debug, is_app, is_project, update_kvs=None):
    if is_project:
        ret = DEFAULT_PROJECT_BUILD_SETTING.copy()
        update_dict = DEBUG_PROJECT_EXTRA_BUILD_SETTING if is_debug else RELEASE_PROJECT_EXTRA_BUILD_SETTING
    else:
        ret = DEFAULT_TARGET_BUILD_SETTING.copy()
        update_dict = APP_EXTRA_BUILD_SETTING if is_app else LIBRARY_EXTRA_BUILD_SETTING
    for k, v in update_dict.iteritems():
        ret[k] = v
    if update_kvs is not None:
        for k, v in update_kvs:
            ret[k] = v
    return ret


ext2type = {
    'cpp': FILE_TYPE_CPP,
    'mm': FILE_TYPE_MM,
    'm': FILE_TYPE_M,
    'c': FILE_TYPE_C,
    'h': FILE_TYPE_HEADER,
    'a': FILE_TYPE_ARCHIVE,
    'storyboard': FILE_TYPE_STORYBOARD,
}


def guess_file_type(file_name):
    ext = file_name.split('.')[-1]
    return ext2type[ext]


def _test_main():
    libAStaticLibrary_a = build_PBXFileReference(FILE_TYPE_ARCHIVE, "libAStaticLibrary.a")
    AStaticLibrary_h = build_PBXFileReference(FILE_TYPE_HEADER, "AStaticLibrary.h")
    AStaticLibrary_cpp = build_PBXFileReference(FILE_TYPE_CPP, "AStaticLibrary.cpp")
    buildfile = build_PBXBuildFile(AStaticLibrary_cpp)
    copyfilephase = build_PBXCopyFilesBuildPhase(dstPath=quote_string(r"include/$(PRODUCT_NAME)"))
    frameworkbuildphase = build_PBXFrameworksBuildPhase()
    sourcephase = build_PBXSourcesBuildPhase(files=(AStaticLibrary_cpp,))
    resourcebuildpahse = build_PBXResourcesBuildPhase()
    group_Products = build_PBXGroup([libAStaticLibrary_a], "Products")
    group_AStaticLibrary = build_PBXGroup(
        [AStaticLibrary_h, AStaticLibrary_cpp], "AStaticLibrary", path="../Source/AStaticLibrary")
    group_top = build_PBXGroup([group_AStaticLibrary, group_Products], "top")

    target_debug = get_build_setting(True, False, False)
    target_release = get_build_setting(False, False, False)

    project_debug = get_build_setting(True, False, True)
    project_release = get_build_setting(False, False, True)

    target_config = build_XCConfigurationList(([target_debug, target_release]), "Release")
    project_config = build_XCConfigurationList([project_debug, project_release], "Release")

    nativetarget = build_PBXNativeTarget(
        buildConfigurationList=[target_config],
        buildPhases=[sourcephase, copyfilephase, frameworkbuildphase, resourcebuildpahse],
        buildRules=(),
        dependencies=(),
        name="AStaticlibrary",
        productName="AStaticlibrary",
        productReference=libAStaticLibrary_a,
        productType=PRODUCT_TYPE_STATIC_LIBRARY,
    )
    project = build_PBXProject(
        buildConfigurationList=[project_config],
        mainGroup=group_top,
        productRefGroup=group_Products,
        targets=(nativetarget,),
    )
    objects = [
        libAStaticLibrary_a, AStaticLibrary_h, AStaticLibrary_cpp, buildfile, copyfilephase,
        frameworkbuildphase, group_top, group_Products, group_AStaticLibrary, nativetarget,
        project,
    ]
    txt = dump_pbxproj(objects, 'TEST_PRJ')
    with open("TestFile.pbxproj", "wb") as wf:
        wf.write(txt)


if __name__ == "__main__":
    _test_main()

