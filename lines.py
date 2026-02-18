#!/usr/bin/env python3
from pathlib import Path
from collections import Counter

def count_non_empty_lines_in_file(path: Path) -> int:
    count = 0
    try:
        with path.open("r", encoding="utf-8", errors="ignore") as f:
            for line in f:
                if line.strip():
                    count += 1
    except (OSError, UnicodeDecodeError):
        # Skip files we can't read
        pass
    return count


def count_non_empty_lines(directories, extensions=None):
    totals_by_dir = {}
    grand_total = 0
    extension_counts = Counter()

    for directory in directories:
        root = Path(directory)
        dir_total = 0

        if not root.exists():
            totals_by_dir[directory] = 0
            continue

        for path in root.rglob("*"):
            if path.is_file():
                ext = path.suffix if path.suffix else "<no_ext>"

                if extensions is None or path.suffix in extensions:
                    extension_counts[ext] += 1
                    lines = count_non_empty_lines_in_file(path)
                    dir_total += lines

        totals_by_dir[directory] = dir_total
        grand_total += dir_total

    return totals_by_dir, grand_total, extension_counts


if __name__ == "__main__":
    DIRECTORIES = [
        "./src",
        "./bindings",
        "./include/basilisk",
        "./rust_gpu/src"
    ]

    totals, total, ext_counts = count_non_empty_lines(DIRECTORIES)

    for directory, lines in totals.items():
        print(f"{directory}: {lines}")

    print(f"\nTotal non-empty lines: {total}\n")

    print("File counts by extension:")
    for ext, count in sorted(ext_counts.items()):
        print(f"{ext}: {count}")
