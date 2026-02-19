#!/usr/bin/env python3
"""
Build script for PyInstaller that:
1. Runs PyInstaller on a given file or spec
2. Copies all top-level files and folders (excluding PyInstaller-generated ones)
   into the dist/.../_internal/ folder

Can be run directly or via: basilisk file.py / basilisk-engine file.py
"""

import sys
from pathlib import Path

# Allow running from repo without installing (add parent to path for basilisk package)
if __name__ == '__main__':
    _root = Path(__file__).resolve().parent.parent
    if str(_root) not in sys.path:
        sys.path.insert(0, str(_root))

from basilisk.build import main

if __name__ == '__main__':
    main()
