# To Do List
- 定义 Dolas 的网格格式，并根据这个网格格式制作导入器（首先实现FBX导入器）
- 实现 pbr brdf standard shade mode
- 实现 SetNamedSampler 接口
- Shader 反射系统
- 实现DebugDrawManager，优先支持圆柱体的渲染，来实现世界坐标系
- 显示世界坐标系
- Asset系统优化
- 制作简易RSD系统
- 制作C#工具项目，第一个工具：ShaderCompiler
    - 制作完成后，Shader 从此完全切换为离线编译
- 实现 Logic Render Swap Buffer
- 确定 spdlog 的线程安全性
- 实现 ColorManager
- 设计RenderPrimitive相关
    - 涉及到的类有：RenderPrimitive, Mesh, RenderEntity, RenderObject, Material, Shader
    - 要求有：
        - Draw 接口简洁明确
        - 各个类之间的关系明确，职能明确
        - 能实现网格共享，后面可以在网格共享的基础上进一步实现 BatchDraw 系统
        - RHI 资源的生命周期的管理要正确，避免出现显存泄露


## 字符串 Hash 与资源 ID 约定
- **STRING_ID 宏**：`STRING_ID(x)` 等价于对编译期字面量 `#x` 做 Hash，只能用于源码里写死的名字（如 `STRING_ID(sphere_render_primitive)`），**不能**用于运行时变量。
- **运行时路径 / 文件名**：一律使用 `HashConverter::StringHash(runtime_string)`，例如 `HashConverter::StringHash(texture_file_path)`，避免所有资源拿到相同的 ID。
- **推荐实践**：
  - 代码中“逻辑名、资源名、枚举名”用 `STRING_ID`；
  - 磁盘上的“真实文件路径”用 `HashConverter::StringHash`；
  - 避免在 `CreateXXXFromFile` 这类函数中直接使用 `STRING_ID(变量名)`。