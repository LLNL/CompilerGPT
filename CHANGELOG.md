# Changelog

## [Unreleased]

### Added
- Environment variable `LLMTOOLS_PATH` to replace hard-coded directory path in test-llmtools.cc
- Comprehensive logging system with timestamps for all operations
- Command-line options for test-llmtools.bin:
  - `--help`: Display help message
  - `--log=<file>`: Enable logging to specified file
  - `--model=<model>`: Specify a different model than the default (gpt-4o)
- Exception handling to capture and log errors
- Detailed documentation in README.md for the llmtools library

### Changed
- Updated Makefile to properly link with Boost libraries (program_options and filesystem)
- Modified test-llmtools.cc to use environment variable instead of hard-coded path
- Updated README.md with example command lines and expected output

### Fixed
- Hard-coded directory path issue in test-llmtools.cc
- Linking issues with Boost libraries
- Added error handling for missing environment variable

## [0.1.0] - 2025-08-26
- Initial version of llmtools library
