# To Do List
- 设计RenderPrimitive相关
    - 涉及到的类有：RenderPrimitive, Mesh, RenderEntity, RenderObject, Material, Shader
    - 要求有：
        - Draw 接口简洁明确
        - 各个类之间的关系明确，职能明确
        - 能实现网格共享，后面可以在网格共享的基础上进一步实现 BatchDraw 系统
        - RHI 资源的生命周期的管理要正确，避免出现显存泄露
- 实现DebugDrawManager，优先支持圆柱体的渲染，来实现世界坐标系
- 显示世界坐标系
- 实现时间系统
- HR宏优化
- Asset系统优化
- 制作简易RSD系统
- 制作C#工具项目，第一个工具：ShaderCompiler
    - 制作完成后，Shader 从此完全切换为离线编译
- 实现 Logic Render Swap Buffer
- 确定 spdlog 的线程安全性

# 2025-10-29
- 集成 ImGUI
# 2025-10-28
- 实现SkyBox完整功能
# 2025-10-25
- 使用vcpkg来管理第三方依赖
- 引入完整的DirectXTex库，从而创建HDR贴图
# 2025-10-22
- 完成SkyBox架构
# 2025-10-16
- 新增日志系统