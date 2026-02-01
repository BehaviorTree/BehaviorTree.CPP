#!/usr/bin/env python3
"""Converts BehaviorTree.CPP V3 compatible tree xml files to V4 format.
"""

import argparse
import copy
import logging
import sys
import typing
import xml.etree.ElementTree as ET

logger = logging.getLogger(__name__)


def strtobool(val: typing.Union[str, int, bool]) -> bool:
    """``distutils.util.strtobool`` equivalent, since it will be deprecated.
    origin: https://stackoverflow.com/a/715468/17094594
    """
    return str(val).lower() in ("yes", "true", "t", "1")


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
        type=argparse.FileType("r"),
        help="The file to convert from (v3). If absent, reads xml string from stdin.",
    )
    parser.add_argument(
        "-o",
        "--out_file",
        nargs="?",
        type=argparse.FileType("w"),
        default=sys.stdout,
        help="The file to write the converted xml (V4)."
        " Prints to stdout if not specified.",
    )

    class ArgsType(typing.NamedTuple):
        """Dummy class to provide type hinting to arguments parsed with argparse"""

        in_file: typing.Optional[typing.TextIO]
        out_file: typing.TextIO

    args: ArgsType = parser.parse_args()

    if args.in_file is None:
        if not sys.stdin.isatty():
            args.in_file = sys.stdin
        else:
            logging.error(
                "The input file was not specified, nor a stdin stream was detected."
            )
            sys.exit(1)

    convert_stream(args.in_file, args.out_file)


if __name__ == "__main__":
    main()
