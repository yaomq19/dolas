# AGENTS.md - Dolas Engine Development Guidelines

Guidelines for AI agents working on the Dolas Engine codebase.

## Project Overview

Dolas is a lightweight game engine built with C++20 and DirectX 11. Uses CMake for building and Catch2 for unit testing.

- `engine_runtime`: Core runtime (platform, math, resources)
- `engine_tool`: Development tools (editor, shader compiler)
- `engine_test`: Unit tests (Catch2)

Third-party dependencies via Git submodules.

## Build Commands

### Environment Setup
```bash
git submodule update --init --recursive
.\script\setup.bat
```

### Quick Build Scripts
```bash
.\script\build-debug.bat
.\script\build-release.bat
.\script\build-relWithDebInfo.bat
.\script\build-minSizeRel.bat
```

### Manual CMake Commands
```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug -j16
cmake --build build --config Release -j16
```

### Output Directories
- Executables: `build/bin/{Configuration}/`
- Libraries: `build/lib/{Configuration}/`
- Intermediate files: `build/`

## Testing Commands

### Running All Tests
```bash
cd build
ctest -C Debug --output-on-failure
# Or directly
.\bin\Debug\DolasTest.exe
```

### Running a Single Test
```bash
# CTest with regex
ctest -C Debug -R "Vector3.*constructor" --output-on-failure

# Catch2 filter
.\bin\Debug\DolasTest.exe "[Vector3][constructor]"

# List tests
.\bin\Debug\DolasTest.exe --list-tests
```

### Test Discovery
Tests use Catch2 `TEST_CASE` with tags:
```cpp
TEST_CASE("Vector3 default constructor", "[Vector3][constructor]")
```

## Code Style Guidelines

### Naming Conventions
- **Classes/Structs**: `PascalCase` (`Vector3`, `Matrix4x4`)
- **Functions/Methods**: `PascalCase` (`Length()`, `Normalized()`)
- **Member variables**: `m_snake_case` with `m_` prefix (`m_window_handle`)
- **Public members**: `snake_case` without prefix (`x`, `y`, `z`)
- **Local variables**: `snake_case` (`texture_id`, `file_path`)
- **Constants**: `UPPER_SNAKE_CASE` (`DOLAS_PI`, `DOLAS_FLOAT_MAX`)
- **Macros**: `UPPER_SNAKE_CASE` (`STRING_ID`, `DOLAS_RETURN_IF_NULL`)
- **Namespaces**: `PascalCase` (only `Dolas` namespace)
- **Type Aliases**: `PascalCase` (`Float`, `UInt`, `StringID`)

### File Organization
- **Headers**: `.h` in `public/include/` or `private/include/`
- **Sources**: `.cpp` in `private/src/`
- **Include guards**: `#ifndef DOLAS_MODULE_NAME_H`, `#define DOLAS_MODULE_NAME_H`

### Formatting
- **Indentation**: 4 spaces
- **Braces**: Allman style (braces on separate lines)
- **Pointer/Reference**: `Type* ptr`, `Type& ref` (no space before `*` or `&`)
- **Class layout**: `public:` sections first, then member variables

### Imports and Includes
- System headers: `#include <library/header.h>`
- Project headers: `#include "module/header.h"` (relative to `src/engine_*/`)
- Order: System first, then third-party, then project headers
- Example:
  ```cpp
  #include <catch2/catch_test_macros.hpp>
  #include "dolas_math.h"
  ```

### Type System
- Use type aliases from `dolas_base.h`:
  - `Float`, `Double`, `Int`, `UInt`
  - `Bool`, `UByte`, `ULong`, `ULongLong`
  - Resource IDs: `StringID`, `FileID`, `MaterialID`, etc.
- Avoid raw built-in types (`float`, `int`) in public interfaces

### Error Handling
- Use `DOLAS_RETURN_*` macros for early returns:
  ```cpp
  DOLAS_RETURN_IF_NULL(pointer);
  DOLAS_RETURN_FALSE_IF_FALSE(condition);
  DOLAS_RETURN_NULL_IF_TRUE(condition);
  ```
- No exceptions; use return values and error codes
- For unrecoverable errors, log with `LOG_ERROR()` and return

### Logging System
- Use logging macros (via `dolas_log_system_manager.h`):
  ```cpp
  LOG_TRACE("Message {}", arg);
  LOG_DEBUG("Debug info");
  LOG_INFO("Information");
  LOG_WARN("Warning");
  LOG_ERROR("Error occurred");
  LOG_CRITICAL("Critical failure");
  ```
- Thread-safe, uses spdlog, outputs to file (`logs/dolas.log`) and console

### String Hashing Convention
**Important**: Follow rules from `Developer.md`:
- `STRING_ID(x)`: For compile-time literal names only
- `HashConverter::StringHash(runtime_string)`: For runtime file paths and variable strings
- **Never** use `STRING_ID()` with runtime variables

### Memory Management
- Use `DOLAS_NEW(type, ...)` and `DOLAS_DELETE(ptr)` macros
- No smart pointers in public interfaces; follow existing patterns
- Ensure proper RHI resource lifecycle to avoid VRAM leaks

## Development Tools
- Tracy Profiler for performance analysis (`ZoneScoped` macros)
- Shaders offline-compiled via `ShaderCompiler` tool (`content/shader/`)
- Textures: DDS via DirectXTex
- 3D models: Assimp
- Materials: Custom `.material` files

## Cursor/Copilot Rules
No `.cursor/rules/`, `.cursorrules`, or `.github/copilot-instructions.md` files found. Follow conventions in this document.

## Common Tasks for Agents

### Adding a New Class
1. Header in appropriate `public/include/` subdirectory
2. Source in `private/src/` subdirectory
3. Follow naming conventions and include guards
4. Add to relevant CMakeLists.txt
5. Write unit tests in `engine_test/`

### Adding a New Test
1. Test file in `src/engine_test/{module}/`
2. Use Catch2 `TEST_CASE` with descriptive tags
3. Include necessary headers (`<catch2/...>` and project headers)
4. Follow existing test patterns (sections, assertions)

### Modifying Existing Code
1. Maintain consistent formatting
2. Update tests if interface changes
3. Use appropriate error handling macros
4. Log significant changes at appropriate levels

## Verification Checklist
Before considering work complete:
- [ ] Build succeeds with `.\script\build-debug.bat`
- [ ] Tests pass: `ctest -C Debug --output-on-failure`
- [ ] No new compiler warnings introduced
- [ ] Code follows naming conventions
- [ ] Headers have proper include guards
- [ ] String hashing follows conventions
- [ ] Error handling uses appropriate macros

---

*Based on analysis of the codebase as of March 2026. Update as conventions evolve.*