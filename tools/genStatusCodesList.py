#!/usr/bin/env python3

import sys

status_codes_hdr = open(f"{sys.argv[1]}", 'r').read().strip()
status_codes_csv = open(f"{sys.argv[2]}/StatusCodesList.csv", 'w')

# Find the start of the status codes definition
def_start = status_codes_hdr.find("define HMDT_STATUS_CODES() \\\n")

if def_start == -1:
    print("ERROR: Unable to find HMDT_STATUS_CODES definition!", file=sys.stderr)
    sys.exit(1)

# Find the end of the status codes definition, which will be the first blank
#   line after the start of the status codes
def_end = status_codes_hdr.find("\n\n", def_start)

if def_end == -1:
    print("ERROR: Unable to find end of HMDT_STATUS_CODES definition!", file=sys.stderr)
    sys.exit(1)

status_codes = status_codes_hdr[def_start:def_end].strip().split('\n')[1:]

def getParensContents(line):
    pstart = line.find("(")

    if pstart == -1:
        print(f"ERROR: Failed to find '(' on line '{line}'")
        sys.exit(1)

    pend = line.find(")", pstart)
    if pend == -1:
        print(f"ERROR: Failed to find '(' on line '{line}'")
        sys.exit(1)

    return line[pstart+1:pend]

group = ""
value = 0

# [ (code, symbol, description, grouping) ]
status_codes_list = []

for code_def in status_codes:
    code_def = code_def.strip()

    if code_def.startswith("X("):
        symbol, description = getParensContents(code_def).split(',')
        status_codes_list.append( (value, symbol.strip(), description.strip()[1:-1], group) )
        value += 1
    elif code_def.startswith("Y("):
        symbol, base_value = getParensContents(code_def).split(',')

        group = symbol.strip()
        value = int(base_value.strip(), base=16) + 1

    # Ignore all other cases, as those would be either blank lines or comments

# Generate the CSV file
status_codes_csv.write('\n'.join([ ','.join([str(v) for v in s]) for s in status_codes_list]))

