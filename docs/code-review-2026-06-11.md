# Code Review Report - 2026-06-11

**Project Positioning:** L3 Team
**Review Scope:** 工作区全仓一方代码、配置、资源；`third_party` 仅审查依赖状态和集成。

## Critical Issues

### 1. RenderResource 在纹理创建失败后仍注册，且多个 ID 未初始化

**File:** `src/engine_runtime/dolas_function/public/include/render/dolas_render_resource.h:19`, `src/engine_runtime/dolas_function/private/src/manager/dolas_render_resource_manager.cpp:37`
**Rule:** CC-153 / PP-36

**Problem:** `RenderResource` 内的 `TextureID` 成员没有默认初始化；`CreateRenderResourceByID` 中每个纹理创建失败时只是不赋值，最后仍执行 `m_render_resources[render_resource_id] = render_resource` 并返回 `true`。

**Impact:** 任意 GBuffer、Depth、SceneResult 创建失败后，渲染管线会拿到随机 ID 或空资源，可能导致错误 RTV/DSV 绑定、黑屏或崩溃。

**Recommendation:** 所有资源 ID 默认初始化为 `*_EMPTY`；任意必需纹理创建失败时立即释放已创建资源并返回 `false`，不要注册半初始化对象。

**Effort:** Low to Medium
**Benefit:** High，直接保护核心渲染启动路径和失败边界。

## Important Issues

### 1. TextureManager 忽略 DirectXTex 加载失败，初始化仍返回成功

**File:** `src/engine_runtime/dolas_function/private/src/manager/dolas_texture_manager.cpp:561`, `src/engine_runtime/dolas_function/public/include/render/dolas_dx_trace.h:5`
**Rule:** CC-153 / PP-36

**Problem:** `Initialize` 无条件返回 `true`；`CreateTextureFromHDRFile/DDS/PNG` 通过 `HR(...)` 调用加载函数但不检查返回值，Release 下 `HR` 只是表达式执行。

**Impact:** 缺失或损坏的 skybox 等全局纹理会静默变成 `TEXTURE_ID_EMPTY`，后续 pass 继续运行，错误被延后到渲染阶段。

**Recommendation:** 捕获并检查 `HRESULT`，失败立即返回 `TEXTURE_ID_EMPTY`；必需全局纹理失败时 `Initialize` 返回 `false` 或显式绑定 fallback。

**Effort:** Low
**Benefit:** Medium，能把资源错误停在加载边界。

### 2. BufferManager 在 Clear 和创建失败路径泄漏 Buffer 对象

**File:** `src/engine_runtime/dolas_function/private/src/manager/dolas_buffer_manager.cpp:26`
**Rule:** PP-40

**Problem:** `Clear` 只调用 `buffer->Release()` 后清空 map，没有 `DOLAS_DELETE(buffer)`；多个 `Create*Buffer` 分支在 `CreateResource` 失败后直接返回，也没有删除刚分配的 `Buffer`。

**Impact:** 资源重建、失败重试或长时间编辑器会积累 CPU 侧对象泄漏。

**Recommendation:** 优先把 map 改为 `std::unique_ptr<Buffer>`；否则在失败路径和 `Clear` 中成对释放并删除。

**Effort:** Medium
**Benefit:** Medium，修复常用资源路径的生命周期问题。

### 3. RenderResourceManager::Clear 泄漏 RenderResource 对象

**File:** `src/engine_runtime/dolas_function/private/src/manager/dolas_render_resource_manager.cpp:20`
**Rule:** PP-40

**Problem:** `CreateRenderResourceByID` 使用 `DOLAS_NEW(RenderResource)`，但 `Clear` 直接 `m_render_resources.clear()`。

**Impact:** 渲染资源重建或引擎关闭时泄漏对象。

**Recommendation:** `Clear` 遍历 delete，或把容器所有权改为 `std::unique_ptr<RenderResource>`。

**Effort:** Low
**Benefit:** Medium，修复核心 manager 的所有权不一致。

### 4. RSD enum 解析把非法字符串静默映射为 0

**File:** `src/engine_runtime/dolas_resource/private/src/dolas_asset_manager.cpp:145`
**Rule:** CC-153 / PP-36

**Problem:** `EnumUInt` 字符串匹配失败后直接 `strtoul`，且不检查 `endptr`，非法枚举名会得到 `0`。

**Impact:** 资产 schema 或内容错误会被隐藏，例如非法 camera/material 枚举可能变成第一个合法值。

**Recommendation:** 使用 `strtoul` + `endptr` 严格校验；非法枚举应返回解析失败，并更新测试期望。

**Effort:** Low
**Benefit:** Medium，能更早暴露资产错误。

## Minor Issues

### 1. RenderPipelineManager 查询缺失 ID 时会污染 map

**File:** `src/engine_runtime/dolas_function/private/src/manager/dolas_render_pipeline_manager.cpp:39`
**Rule:** CC-153

**Problem:** `GetRenderPipelineByID` 使用 `m_render_pipelines[id]`，缺失 ID 会插入空项。

**Recommendation:** 改为 `find`，未命中返回 `nullptr`，与其他 manager getter 保持一致。

### 2. 仓库跟踪了编辑器备份文件

**File:** `src/engine_runtime/dolas_function/public/include/manager/dolas_render_primitive_manager.h~`
**Rule:** CC-58

**Problem:** `*.h~` 备份文件进入版本库，增加重复源码和误改风险。

**Recommendation:** 删除该 tracked 文件，并在 `.gitignore` 加入 `*~`。

## Verdict

**Needs fixes**

当前存在 1 个必须修复的渲染资源初始化/失败处理问题，以及多处资源生命周期和错误边界问题。建议先修复 Critical，再处理 `TextureManager`、`BufferManager`、`RenderResourceManager` 的生命周期与失败路径。
