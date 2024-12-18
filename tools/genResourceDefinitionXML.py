#!/usr/bin/env python3

# Args: [0]=Name, [1]=PREFIX, [2]=OutFile, [3]=ResourcesDir

# from io import BytesIO

import sys, os
import datetime
import xml.etree.ElementTree as ET
from xml.dom import minidom

name = sys.argv[1]
in_prefix = sys.argv[2]
out_dir = sys.argv[3]
res_dir = sys.argv[4]

print("Name=", name)
print("Prefix=", in_prefix)
print("OutDir=", out_dir)
print("ResDir=", res_dir)

# Generate XML in following format:
# <?xml version="1.0" encoding="UTF-8"?>
# <gresources>
#     <gresource prefix="{PREFIX}">
#         <file>{FILE1}</file>
#         <file>{FILE2}</file>
#         ...
#     </gresource>
# </gresources>

gresources = ET.Element("gresources")
gresource = ET.SubElement(gresources, "gresource", prefix=in_prefix)

print("Walk ", res_dir)
for root, dir, files in os.walk(res_dir):
    # path = root.split(os.sep)
    # print(root, ": ", len(files), " files")
    rel_path = os.path.relpath(root, out_dir)
    print(out_dir, "->", root, "=", rel_path)
    for file_name in files:
        f = ET.SubElement(gresource, "file").text = os.path.join(rel_path, file_name)

print("Write file ", out_dir)

out_file = os.path.join(out_dir, name + "ResourceDefinition.gresource.xml")

print("Writing to ", out_file)
with open(out_file, 'wb') as out:
    tree = ET.ElementTree(gresources)

    # Write header comment denoting that this file was auto-generated
    out.write(f"<!-- This file was autogenerated on {datetime.datetime.now()}, manual changes will not be tracked. -->\n".encode('utf-8'))
    # Write out the XML file. This horrible line is so we can get pretty-printing,
    #   see https://code.whatever.social/questions/49990435/python-xml-pretty-printing-elementtree
    out.write(minidom.parseString(ET.tostring(tree.getroot(), encoding='utf-8', xml_declaration=True)).toprettyxml(indent="  ", encoding='utf-8'))

