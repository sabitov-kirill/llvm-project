#!/usr/bin/env python3
"""Generate scop_feature_map.json from the FeatureID enum in ScopFeatures.h.

Usage:
    gen_scop_feature_map.py <ScopFeatures.h path> <output.json path>

Reads lines of the form:
    Name = N, // description
inside the FeatureID enum and produces:
    [{"index": N, "name": "snake_case_name", "description": "..."}, ...]

The "NUM_FEATURES" sentinel is excluded from the output.
"""

import json
import re
import sys
from pathlib import Path


def camel_to_snake(name: str) -> str:
    """Convert CamelCase to snake_case."""
    s1 = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", name)
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", s1).lower()


def parse_feature_enum(header_text: str) -> list[dict]:
    # Find the FeatureID enum block.
    enum_match = re.search(
        r"enum\s+class\s+FeatureID\s*:\s*\w+\s*\{([^}]*)\}", header_text, re.DOTALL
    )
    if not enum_match:
        raise ValueError("Could not find 'enum class FeatureID' in header")

    enum_body = enum_match.group(1)
    features = []

    # Match: Name = N, // description (trailing comma optional)
    pattern = re.compile(
        r"^\s*(\w+)\s*=\s*(\d+)\s*,?\s*//\s*(.*?)\s*$", re.MULTILINE
    )
    for m in pattern.finditer(enum_body):
        name, index, desc = m.group(1), int(m.group(2)), m.group(3)
        if name == "NUM_FEATURES":
            continue
        features.append(
            {
                "index": index,
                "name": camel_to_snake(name),
                "description": desc,
            }
        )

    features.sort(key=lambda f: f["index"])
    return features


def main() -> None:
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <ScopFeatures.h> <output.json>", file=sys.stderr)
        sys.exit(1)

    header_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2])

    header_text = header_path.read_text(encoding="utf-8")
    features = parse_feature_enum(header_text)

    output_path.write_text(
        json.dumps(features, indent=2) + "\n", encoding="utf-8"
    )
    print(f"[gen_scop_feature_map] wrote {len(features)} features to {output_path}")


if __name__ == "__main__":
    main()
