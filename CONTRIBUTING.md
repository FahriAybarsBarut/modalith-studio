# Contributing to Modalith Studio

Thank you for your interest in contributing to Modalith Studio! This document outlines the process and guidelines for contributing.

## Getting Started

### Build Instructions

Please refer to the [README](README.md) for detailed build instructions, including prerequisites and platform-specific setup.

### Development Environment

- **C++ Standard:** C++20
- **Build System:** CMake
- **Package Manager:** vcpkg (optional)
- **Formatter:** clang-format (see [`.clang-format`](.clang-format))
- **Linter:** clang-tidy (see [`.clang-tidy`](.clang-tidy))

## How to Contribute

### Reporting Bugs

Open a [bug report](https://github.com/FahriAybarsBarut/modalith-studio/issues/new?template=bug_report.md) using the provided issue template.

### Suggesting Features

Open a [feature request](https://github.com/FahriAybarsBarut/modalith-studio/issues/new?template=feature_request.md) using the provided issue template.

### Pull Request Process

1. **Fork & branch** — Create a feature branch from `main`:
   ```bash
   git checkout -b feature/my-feature main
   ```
2. **Develop** — Make your changes, following the code style guidelines below.
3. **Test** — Ensure all existing and new tests pass.
4. **Push & PR** — Push your branch and open a Pull Request against `main`.
5. **Review** — Address any review feedback.
6. **Merge** — Once approved, a maintainer will merge your PR.

### Commit Message Format

We follow [Conventional Commits](https://www.conventionalcommits.org/). Each commit message should have the format:

```
<type>(<scope>): <description>

[optional body]

[optional footer(s)]
```

**Types:**

| Type       | Description                          |
|------------|--------------------------------------|
| `feat`     | A new feature                        |
| `fix`      | A bug fix                            |
| `docs`     | Documentation changes                |
| `style`    | Code style (formatting, no logic)    |
| `refactor` | Code refactoring                     |
| `perf`     | Performance improvement              |
| `test`     | Adding or updating tests             |
| `build`    | Build system or dependencies         |
| `ci`       | CI configuration changes             |
| `chore`    | Other maintenance tasks              |

**Examples:**

```
feat(solver): add sparse matrix support for eigenvalue analysis
fix(gui): correct frequency axis scaling in mode shape viewer
docs(readme): update build instructions for macOS
```

## Testing

- All new features **must** include unit tests.
- All bug fixes **should** include a regression test.
- Tests use [GoogleTest](https://github.com/google/googletest).
- Run the test suite before submitting a PR:
  ```bash
  cmake -S . -B build -DMODALITH_BUILD_TESTS=ON
  cmake --build build --config Release --parallel
  ctest --test-dir build -C Release --output-on-failure
  ```

## Code Style

- Format your code with **clang-format** before committing:
  ```bash
  clang-format -i src/**/*.cpp include/**/*.h
  ```
- The project's [`.clang-format`](.clang-format) configuration is enforced in CI.
- Run **clang-tidy** to catch common issues:
  ```bash
  clang-tidy -p build src/**/*.cpp
  ```

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).
