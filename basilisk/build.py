#!/usr/bin/env python3
"""
Build script for cx_Freeze that:
1. Runs cx_Freeze on a given Python script
2. Copies all top-level files and folders (excluding build-generated ones) 
   into the output folder
3. Reorganizes output to a clean 'build/' directory structure
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


def get_cxfreeze_build_dir():
    """
    Determine the cx_Freeze build directory name based on platform.
    cx_Freeze creates directories like: exe.linux-x86_64-3.10
    """
    system = platform.system().lower()
    machine = platform.machine()
    py_version = f"{sys.version_info.major}.{sys.version_info.minor}"
    
    return f"exe.{system}-{machine}-{py_version}"


def create_setup_script(script_path, work_dir, output_name):
    """
    Create a temporary setup.py for cx_Freeze.
    Returns the path to the created setup.py.
    """
    setup_content = f'''"""
Auto-generated cx_Freeze setup script
"""

import sys
from cx_Freeze import setup, Executable

build_exe_options = {{
    "packages": ["glm", "glcontext"],
    "includes": [],
    "excludes": ["tkinter", "unittest", "email", "html", "http", "xml", "pydoc"],
}}

base = None

executables = [
    Executable(
        "{script_path.name}",
        base=base,
        target_name="{output_name}",
    )
]

setup(
    name="{output_name}",
    version="1.0",
    description="Built with cx_Freeze",
    options={{"build_exe": build_exe_options}},
    executables=executables,
)
'''
    
    setup_path = work_dir / "_temp_setup.py"
    with open(setup_path, 'w') as f:
        f.write(setup_content)
    
    return setup_path


def run_cxfreeze(script_path, work_dir=None):
    """Run cx_Freeze on the given Python script."""
    if work_dir is None:
        work_dir = Path(script_path).parent
    
    work_dir = Path(work_dir)
    script_path = Path(script_path)
    
    # Make path absolute
    if not script_path.is_absolute():
        script_path = work_dir / script_path
    
    output_name = script_path.stem
    
    print(f"Running cx_Freeze on: {script_path}")
    print(f"Working directory: {work_dir}")
    
    # Create temporary setup.py
    setup_path = create_setup_script(script_path, work_dir, output_name)
    
    try:
        # Run cx_Freeze build
        result = subprocess.run(
            [sys.executable, str(setup_path), "build"],
            cwd=work_dir,
            check=True
        )
    finally:
        # Clean up temporary setup.py
        if setup_path.exists():
            setup_path.unlink()
    
    return result


def copy_top_level_files(source_dir, dest_dir, exclude_patterns=None):
    """
    Copy all top-level files and folders from source_dir to dest_dir,
    excluding build-generated files and patterns in exclude_patterns.
    """
    if exclude_patterns is None:
        exclude_patterns = []
    
    # Default exclusions for build-generated files
    default_excludes = {
        'build',
        'dist',
        '__pycache__',
        '*.pyc',
        '*.pyo',
        '*.spec',
        '*.py',
        '*.txt',
        '*.md',
        '*.github',
        '*.git',
        '*.gitkeep',
        '.gitignore',
        '.pytest_cache',
        '.mypy_cache',
        '*.egg-info',
        '_temp_setup.py',
        'build_cxfreeze.py',
        'build_pyinstaller.py',
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
            if pattern.startswith('*.'):
                # File extension pattern
                if item.suffix == pattern[1:] or item.name.endswith(pattern[1:]):
                    should_exclude = True
                    break
            elif pattern.endswith('*'):
                # Wildcard pattern
                if item.name.startswith(pattern[:-1]):
                    should_exclude = True
                    break
            elif item.name == pattern:
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


def copy_basilisk_resources(dest_dir):
    """
    Copy basilisk package resources (shaders, models, textures) to dest_dir/lib/.
    These are needed at runtime for the engine to find default resources.
    """
    try:
        import basilisk
        basilisk_package_dir = Path(basilisk.__file__).parent
    except ImportError:
        print("Warning: Could not import basilisk package. Skipping resource copy.")
        print("  Make sure basilisk is installed: pip install -e .")
        return
    
    # The resources should be in the same directory as the .so/.pyd file
    # (shaders/, models/, textures/ directories)
    basilisk_resources = ['shaders', 'models', 'textures']
    basilisk_dest = dest_dir / 'lib'
    basilisk_dest.mkdir(parents=True, exist_ok=True)
    
    print(f"Copying basilisk resources from {basilisk_package_dir} to {basilisk_dest}")
    
    copied_count = 0
    for resource_dir in basilisk_resources:
        source_path = basilisk_package_dir / resource_dir
        dest_path = basilisk_dest / resource_dir
        
        if source_path.exists() and source_path.is_dir():
            if dest_path.exists():
                shutil.rmtree(dest_path)
            shutil.copytree(source_path, dest_path, dirs_exist_ok=True)
            print(f"  Copied: {resource_dir}/")
            copied_count += 1
        else:
            print(f"  Warning: {resource_dir}/ not found in {basilisk_package_dir}")
    
    if copied_count == 0:
        print("  Warning: No basilisk resources found. The application may not work correctly.")
    else:
        print(f"  Successfully copied {copied_count} resource directories")


def main():
    parser = argparse.ArgumentParser(
        description='Build cx_Freeze bundle and copy top-level files'
    )
    parser.add_argument(
        'file',
        nargs='?',
        default='main.py',
        help='Python script to build (default: main.py)'
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
        help='Skip copying files, only run cx_Freeze'
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
    
    output_name = file_path.stem
    
    # Step 1: Run cx_Freeze
    print("=" * 60)
    print("Step 1: Running cx_Freeze")
    print("=" * 60)
    try:
        run_cxfreeze(file_path, work_dir)
    except subprocess.CalledProcessError as e:
        print(f"Error: cx_Freeze failed with exit code {e.returncode}")
        sys.exit(1)
    
    # Step 2: Find the cx_Freeze output directory
    # cx_Freeze creates: build/exe.<platform>-<version>/
    cxfreeze_build_subdir = get_cxfreeze_build_dir()
    cxfreeze_output_dir = work_dir / 'build' / cxfreeze_build_subdir
    
    if not cxfreeze_output_dir.exists():
        # Try to find it by pattern matching
        build_dir = work_dir / 'build'
        if build_dir.exists():
            for item in build_dir.iterdir():
                if item.is_dir() and item.name.startswith('exe.'):
                    cxfreeze_output_dir = item
                    break
    
    if not cxfreeze_output_dir.exists():
        print(f"\nError: cx_Freeze output directory not found")
        print(f"Expected: {cxfreeze_output_dir}")
        print("Please check the build/ directory manually.")
        sys.exit(1)
    
    print(f"\nFound cx_Freeze output: {cxfreeze_output_dir}")
    
    # Step 2: Copy basilisk package resources
    print("\n" + "=" * 60)
    print("Step 2: Copying basilisk package resources")
    print("=" * 60)
    copy_basilisk_resources(cxfreeze_output_dir)
    
    # Step 3: Copy files to output directory (if not skipped)
    if not args.no_copy:
        print("\n" + "=" * 60)
        print("Step 3: Copying top-level files to build output")
        print("=" * 60)
        copy_top_level_files(source_dir, cxfreeze_output_dir, args.exclude)
    
    # Step 4: Reorganize to final structure
    print("\n" + "=" * 60)
    print("Step 4: Reorganizing output structure")
    print("=" * 60)
    
    # Target location: top-level 'build' folder with just the contents
    final_build_dir = work_dir / 'build'
    temp_output_dir = work_dir / f'_temp_build_{output_name}'
    
    # Move cx_Freeze output to temp location
    print(f"Moving {cxfreeze_output_dir} to temporary location")
    shutil.move(str(cxfreeze_output_dir), str(temp_output_dir))
    
    # Remove the build directory (which now only has the exe.* folder gone)
    if final_build_dir.exists():
        # Check if there are other items in build/
        remaining = list(final_build_dir.iterdir())
        if not remaining:
            final_build_dir.rmdir()
            print(f"Removed empty build directory")
        else:
            # Remove remaining cx_Freeze artifacts
            for item in remaining:
                if item.is_dir():
                    shutil.rmtree(item)
                else:
                    item.unlink()
            final_build_dir.rmdir()
            print(f"Cleaned up build directory")
    
    # Move temp to final build location
    print(f"Moving to final location: {final_build_dir}")
    shutil.move(str(temp_output_dir), str(final_build_dir))
    
    # Rename executable if needed (cx_Freeze should already use the right name)
    executable_path = final_build_dir / output_name
    if not executable_path.exists():
        # Try to find the executable
        for item in final_build_dir.iterdir():
            if item.is_file() and os.access(item, os.X_OK) and not item.suffix:
                print(f"Renaming {item.name} to {output_name}")
                item.rename(executable_path)
                break
    
    # Clean up .egg-info directories created by cx_Freeze
    print("Cleaning up build artifacts...")
    for item in work_dir.iterdir():
        if item.is_dir() and item.name.endswith('.egg-info'):
            shutil.rmtree(item)
            print(f"  Removed: {item.name}")
    
    print("\n" + "=" * 60)
    print("Build complete!")
    print(f"Final output directory: {final_build_dir}")
    print(f"Executable: {executable_path}")
    print("=" * 60)


if __name__ == '__main__':
    main()

