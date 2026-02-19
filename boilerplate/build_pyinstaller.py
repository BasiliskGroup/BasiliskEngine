#!/usr/bin/env python3
"""
Build script for PyInstaller that:
1. Runs PyInstaller on a given file or spec
2. Copies all top-level files and folders (excluding PyInstaller-generated ones) 
   into the dist/.../_internal/ folder
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


def get_pyinstaller_output_dir(spec_file_or_script):
    """
    Determine the output directory name from a spec file or Python script.
    Returns the name that PyInstaller will use for the dist/ subdirectory.
    """
    path = Path(spec_file_or_script)
    
    if path.suffix == '.spec':
        # Read the spec file to find the name
        with open(path, 'r') as f:
            content = f.read()
            # Look for name='...' in EXE or COLLECT sections
            import re
            match = re.search(r"name=['\"]([^'\"]+)['\"]", content)
            if match:
                return match.group(1)
            # Fallback: use the spec file name without extension
            return path.stem
    else:
        # For Python scripts, use the script name without extension
        return path.stem


def run_pyinstaller(spec_file_or_script, work_dir=None):
    """Run PyInstaller on the given file."""
    if work_dir is None:
        work_dir = Path(spec_file_or_script).parent
    
    work_dir = Path(work_dir)
    spec_file_or_script = Path(spec_file_or_script)
    
    # Make path relative to work_dir if it's not already absolute
    if not spec_file_or_script.is_absolute():
        spec_file_or_script = work_dir / spec_file_or_script
    else:
        # Make it relative to work_dir for the command
        try:
            spec_file_or_script = spec_file_or_script.relative_to(work_dir)
        except ValueError:
            # If it's not under work_dir, use absolute path
            pass
    
    print(f"Running PyInstaller on: {spec_file_or_script}")
    print(f"Working directory: {work_dir}")
    
    result = subprocess.run(
        [sys.executable, '-m', 'PyInstaller', str(spec_file_or_script)],
        cwd=work_dir,
        check=True
    )
    
    return result


def copy_top_level_files(source_dir, dest_dir, exclude_patterns=None):
    """
    Copy all top-level files and folders from source_dir to dest_dir,
    excluding PyInstaller-generated files and patterns in exclude_patterns.
    """
    if exclude_patterns is None:
        exclude_patterns = []
    
    # Default exclusions for PyInstaller-generated files
    default_excludes = {
        'build',
        'dist',
        '__pycache__',
        '*.pyc',
        '*.pyo',
        '*.spec',
        '.pytest_cache',
        '.mypy_cache',
        '*.egg-info',
    }
    
    # Combine with user-provided exclusions
    all_excludes = set(default_excludes) | set(exclude_patterns)
    
    source_dir = Path(source_dir)
    dest_dir = Path(dest_dir)
    
    if not source_dir.exists():
        print(f"Warning: Source directory {source_dir} does not exist")
        return
    
    dest_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"Copying files from {source_dir} to {dest_dir}")
    
    copied_count = 0
    skipped_count = 0
    
    for item in source_dir.iterdir():
        # Skip if matches exclusion pattern
        should_exclude = False
        for pattern in all_excludes:
            if pattern.endswith('*'):
                # Wildcard pattern
                if item.name.startswith(pattern[:-1]) or item.suffix in ['.pyc', '.pyo']:
                    should_exclude = True
                    break
            elif item.name == pattern or item.name.startswith(pattern):
                should_exclude = True
                break
        
        if should_exclude:
            print(f"  Skipping: {item.name}")
            skipped_count += 1
            continue
        
        dest_item = dest_dir / item.name
        
        try:
            if item.is_file():
                shutil.copy2(item, dest_item)
                print(f"  Copied file: {item.name}")
                copied_count += 1
            elif item.is_dir():
                if dest_item.exists():
                    shutil.rmtree(dest_item)
                shutil.copytree(item, dest_item, dirs_exist_ok=True)
                print(f"  Copied directory: {item.name}/")
                copied_count += 1
        except Exception as e:
            print(f"  Error copying {item.name}: {e}")
    
    print(f"\nCopy complete: {copied_count} items copied, {skipped_count} items skipped")


def main():
    parser = argparse.ArgumentParser(
        description='Build PyInstaller bundle and copy top-level files to _internal/'
    )
    parser.add_argument(
        'file',
        nargs='?',
        default='test.spec',
        help='PyInstaller spec file or Python script to build (default: test.spec)'
    )
    parser.add_argument(
        '--work-dir',
        type=str,
        default=None,
        help='Working directory (default: directory containing the file)'
    )
    parser.add_argument(
        '--source-dir',
        type=str,
        default=None,
        help='Source directory to copy files from (default: work-dir)'
    )
    parser.add_argument(
        '--exclude',
        action='append',
        default=[],
        help='Additional patterns to exclude (can be used multiple times)'
    )
    parser.add_argument(
        '--no-copy',
        action='store_true',
        help='Skip copying files, only run PyInstaller'
    )
    
    args = parser.parse_args()
    
    # Determine working directory
    file_path = Path(args.file)
    if not file_path.is_absolute():
        file_path = Path.cwd() / file_path
    
    if args.work_dir:
        work_dir = Path(args.work_dir)
    else:
        work_dir = file_path.parent
    
    if args.source_dir:
        source_dir = Path(args.source_dir)
    else:
        source_dir = work_dir
    
    # Step 1: Run PyInstaller
    print("=" * 60)
    print("Step 1: Running PyInstaller")
    print("=" * 60)
    try:
        run_pyinstaller(file_path, work_dir)
    except subprocess.CalledProcessError as e:
        print(f"Error: PyInstaller failed with exit code {e.returncode}")
        sys.exit(1)
    
    # Step 2: Find the _internal directory
    output_name = get_pyinstaller_output_dir(file_path)
    dist_dir = work_dir / 'dist' / output_name
    internal_dir = dist_dir / '_internal'
    
    if not internal_dir.exists():
        print(f"\nWarning: _internal directory not found at {internal_dir}")
        print("PyInstaller may have used a different output structure.")
        print("Please check the dist/ directory manually.")
        return
    
    # Step 3: Copy files to _internal (if not skipped)
    if not args.no_copy:
        print("\n" + "=" * 60)
        print("Step 2: Copying top-level files to _internal/")
        print("=" * 60)
        copy_top_level_files(source_dir, internal_dir, args.exclude)
    
    # Step 4: Move dist/<name> to top level, rename to 'build', and clean up
    print("\n" + "=" * 60)
    print("Step 3: Reorganizing output structure")
    print("=" * 60)
    
    # Target location: top-level 'build' folder
    final_build_dir = work_dir / 'build'
    
    # First, remove PyInstaller's temporary build directory (build/<name>/)
    pyinstaller_build_dir = work_dir / 'build' / output_name
    if pyinstaller_build_dir.exists():
        print(f"Removing PyInstaller build directory: {pyinstaller_build_dir}")
        shutil.rmtree(pyinstaller_build_dir)
    
    # Remove PyInstaller's build parent directory if it's now empty
    pyinstaller_build_parent = work_dir / 'build'
    if pyinstaller_build_parent.exists():
        try:
            if not any(pyinstaller_build_parent.iterdir()):
                pyinstaller_build_parent.rmdir()
                print(f"Removed empty PyInstaller build parent: {pyinstaller_build_parent}")
        except Exception:
            pass
    
    # Remove old top-level build directory if it exists (from previous runs)
    if final_build_dir.exists():
        print(f"Removing existing build directory: {final_build_dir}")
        shutil.rmtree(final_build_dir)
    
    # Move dist/<name> to top-level 'build'
    print(f"Moving {dist_dir} to {final_build_dir}")
    shutil.move(str(dist_dir), str(final_build_dir))
    
    # Clean up PyInstaller-generated directories and files
    print("Cleaning up PyInstaller-generated files...")
    
    # Remove dist/ directory (should be empty now)
    dist_parent = work_dir / 'dist'
    if dist_parent.exists():
        try:
            remaining_items = list(dist_parent.iterdir())
            if remaining_items:
                # If there are other builds, just note it
                print(f"  Note: dist/ directory contains other builds, keeping it")
            else:
                dist_parent.rmdir()
                print(f"  Removed: {dist_parent}")
        except Exception as e:
            print(f"  Warning: Could not remove {dist_parent}: {e}")
    
    # Remove .spec file
    spec_file_path = file_path if file_path.suffix == '.spec' else work_dir / f"{output_name}.spec"
    if spec_file_path.exists() and spec_file_path.suffix == '.spec':
        spec_file_path.unlink()
        print(f"  Removed: {spec_file_path}")
    
    print("\n" + "=" * 60)
    print("Build complete!")
    print(f"Final output directory: {final_build_dir}")
    print(f"Executable: {final_build_dir / output_name}")
    print("=" * 60)


if __name__ == '__main__':
    main()

