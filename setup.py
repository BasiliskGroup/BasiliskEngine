from setuptools import setup, find_packages
from os import path
working_directory = path.abspath(path.dirname(__file__))

with open(path.join(working_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name = 'basilisk-engine',  # Name of folder containing scripts and __init__
    version = '0.1.31',
    url = 'https://basilisk-website.vercel.app/',
    author = 'Name',
    author_email = 'basiliskengine@gmail.com',
    description = 'Python 3D Framework',
    long_description = long_description,  # Load from file
    long_description_content_type = 'text/markdown',
    packages=find_packages(),
    include_package_data=True,
    install_requires=['numpy', 'pillow', 'pygame-ce', 'moderngl', 'PyGLM', 'numba'],  # Include all used packages
)