# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0-alpha.1] - 2025-11-26
### Added
- Introduced CI/CD automated release process
- Added a `CHANGELOG.md`
- Added a `Version.hpp`, which includes multiple macros related to version numbers: `*_MAJOR`, `*_MINOR`, `*_PATCH`, and `*_VERSION`, etc.
- Added a CMake script that automatically parses the version number from `Version.hpp`
- Added a CMake script for handling directory adjustments during packaging

### Changed
- Renamed `main.cpp` to `iterate.cpp`
- Improved the internal template meta-tools
- Modified `.gitignore` to track `.github/`
- Squashed some small, semantically ambiguous or meaningless commits
- Altered the CMake packaging logic, no longer relying on CPack to package the project

### Fixed
- None

### Removed
- None

- - -
