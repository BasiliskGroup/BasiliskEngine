#!/usr/bin/env python3
"""Wrapper script to run pybind11-stubgen, catching errors to make it non-fatal."""
import os
import sys
import subprocess

# Set up PYTHONPATH
pythonpath = os.environ.get('PYTHONPATH', '')
build_dir = sys.argv[1]
output_dir = sys.argv[2]
os.environ['PYTHONPATH'] = build_dir + os.pathsep + pythonpath if pythonpath else build_dir

# Run stubgen, but don't fail if it errors (e.g., missing runtime dependencies)
try:
    result = subprocess.run(
        [sys.executable, '-m', 'pybind11_stubgen', '--output-dir', output_dir, 'basilisk', '--ignore-all-errors'],
        cwd=build_dir,
        capture_output=False  # Let output go through
    )
    # Always exit successfully so build continues
    sys.exit(0)
except Exception as e:
    # Log error but don't fail build
    print(f"Warning: Stub generation failed (non-fatal): {e}", file=sys.stderr)
    sys.exit(0)

