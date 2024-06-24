#!/usr/bin/python
# Generate the header file containing the Unicode database.
# It uses the three stage approach described e.g. here https://here-be-braces.com/fast-lookup-of-unicode-properties/.
# For case folding, it stores the offset to the simple case folded code point.
# This compresses very well.
# The generated arrays themselves are written as string literals, which are more efficiently represented in compilers.

UNICODE_VERSION = '14.0.0'
UCD_URL = f'https://www.unicode.org/Public/{UNICODE_VERSION}/ucd/'

#=== Unicode Database ===#
import urllib.request
import csv
import re

comment_pattern = re.compile(r'\s*#.*$')
range_pattern   = re.compile(r'([0-9A-F]+)\.\.([0-9A-F]+)')

class CodePointProperties:
    def __init__(self):
        self.general_category = 'Cn'

        self.case_folding = 0

        self.is_whitespace   = False
        self.is_join_control = False
        self.is_alphabetic   = False
        self.is_uppercase    = False
        self.is_lowercase    = False
        self.is_xid_start    = False
        self.is_xid_continue = False

    def set_general_category(self, category):
        self.general_category = category

    def set_core_prop(self, prop):
        if prop == 'White_Space':
            self.is_whitespace = True
        elif prop == 'Join_Control':
            self.is_join_control = True

    def set_derived_prop(self, prop):
        if prop == 'Alphabetic':
            self.is_alphabetic = True
        elif prop == 'Uppercase':
            self.is_uppercase = True
        elif prop == 'Lowercase':
            self.is_lowercase = True
        elif prop == 'XID_Start':
            self.is_xid_start = True
        elif prop == 'XID_Continue':
            self.is_xid_continue = True

    def dict_key(self):
        return (self.general_category, self.case_folding, self.is_whitespace, self.is_join_control,
                self.is_alphabetic, self.is_uppercase, self.is_lowercase, self.is_xid_start, self.is_xid_continue)

def fetch_ucd_file(path):
    return (line.decode('utf-8') for line in urllib.request.urlopen(UCD_URL + path))

def fetch_ucd_data(path):
    def skip_comments(file):
        for line in file:
            line = re.sub(comment_pattern, '', line).strip()
            if line:
                yield line
    return skip_comments(fetch_ucd_file(path))

def set_database_value(database, path, setter):
    data = fetch_ucd_data(path)
    for row in csv.reader(data, delimiter=';'):
        value = row[1].strip()

        if match := range_pattern.match(row[0]):
            first = int(match.group(1), 16)
            last  = int(match.group(2), 16)

            for cp in range(first, last + 1):
                setter(database[cp], value)
        else:
            cp = int(row[0], 16)
            setter(database[cp], value)

def set_database_case_folding(database):
    data = fetch_ucd_data('CaseFolding.txt')
    for row in csv.reader(data, delimiter=';'):
        cp = int(row[0], 16)
        status = row[1].strip()
        if status == 'C' or status == 'S':
            mapping = int(row[2], 16)
            database[cp].case_folding = mapping - cp

def unicode_database():
    database = [CodePointProperties() for cp in range(0, 0x10FFFF + 1)]

    set_database_value(database, 'extracted/DerivedGeneralCategory.txt', CodePointProperties.set_general_category)
    set_database_value(database, 'PropList.txt', CodePointProperties.set_core_prop)
    set_database_value(database, 'DerivedCoreProperties.txt', CodePointProperties.set_derived_prop)
    set_database_case_folding(database)

    return database

#=== Lookup tables ===#
class LookupTables:
    BLOCK_SIZE = 256

    def __init__(self):
        self.block_starts = [] # Stage 1
        self.blocks       = [] # Stage 2
        self.properties   = [] # Stage 3

def build_lookup_tables(database):
    result = LookupTables()
    # Maps CodePointProperties to their index in the stage 3 table.
    property_dict = {}
    # Maps blocks to their index in the stage 2 table.
    blocks_dict = {}

    cur_block = []
    for cp, properties in enumerate(database):
        # Get index of that particular combination of properties.
        prop_key   = properties.dict_key()
        prop_index = property_dict.get(prop_key)
        if prop_index is None:
            prop_index = len(result.properties)
            result.properties.append(properties)
            property_dict[prop_key] = prop_index

        # Add that index to the current block.
        cur_block.append(prop_index)

        # Flush the block if necessary.
        if len(cur_block) == LookupTables.BLOCK_SIZE or cp == 0x10FFFF:
            # For that, determine the index of the first entry in the block.
            block_key = tuple(cur_block)
            block_index = blocks_dict.get(block_key)
            if block_index is None:
                block_index = len(result.blocks) // LookupTables.BLOCK_SIZE
                result.blocks.extend(cur_block)
                blocks_dict[block_key] = block_index

            # ... and append it to the stage 1 table.
            result.block_starts.append(block_index)
            cur_block = []

    return result

def estimate_table_size(tables):
    # one byte each
    block_starts = len(tables.block_starts)
    blocks = len(tables.blocks)
    assert max(tables.block_starts) < 2**8
    assert max(tables.blocks) < 2**8

    # general category is an int
    category = len(tables.properties) * 4
    # binary properties is one byte each
    binary_properties = len(tables.properties)
    # case folding is four bytes each
    case_folding = len(tables.properties) * 4

    return block_starts + blocks + category + binary_properties + case_folding

def generate_lookup_tables(tables):
    def int_array(ints):
        yield '"'
        for idx, value in enumerate(ints):
            if idx > 0 and idx % 1024 == 0:
                yield '"\n    "'

            char = chr(value)
            if char == '"':
                yield r'\"'
            elif char == '\\':
                yield r'\\'
            elif char.isprintable():
                yield char
            else:
                yield f'\\{value:03o}'
        yield '"'

    def lookup_function():
        yield 'constexpr std::size_t property_index(char32_t code_point)\n'
        yield '{\n'
        yield '    LEXY_PRECONDITION(code_point <= 0x10FFFF);\n'
        yield '    auto high = (code_point >> 8) & 0xFFFF;\n'
        yield '    auto low  = code_point & 0xFF;\n'
        yield '\n'
        yield '    auto block_start = static_cast<unsigned char>(block_starts[high]);\n'
        yield f'    auto block_index = block_start * {LookupTables.BLOCK_SIZE}u + low;\n'
        yield '    return static_cast<unsigned char>(blocks[block_index]);\n'
        yield '}\n'

    def category_array(properties):
        yield '{'
        for prop in properties:
            yield f'lexy::code_point::{prop.general_category},'
        yield '}'

    def bprop_enum():
        yield 'enum binary_properties_t\n'
        yield '{\n'
        yield '    whitespace,\n'
        yield '    join_control,\n'
        yield '    alphabetic,\n'
        yield '    uppercase,\n'
        yield '    lowercase,\n'
        yield '    xid_start,\n'
        yield '    xid_continue,\n'
        yield '    _property_count,\n'
        yield '};\n'
        yield 'static_assert(static_cast<int>(_property_count) <= 8);'

    def bprop_array(properties):
        yield '{'
        for prop in properties:
            mask = 0
            if prop.is_whitespace:
                mask |= 1 << 0
            if prop.is_join_control:
                mask |= 1 << 1
            if prop.is_alphabetic:
                mask |= 1 << 2
            if prop.is_uppercase:
                mask |= 1 << 3
            if prop.is_lowercase:
                mask |= 1 << 4
            if prop.is_xid_start:
                mask |= 1 << 5
            if prop.is_xid_continue:
                mask |= 1 << 6
            yield f'{mask},'
        yield '}'

    def case_fold_array(properties):
        yield '{'
        for prop in properties:
            yield f'{prop.case_folding},'
        yield '}'

    assert max(tables.block_starts) < 2**8
    assert max(tables.blocks) < 2**8

    yield '// AUTOGENERATED FILE --- DO NOT EDIT!\n'
    yield '// Generated by `support/generate-unicode-db.py`.\n'
    yield '\n'
    yield f'#define LEXY_UNICODE_DATABASE_VERSION "{UNICODE_VERSION}"\n'
    yield '\n'

    yield 'namespace lexy::_unicode_db\n'
    yield '{\n'

    yield 'constexpr const char* block_starts = '
    yield from int_array(tables.block_starts)
    yield ';\n'

    yield 'constexpr const char* blocks = '
    yield from int_array(tables.blocks)
    yield ';\n'

    yield '\n'
    yield from lookup_function()
    yield '\n'

    yield 'constexpr lexy::code_point::general_category_t category[] = '
    yield from category_array(tables.properties)
    yield ';\n'

    yield '\n'
    yield from bprop_enum()
    yield '\n'

    yield '\n'
    yield 'constexpr std::uint_least8_t binary_properties[] = '
    yield from bprop_array(tables.properties)
    yield ';\n'

    yield '\n'
    yield 'constexpr std::int_least32_t case_folding_offset[] = '
    yield from case_fold_array(tables.properties)
    yield ';\n'

    yield '} // namespace lexy::_unicode_db'

#=== main ===#
import sys

if len(sys.argv) < 2:
    print(f'usage: {sys.argv[0]} <output-file>')
    sys.exit(1)

output_path = sys.argv[1]

database = unicode_database()
tables = build_lookup_tables(database)

print(f"block_starts length: {len(tables.block_starts)}")
print(f"blocks length: {len(tables.blocks)}")
print(f"properties length: {len(tables.properties)}")
print(f"total size estimate: {estimate_table_size(tables)} bytes")

with open(output_path, "w") as output_file:
    print(''.join(generate_lookup_tables(tables)), file=output_file)

