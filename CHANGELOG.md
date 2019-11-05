# Change Log

All notable changes to this project will be documented in this file.

## [Unreleased]
### Added
- Add PyPI support.
- Add x86 support by @adrianherrera.
- Fix warnings thrown in function `x86_run` and `arm_run` when compiling `pyasmjit.c`

### Changed
- Rename x86 JIT to x86_64 by @adrianherrera.
- Improvements to `setup.py` by @adrianherrera.
- General clean-up of `pyasmjit` module by @adrianherrera.
- Updated ARM test by @adrianherrera.
- Refactor test code to make use of Python's unit test module by @adrianherrera.

### Fixed
- Fix Python 3 compatibility issues.
- Fix x86 tests by @adrianherrera.

## [0.2] - 2015-04-06
### Added
- Memory allocation mechanism (`alloc`/`free` style).
- Add support for the ARM architecture (32 bits).

### Fixed
- Fix context saving for long ints.
- Fix missing registers when loading and saving context.

## 0.1 - 2014-11-19
### Added
- First release.

[Unreleased]: https://github.com/programa-stic/pyasmjit/compare/v0.2...master
[0.2]: https://github.com/programa-stic/pyasmjit/compare/v0.1...v0.2
