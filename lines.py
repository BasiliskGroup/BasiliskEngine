#!/usr/bin/env python3
from pathlib import Path

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
    total_lines = 0

    for directory in directories:
        root = Path(directory)
        if not root.exists():
            continue

        for path in root.rglob("*"):
            if path.is_file():
                if extensions is None or path.suffix in extensions:
                    total_lines += count_non_empty_lines_in_file(path)

    return total_lines


if __name__ == "__main__":
    # Directories to search (edit these)
    DIRECTORIES = [
        "./src",
        "./bindings",
        "./include/basilisk"
    ]

    total = count_non_empty_lines(DIRECTORIES)
    print(f"Total non-empty lines: {total}")
