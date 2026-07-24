"""Sphinx config."""

project = "pybt"
author = "BehaviorTree.CPP contributors"
extensions = ["myst_parser"]
source_suffix = {".md": "markdown", ".rst": "restructuredtext"}
exclude_patterns = ["_build"]
html_theme = "alabaster"  # default; furo theme arrives in Phase 7
