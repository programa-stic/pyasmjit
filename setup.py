#! /usr/bin/env python

from setuptools import setup, Extension

pyasmjit_module = Extension(
    'pyasmjit.pyasmjit',
    sources      = [
        'pyasmjit/pyasmjit.c'
    ],
)

setup(
    author       = 'Christian Heitman',
    author_email = 'cnheitman@fundacionsadosky.org.ar',
    description  = 'JIT assemby code generation and execution',
    ext_modules  = [
        pyasmjit_module
    ],
    license      = 'BSD 2-Clause',
    name         = 'pyasmjit',
    url          = 'http://github.com/programa-stic/pyasmjit',
    version      = '0.2',
    test_suite   = 'tests',
)
