# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

Dolas is a C++20 game engine for **Windows only** (Visual Studio 2022, CMake >= 3.15).

```bash
# First-time setup (after git clone --recursive):
cmake --preset vs2022-debug

# Build (Debug):
cmake --build --preset vs2022-debug -j16

# Build (Release):
cmake --build --preset vs2022-release -j16

# Run tests:
cd build/vs2022-debug && ctest --config Debug

# Run a single test via CTest:
cd build/vs2022-debug && ctest --config Debug -R "test_name_pattern"
```

Outputs go to `build/<preset>/bin/` (executables) and `build/<preset>/lib/` (static libraries). The three executables are `DolasEditor.exe`, `ShaderCompiler.exe`, and `DolasTest.exe`.

Note: README.md references `setup.bat`, `build-debug.bat`, and `build-release.bat` — these **do not exist** in the repo yet.

## Architecture Overview

```
DolasEditor (app)        ShaderCompiler (standalone tool, no engine deps)
    └─ DolasFunction          └─ d3dcompiler only
          ├─ DolasResource
          ├─ DolasCore
          └─ DolasPlatform
```

- **DolasPlatform** — Lowest layer. Window creation, D3D12 device initialization (`RenderHardwareInterface`), logging macros (LOG_TRACE/DEBUG/INFO/WARN/ERROR/CRITICAL wrapping spdlog), file system, thread pool.
- **DolasCore** — Fundamental types (`dolas_base.h` type aliases and typed IDs), math library (`dolas_math.h`), hash utilities (`STRING_ID` macro, `HashConverter`), path resolution (`PathUtils`).
- **DolasResource** — C++-defined asset system. `AssetManager` validates, loads, and caches reflected asset descriptions from XML. Built-in descriptions are `MaterialAssetDesc`, `MeshAssetDesc`, `EntityAssetDesc`, `CameraAssetDesc`, and `SceneAssetDesc`; serialized contracts use stable type IDs, schema versions, and field IDs.
- **DolasFunction** — The main engine library, organized into three areas:
  - **Manager layer** (18 manager classes): `DolasEngine` owns all managers via `g_dolas_engine` singleton. Key managers: `MeshManager`, `TextureManager`, `ShaderManager`, `MaterialManager`, `BufferManager`, `RenderSceneManager`, `RenderPipelineManager`, `ImGuiManager`, `TaskManager` (async via `ThreadPool`), `TimerManager`, `TickManager`.
  - **Render layer**: `DolasRHI` (D3D11-based for now), `RenderPipeline` (multi-pass: Clear → GBuffer → DeferredShading → ForwardShading → Skybox → Debug → ImGUI → PostProcess → Present), resource wrappers (`Buffer`, `Texture`, `Shader`, `Material`), transform/camera/entity/primitive/scene/view definitions.
  - **Editor layer**: `ContentBrowser` for asset browsing within the editor.

## DX11 → DX12 Migration (In Progress)

The engine is mid-migration. Current state:
- `dolas_platform/dolas_render_hardware_interface.cpp` links `d3d12.lib` and creates a D3D12 device + command queue
- `dolas_function/dolas_rhi.cpp` and all render layer code still use D3D11 exclusively (`d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`)
- See `docs/DX12迁移计划.md` for the full 8-phase migration plan (estimated 17-24 workdays total)

When implementing DX12 features, follow the RHI interface designs in the migration plan doc.

## Key Conventions

**Resource ID hashing** (see `docs/Developer.md`):
- `STRING_ID(x)` — compile-time hash of literal `#x`. Use for hardcoded names in source (e.g., `STRING_ID(sphere_render_primitive)`). Must NOT be used with runtime variables.
- `HashConverter::StringHash(runtime_string)` — runtime hash for file paths and dynamic strings.
- Logic/type/enum names → `STRING_ID`; real file paths → `HashConverter::StringHash`.

## Third-Party Dependencies

All in `third_party/` via git submodules: ImGui (docking), Assimp, DirectXTex, Tracy, Catch2, spdlog, tinyxml2, ImGuizmo. The repo uses forks under `github.com/yaomq19/` for most.

## Tests

Located in `src/engine_test/`. Uses Catch2 with `catch_discover_tests` for automatic CTest registration. Currently covers: asset manager, math (Vector2, Vector3), and path utilities. No render/manager tests yet.
