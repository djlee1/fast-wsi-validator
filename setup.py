import sys
from setuptools import setup, Extension
import pybind11

# README.md 파일 읽기
with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

# 시스템에 설치된 라이브러리 경로
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
        # 소스 파일 경로 수정
        sources=['src/validator.cpp'], 
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=['jpeg', 'tiff'],
        language='c++',
        extra_compile_args=['-std=c++17', '-O3'],
    ),
]

setup(
    name='fast-wsi-validator',  # PyPI에 등록될 이름
    version='0.1.0',           # 첫 배포 버전
    author='Dongjoo Lee',        # 본인 이름
    author_email='dongjoo.lee@portrai.io', # 본인 이메일
    description='A highly optimized validator for JPEG tiles in WSI files (TIFF, SVS)',
    long_description=long_description,
    long_description_content_type="text/markdown",
    url='https://github.com/djlee1/fast-wsi-validator', # 2단계에서 만들 GitHub 저장소 주소
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