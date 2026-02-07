#!/usr/bin/env python3
"""Converts BehaviorTree.CPP V3 compatible tree xml files to V4 format.
"""

import argparse
import copy
import logging
import os
from pathlib import Path
import sys
import typing
import xml.etree.ElementTree as ET

logger = logging.getLogger(__name__)


def strtobool(val: typing.Union[str, int, bool]) -> bool:
    """``distutils.util.strtobool`` equivalent, since it will be deprecated.
    origin: https://stackoverflow.com/a/715468/17094594
    """
    return str(val).lower() in ("yes", "true", "t", "1")


def validate_file_path(file_path: str, mode: str = "r", base_dir: typing.Optional[str] = None) -> typing.TextIO:
    """Validates and sanitizes file paths to prevent directory traversal attacks (CWE-22).
    
    This function implements comprehensive security checks to prevent path traversal
    vulnerabilities. It ensures file paths cannot escape the base directory through
    directory traversal sequences (../, ../../, etc.), symlink attacks, or other
    path manipulation techniques.
    
    Security measures:
    - Input sanitization (null byte detection)
    - Directory traversal pattern detection
    - Path canonicalization using pathlib.Path.resolve()
    - Strict base directory boundary enforcement
    
    Args:
        file_path (str): The file path to validate.
        mode (str): The file open mode ('r' for read, 'w' for write).
        base_dir (typing.Optional[str]): The base directory to restrict access to.
                                         Defaults to current working directory.
    
    Returns:
        typing.TextIO: The opened file object.
    
    Raises:
        ValueError: If the path contains directory traversal sequences or escapes base_dir.
        FileNotFoundError: If the file doesn't exist (read mode).
        PermissionError: If the file can't be accessed.
    """
    # Set base directory to current working directory if not specified
    if base_dir is None:
        base_dir = os.getcwd()
    
    # Security check 1: Detect null bytes (CWE-158 - path truncation attack)
    if "\0" in file_path:
        raise ValueError(
            f"Security violation: Path '{file_path}' contains null bytes"
        )
    
    # Security check 2: Detect directory traversal sequences in raw input (CWE-22)
    # Check before any normalization to catch malicious patterns early
    dangerous_patterns = ["..", "..\\", "../", "..\\\\"]
    for pattern in dangerous_patterns:
        if pattern in file_path:
            raise ValueError(
                f"Security violation: Path '{file_path}' contains directory traversal sequence"
            )
    
    # Security check 3: Use pathlib for secure path resolution
    try:
        # Convert to Path objects
        base_path = Path(base_dir).resolve()
        
        # Resolve the target path (handles symlinks, relative paths, etc.)
        # Construct path relative to base to prevent absolute path injection
        target_path = (base_path / file_path).resolve()
        
        # Security check 4: Verify resolved path is within base directory
        # This is the critical security boundary check
        try:
            # Python 3.9+ has is_relative_to()
            if hasattr(target_path, 'is_relative_to'):
                if not target_path.is_relative_to(base_path):
                    raise ValueError(
                        f"Security violation: Path escapes allowed directory"
                    )
            else:
                # Python < 3.9 fallback using relative_to()
                target_path.relative_to(base_path)
        except ValueError:
            raise ValueError(
                f"Security violation: Path '{file_path}' resolves outside allowed directory"
            )
        
        # All security checks passed, safe to open the file
        return target_path.open(mode)
        
    except (OSError, RuntimeError) as e:
        logger.error(f"Failed to validate or open file '{file_path}': {e}")
        raise ValueError(f"Invalid file path '{file_path}': {e}")


# see ``XMLParser::Pimpl::createNodeFromXML`` for all underscores
SCRIPT_DIRECTIVES = [
    "_successIf",
    "_failureIf",
    "_skipIf",
    "_while",
    "_onSuccess",
    "_onFailure",
    "_onHalted",
    "_post",
]


def convert_single_node(node: ET.Element) -> None:
    """converts a leaf node from V3 to V4.
    Args:
        node (ET.Element): the node to convert.
    """
    if node.tag == "root":
        node.attrib["BTCPP_format"] = "4"

    def convert_no_warn(node_type: str, v3_name: str, v4_name: str):
        if node.tag == v3_name:
            node.tag = v4_name
        elif (
            (node.tag == node_type)
            and ("ID" in node.attrib)
            and (node.attrib["ID"] == v3_name)
        ):
            node.attrib["ID"] = v3_name

    original_attrib = copy.copy(node.attrib)
    convert_no_warn("Control", "SequenceStar", "SequenceWithMemory")

    if node.tag == "SubTree":
        logger.info(
            "SubTree is now deprecated, auto converting to V4 SubTree"
            " (formerly known as SubTreePlus)"
        )
        for key, val in original_attrib.items():
            if key == "__shared_blackboard" and strtobool(val):
                logger.warning(
                    "__shared_blackboard for subtree is deprecated"
                    ", using _autoremap instead."
                    " Some behavior may change!"
                )
                node.attrib.pop(key)
                node.attrib["_autoremap"] = "1"
            elif key == "ID":
                pass
            else:
                node.attrib[key] = f"{{{val}}}"

    elif node.tag == "SubTreePlus":
        node.tag = "SubTree"
        for key, val in original_attrib.items():
            if key == "__autoremap":
                node.attrib.pop(key)
                node.attrib["_autoremap"] = val

    for key in node.attrib:
        if key in SCRIPT_DIRECTIVES:
            logging.error(
                "node %s%s has port %s, this is reserved for scripts in V4."
                " Please edit the node before converting to V4.",
                node.tag,
                f" with ID {node.attrib['ID']}" if "ID" in node.attrib else "",
                key,
            )


def convert_all_nodes(root_node: ET.Element) -> None:
    """recursively converts all nodes inside a root node.
    Args:
        root_node (ET.Element): the root node to start the conversion.
    """

    def recurse(base_node: ET.Element) -> None:
        convert_single_node(base_node)
        for node in base_node:
            recurse(node)

    recurse(root_node)


def convert_stream(in_stream: typing.TextIO, out_stream: typing.TextIO):
    """Converts the behavior tree V3 xml from in_file to V4, and writes to out_file.
    Args:
        in_stream (typing.TextIO): The input file stream.
        out_stream (typing.TextIO): The output file stream.
    """

    class CommentedTreeBuilder(ET.TreeBuilder):
        """Class for preserving comments in xml
        see: https://stackoverflow.com/a/34324359/17094594
        """

        def comment(self, text):
            self.start(ET.Comment, {})
            self.data(text)
            self.end(ET.Comment)

    element_tree = ET.parse(in_stream, ET.XMLParser(target=CommentedTreeBuilder()))
    convert_all_nodes(element_tree.getroot())
    element_tree.write(out_stream, encoding="unicode", xml_declaration=True)


def main():
    """the main function when used in cli mode"""

    logger.addHandler(logging.StreamHandler())
    logger.setLevel(logging.DEBUG)

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "-i",
        "--in_file",
        type=str,
        help="The file to convert from (v3). If absent, reads xml string from stdin.",
    )
    parser.add_argument(
        "-o",
        "--out_file",
        nargs="?",
        type=str,
        default=None,
        help="The file to write the converted xml (V4)."
        " Prints to stdout if not specified.",
    )

    class ArgsType(typing.NamedTuple):
        """Dummy class to provide type hinting to arguments parsed with argparse"""

        in_file: typing.Optional[str]
        out_file: typing.Optional[str]

    args: ArgsType = parser.parse_args()

    # Validate and open input file
    in_stream: typing.TextIO
    if args.in_file is None:
        if not sys.stdin.isatty():
            in_stream = sys.stdin
        else:
            logging.error(
                "The input file was not specified, nor a stdin stream was detected."
            )
            sys.exit(1)
    else:
        try:
            in_stream = validate_file_path(args.in_file, "r")
        except (ValueError, FileNotFoundError, PermissionError) as e:
            logging.error(f"Invalid input file: {e}")
            sys.exit(1)

    # Validate and open output file
    out_stream: typing.TextIO
    if args.out_file is None:
        out_stream = sys.stdout
    else:
        try:
            out_stream = validate_file_path(args.out_file, "w")
        except (ValueError, PermissionError, OSError) as e:
            logging.error(f"Invalid output file: {e}")
            sys.exit(1)

    convert_stream(in_stream, out_stream)


if __name__ == "__main__":
    main()
