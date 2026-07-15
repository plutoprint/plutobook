#!/usr/bin/env python3
# Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Generates a C++ translation unit embedding libhyphen dictionaries.
# Usage: embed_dicts.py <output.cpp> [<locale> <dict-path>]...

import sys


def main(argv):
    output = argv[1]
    pairs = argv[2:]
    if len(pairs) % 2 != 0:
        sys.exit("embed_dicts.py: expected <locale> <path> pairs")

    entries = []
    with open(output, "w", encoding="utf-8") as out:
        out.write('#include "hyphendicts.h"\n\n')
        out.write("namespace plutobook {\n\n")
        for index in range(0, len(pairs), 2):
            name = pairs[index].replace("-", "_")
            with open(pairs[index + 1], "rb") as dict_file:
                data = dict_file.read()
            variable = "kDict{}".format(index // 2)
            out.write("static const unsigned char {}[] = {{".format(variable))
            out.write(",".join(str(byte) for byte in data))
            out.write("};\n")
            entries.append((name, variable, len(data)))

        out.write("\nconst EmbeddedHyphenationDict kEmbeddedHyphenationDicts[] = {\n")
        for name, variable, size in entries:
            out.write('    {{"{}", {}, {}}},\n'.format(name, variable, size))
        # Keep the array non-empty so it is well-formed when no dictionary is embedded.
        if not entries:
            out.write("    {nullptr, nullptr, 0},\n")
        out.write("};\n\n")
        out.write("const unsigned int kEmbeddedHyphenationDictCount = {};\n\n".format(len(entries)))
        out.write("} // namespace plutobook\n")


if __name__ == "__main__":
    main(sys.argv)
