#!/usr/bin/python
# Replaces all lexy includes of the input file with the header contents, recursively.
# As an optimization, it does not resolve the same header twice.

import re
import sys
import os

# The location of the lexy header files.
prefix_path = os.path.join(os.path.dirname(__file__), '..', 'include')

# Matches lexy/ and lexy_ext/ includes, group 1 is the file path.
include_pattern = re.compile(r'#\s*include <(lexy/.*\.hpp|lexy_ext/.*\.hpp)>')

already_resolved = []

# Writes the file at the given path to the output.
def cat(output_file, path):
    already_resolved.append(path)

    # Read entire file.
    with open(path) as input_file:
        contents = input_file.read()

    cur_pos = 0
    for match in include_pattern.finditer(contents):
        # Print everything since the last include.
        output_file.write(contents[cur_pos:match.start()])

        # Print the included file.
        included = os.path.join(prefix_path, match.group(1))
        if included not in already_resolved:
            cat(output_file, included)

        # Move past the include.
        cur_pos = match.end()

    # Print everything after the last include
    output_file.write(contents[cur_pos:-1])

#== Main ===#
if len(sys.argv) < 3:
    print(f'usage: {sys.argv[0]} <input> <output>')
    sys.exit(1)

input_path = sys.argv[1]
output_path = sys.argv[2]

with open(output_path, "w") as output_file:
    cat(output_file, input_path)

