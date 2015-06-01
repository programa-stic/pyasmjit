#! /usr/bin/env python

from setuptools import setup, Extension

__version__ = '0.2'

pyasmjit_module = Extension(
    'pyasmjit.pyasmjit',
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
    ext_modules  = [
        pyasmjit_module,
    ],
)
