#!/usr/bin/env python

from setuptools import Extension
from setuptools import find_packages
from setuptools import setup

__version__ = '0.2'

pyasmjit_module = Extension(
    'pyasmjit',
    sources      = [
        'pyasmjit/pyasmjit.c'
    ],
)

setup(
    name         = 'pyasmjit',
    author       = 'Christian Heitman',
    author_email = 'cnheitman@fundacionsadosky.org.ar',
    description  = 'JIT assemby code generation and execution',
    download_url = 'https://github.com/programa-stic/pyasmjit/tarball/v0.2',
    license      = 'BSD 2-Clause',
    url          = 'http://github.com/programa-stic/pyasmjit',
    version      = __version__,
    classifiers  = [
        'Development Status :: 2 - Pre-Alpha',
        'License :: OSI Approved :: BSD License',
        'Natural Language :: English',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: C',
        'Topic :: Software Development :: Assemblers',
    ],
    ext_package  = 'pyasmjit',
    ext_modules  = [
        pyasmjit_module,
    ],
    packages=find_packages(exclude=['tests', 'tests.*']),
    test_suite   = 'tests',
)
