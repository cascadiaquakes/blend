import importlib.util
import json
import os
from pathlib import Path

from docutils import nodes
from docutils.parsers.rst import Directive

project = "BLEND"
author = "Rasheed Ajala"
copyright = "2026, The CRESCENT cyberinfrastructure team"

release = "2.0.0"
version = release

if importlib.util.find_spec("sphinx_rtd_theme") is None:
    raise RuntimeError(
        "BLEND documentation uses the Read the Docs theme. "
        "Install it with 'python -m pip install sphinx-rtd-theme' "
        "or 'conda install -c conda-forge sphinx_rtd_theme'."
    )

extensions = ["sphinx_rtd_theme"]
templates_path = ["_templates"]
exclude_patterns = []

html_theme = "sphinx_rtd_theme"
html_title = "BLEND Documentation"
html_static_path = ["_static"]
html_css_files = ["blend.css"]
html_js_files = ["blend.js"]
html_copy_source = False
html_show_sourcelink = False
html_theme_options = {
    "collapse_navigation": False,
    "navigation_depth": 4,
    "style_external_links": True,
}


def _blend_versions():
    default_versions = [
        ("devel-2.0", "https://cascadiaquakes.github.io/blend/"),
        ("2.0.0", "https://cascadiaquakes.github.io/blend/2.0.0/"),
    ]
    raw_versions = os.environ.get("BLEND_DOC_VERSIONS")
    if not raw_versions:
        return default_versions

    try:
        versions = json.loads(raw_versions)
    except json.JSONDecodeError as exc:
        raise RuntimeError("BLEND_DOC_VERSIONS must be JSON") from exc

    parsed_versions = []
    for item in versions:
        if not isinstance(item, list) or len(item) != 2:
            raise RuntimeError("BLEND_DOC_VERSIONS entries must be [label, url] pairs")
        parsed_versions.append((str(item[0]), str(item[1])))
    return parsed_versions


html_context = {
    "display_github": True,
    "github_user": "cascadiaquakes",
    "github_repo": "blend",
    "github_version": os.environ.get("BLEND_DOC_GITHUB_VERSION", "devel-2.0"),
    "conf_py_path": "/doc/rst/source/",
    "blend_current_version": os.environ.get("BLEND_DOC_CURRENT_VERSION", "devel-2.0"),
    "blend_versions": _blend_versions(),
}

latex_documents = [
    ("index", "blend.tex", "BLEND Documentation", author, "manual"),
]

latex_elements = {
    "papersize": "letterpaper",
    "pointsize": "10pt",
    "fontpkg": "",
}


class BlendUsageDirective(Directive):
    required_arguments = 1
    optional_arguments = 0
    has_content = False

    def run(self):
        module = self.arguments[0]
        usage_dir = os.environ.get("BLEND_DOC_USAGE_DIR")
        if not usage_dir:
            warning = self.state_machine.reporter.warning(
                "BLEND_DOC_USAGE_DIR is not set; generated usage is unavailable",
                line=self.lineno,
            )
            return [warning]

        usage_path = Path(usage_dir) / f"{module}.txt"
        try:
            text = usage_path.read_text(encoding="utf-8")
        except OSError as exc:
            warning = self.state_machine.reporter.warning(
                f"Could not read generated usage file {usage_path}: {exc}",
                line=self.lineno,
            )
            return [warning]

        literal = nodes.literal_block(text, text)
        literal["language"] = "text"
        return [literal]


def setup(app):
    app.add_directive("blend-usage", BlendUsageDirective)
    return {"version": "1.0", "parallel_read_safe": True}
