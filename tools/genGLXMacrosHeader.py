#!/usr/bin/env python3

import os
import sys
import datetime

def textFinder(file_contents, line_prefix):
    for line in file_contents.split("\n"):
        if line.startswith(line_prefix):
            yield line

glxmacros_hdr = open(f"{sys.argv[2]}/GLXMacros.h", 'w')

mingw_inc_path = os.path.normpath(sys.argv[1])
gl_inc_path = os.path.normpath(os.path.join(mingw_inc_path, "GL"))
glew_path = os.path.normpath(os.path.join(gl_inc_path, "glew.h")) # Normalize the path to remove mixed slashes

# Verify that the path exists
if not os.path.exists(glew_path):
    print(f"ERROR: Path {glew_path} does not exist.")
    if not os.path.exists(gl_inc_path):
        print(f"ERROR: Path {gl_inc_path} does not exist.")
    if not os.path.exists(mingw_inc_path):
        print(f"ERROR: Path {mingw_inc_path} does not exist.")
    sys.exit(1)

glew_source = open(glew_path, 'r').read()

glenum_define_strings = textFinder(glew_source, "#define GL_")
glenum_defines = { } # VALUE => [NAMES]

for def_string in glenum_define_strings:
    # (#define, NAME, VALUE)
    define = def_string.split(' ')

    name = define[1]

    # Skip the VERSION defines
    if name.startswith("GL_VERSION_"): continue

    # Handle the case of #define NAME OTHER_NAME
    try:
        value = int(define[2], 0)
    except ValueError as e:
        print(f"Failed to parse \"{def_string}\". Skipping.")
        continue

    if value in glenum_defines:
        glenum_defines[value].append(name)
    else:
        glenum_defines[value] = [name]

glxmacros_hdr.write( "/**\n")
glxmacros_hdr.write( " * @file GLXMacros.h\n")
glxmacros_hdr.write( " *\n")
glxmacros_hdr.write(f" * @brief This file was autogenerated on {datetime.datetime.now()}\n")
glxmacros_hdr.write( " *        manual changes will not be tracked.\n")
glxmacros_hdr.write( " *\n")
glxmacros_hdr.write( " */\n\n")

glxmacros_hdr.write( "#ifndef GL_XMACROS_H\n")
glxmacros_hdr.write( "# define GL_XMACROS_H\n\n")

glxmacros_hdr.write( "# define HMDT_GL_XMACRO")

for value, names in glenum_defines.items():
    name = names[0]
    duplicates = ", ".join([f"\"{n}\"" for n in names[1:]])
    glxmacros_hdr.write(f"\\\n    X({name}, {value}, {duplicates})")
glxmacros_hdr.write("\n")

glxmacros_hdr.write( "\n#endif\n")

