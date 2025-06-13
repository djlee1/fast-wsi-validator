import sys
from setuptools import setup, Extension
import pybind11

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

include_dirs = [
    pybind11.get_include(),
    '/usr/include',
    '/opt/homebrew/include',
    '/usr/local/include'
]

library_dirs = [
    '/usr/lib',
    '/opt/homebrew/lib',
    '/usr/local/lib'
]

ext_modules = [
    Extension(
        'fast_wsi_validator',
        sources=['src/validator.cpp'], 
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=['jpeg', 'tiff'],
        language='c++',
        extra_compile_args=['-std=c++17', '-O3'],
    ),
]

setup(
    name='fast-wsi-validator', 
    version='0.1.0',           
    author='Dongjoo Lee',        
    author_email='dongjoo.lee@portrai.io', 
    description='A highly optimized validator for JPEG tiles in WSI files (TIFF, SVS)',
    long_description=long_description,
    long_description_content_type="text/markdown",
    url='https://github.com/djlee1/fast-wsi-validator',
    ext_modules=ext_modules,
    classifiers=[
        'Programming Language :: Python :: 3',
        'Programming Language :: C++',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering :: Image Processing',
    ],
    python_requires='>=3.7',
)
