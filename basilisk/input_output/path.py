import os
import sys


# Credit for function: https://stackoverflow.com/questions/7674790/bundling-data-files-with-pyinstaller-onefile
def get_root():
    """ Get absolute path to resource, works for dev and for PyInstaller """
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return base_path + '/basilisk'
