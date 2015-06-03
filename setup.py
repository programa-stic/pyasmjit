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
    license      = 'BSD 2-Clause',
    url          = 'http://github.com/programa-stic/pyasmjit',
    version      = __version__,
    test_suite   = 'tests',
    ext_package  = 'pyasmjit',
    ext_modules  = [
        pyasmjit_module,
    ],
    packages=find_packages(exclude=['tests', 'tests.*']),
)
