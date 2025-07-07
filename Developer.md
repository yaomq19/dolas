# 基本几何体管理器
需要一个基本几何体管理器 `GeometricPrimitivesManager`
以球体为例，一个球体需要一些顶点的集合，顶点包括 Local 空间的三维坐标，还需要UV信息
其他必要信息，如法线、albedo等通过UV坐标采样贴图获得

一个drawcall里面，顶点数据流由mesh文件确定、vs、ps代码以及引用的纹理贴图路径由 material.ast 文件确定、IA 在 发起 DC 时动态创建、renderState 由 C++ 代码在 pipeline 里确定。
一个 MeshRenderEntity 可以综合 mesh 文件和 material 文件。
mesh 和 material 要能匹配生效的关键在于 material 中 vs 的输入要和 mesh 的结构匹配，这样才能
正确创建出 IA

回到正题，先设计 MeshRenderEntity 对象
需要一个 draw 接口，传入 immediatecontext 进行渲染
要有 VertexBuffer 成员变量，在销毁时注意 Release
要有vs 代码和 ps 代码，要有 SRV 和 Texture 的指针
GeometricPrimitivesManager 可以返回一个 MeshRenderEntity 对象的指针，对象本身在 Manager 中进行管理

# 2025.6.15
- RenderView 类
- RenderViewManager 类
- RenderCamera 类
- RenderCameraManager 类
- RenderResource 类
- RenderResouceManager 类
- RenderScene 类
- RenderSceneManager 类