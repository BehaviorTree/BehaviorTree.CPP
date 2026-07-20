#!/usr/bin/env python3
"""Validate a BehaviorTree.CPP XML file against XSD and/or Schematron schemas.

Exit codes:
  0  all enabled validations passed
  1  one or more validation errors
  2  usage error or file not found
"""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

try:
    from lxml import etree, isoschematron
    _LXML = True
except ImportError:
    _LXML = False

_SCRIPT_DIR = Path(__file__).resolve().parent
_DEFAULT_XSD = _SCRIPT_DIR / "generated" / "bt4.xsd"
_DEFAULT_SCH = _SCRIPT_DIR / "generated" / "bt4.sch"


def validate_xsd(xml_file: str, schema_file: str, force_lxml: bool = False) -> bool:
    if not force_lxml and shutil.which("xmllint"):
        result = subprocess.run(
            ["xmllint", "--noout", "--schema", schema_file, xml_file],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            sys.stderr.write(result.stderr)
            return False
        return True

    if not _LXML:
        print("error: neither xmllint nor lxml is available for XSD validation", file=sys.stderr)
        sys.exit(2)

    schema = etree.XMLSchema(etree.parse(schema_file))
    doc = etree.parse(xml_file)
    if not schema.validate(doc):
        for err in schema.error_log:
            print(f"{xml_file}:{err.line}: {err.message}", file=sys.stderr)
        return False
    return True


def validate_schematron(xml_file: str, sch_file: str) -> bool:
    if not _LXML:
        print("error: lxml is required for Schematron validation (pip install lxml)", file=sys.stderr)
        sys.exit(2)

    sch = isoschematron.Schematron(etree.parse(sch_file), store_report=True)
    ok = sch.validate(etree.parse(xml_file))
    if not ok:
        ns = "http://purl.oclc.org/dsdl/svrl"
        for fa in sch.validation_report.findall(f".//{{{ns}}}failed-assert"):
            loc  = fa.get("location")
            text = fa.findtext(f"{{{ns}}}text", "").strip()
            print(f"{xml_file}: {loc}: {text}", file=sys.stderr)
    return ok


def main():
    parser = argparse.ArgumentParser(
        description="Validate a BehaviorTree.CPP XML file against XSD and Schematron schemas.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
examples:
  %(prog)s myfile.xml
  %(prog)s --schema custom.xsd --schematron custom.sch myfile.xml
  %(prog)s --no-xsd myfile.xml
  %(prog)s --no-sch myfile.xml
""",
    )
    parser.add_argument("xml_file", help="BehaviorTree XML file to validate")
    parser.add_argument(
        "-s", "--schema",
        default=str(_DEFAULT_XSD),
        metavar="FILE",
        help=f"XSD schema (default: {_DEFAULT_XSD})",
    )
    parser.add_argument(
        "-t", "--schematron",
        default=str(_DEFAULT_SCH),
        metavar="FILE",
        help=f"Schematron schema (default: {_DEFAULT_SCH})",
    )
    parser.add_argument("--no-xsd", action="store_true", help="skip XSD validation")
    parser.add_argument("--no-sch", action="store_true", help="skip Schematron validation")
    parser.add_argument("--lxml", action="store_true", help="force use of lxml for XSD validation")

    args = parser.parse_args()

    if args.no_xsd and args.no_sch:
        parser.error("--no-xsd and --no-sch together disable all validation; nothing to do")

    if not Path(args.xml_file).exists():
        parser.error(f"file not found: {args.xml_file}")

    passed = True

    if not args.no_xsd:
        if not Path(args.schema).exists():
            parser.error(f"XSD schema not found: {args.schema}")
        if not validate_xsd(args.xml_file, args.schema, force_lxml=args.lxml):
            passed = False

    if not args.no_sch:
        if not Path(args.schematron).exists():
            parser.error(f"Schematron schema not found: {args.schematron}")
        if not validate_schematron(args.xml_file, args.schematron):
            passed = False

    if passed:
        print(f"{args.xml_file} validates OK")
    sys.exit(0 if passed else 1)


if __name__ == "__main__":
    main()
