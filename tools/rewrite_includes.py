#!/usr/bin/env python3
import os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "src"
PREFIX = "basilisk"

pattern = re.compile(r'#\s*include\s*"([^"]+)"')

for root, _, files in os.walk(ROOT):
    for f in files:
        if f.endswith((".cpp", ".h", ".hpp", ".tpp")):
            path = os.path.join(root, f)
            with open(path, "r") as file:
                content = file.read()
            # Replace "whatever" with <basilisk/whatever>
            new_content = pattern.sub(lambda m: f'#include <{PREFIX}/{m.group(1)}>', content)
            if new_content != content:
                with open(path, "w") as file:
                    file.write(new_content)
                print(f"Updated includes in {path}")