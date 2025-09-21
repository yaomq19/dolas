# RenderObject 和 RenderModel 使用说明

## 📋 **概述**

我已经为您的Dolas引擎创建了两个新的渲染层次结构：

- **RenderObject**: 单个可渲染对象（带有变换信息的实体）
- **RenderModel**: 多个RenderObject的集合（复杂模型、层次结构）

## 🏗️ **架构层次**

```
RenderObject (单个对象)
    ├── RenderModel (对象的模型)
    │   └── RenderEntity (渲染实体)
    │       ├── Mesh (几何数据)
    │       └── Material (着色器+纹理)
    │       └── RigidBody (刚体)
    │       └── Skeleton (骨骼)
    │   └── 其他属性
    └── 其他属性
```

## 🔧 **基本使用方法**

### **1. 创建单个RenderObject**

```cpp
// 通过引擎管理器创建对象
RenderObjectID object_id = g_dolas_engine.m_render_object_manager->CreateRenderObjectFromEntity(
    "cube.entity",                    // 实体文件
    Vector3(0.0f, 1.0f, 0.0f),       // 位置
    Vector3(0.0f, 45.0f, 0.0f),      // 旋转（度）
    Vector3(1.0f, 1.0f, 1.0f)        // 缩放
);

// 获取对象并修改属性
RenderObject* obj = g_dolas_engine.m_render_object_manager->GetRenderObjectByID(object_id);
if (obj)
{
    obj->SetName("我的立方体");
    obj->SetVisible(true);
}
```

### **2. 创建复杂的RenderModel**

```cpp
// 创建一个汽车模型（示例）
RenderModelID car_model_id = g_dolas_engine.m_render_model_manager->CreateRenderModel("汽车模型");
RenderModel* car_model = g_dolas_engine.m_render_model_manager->GetRenderModelByID(car_model_id);

if (car_model)
{
    // 车身
    RenderObjectID body_id = g_dolas_engine.m_render_object_manager->CreateRenderObjectFromEntity("car_body.entity");
    car_model->AddRenderObject(body_id, Vector3(0.0f, 0.0f, 0.0f));
    
    // 前轮
    RenderObjectID front_wheel_left_id = g_dolas_engine.m_render_object_manager->CreateRenderObjectFromEntity("wheel.entity");
    car_model->AddRenderObject(front_wheel_left_id, Vector3(-1.0f, -0.5f, 1.5f));
    
    RenderObjectID front_wheel_right_id = g_dolas_engine.m_render_object_manager->CreateRenderObjectFromEntity("wheel.entity");
    car_model->AddRenderObject(front_wheel_right_id, Vector3(1.0f, -0.5f, 1.5f));
    
    // 后轮
    RenderObjectID rear_wheel_left_id = g_dolas_engine.m_render_object_manager->CreateRenderObjectFromEntity("wheel.entity");
    car_model->AddRenderObject(rear_wheel_left_id, Vector3(-1.0f, -0.5f, -1.5f));
    
    RenderObjectID rear_wheel_right_id = g_dolas_engine.m_render_object_manager->CreateRenderObjectFromEntity("wheel.entity");
    car_model->AddRenderObject(rear_wheel_right_id, Vector3(1.0f, -0.5f, -1.5f));
    
    // 设置整个汽车的位置
    car_model->SetPosition(Vector3(10.0f, 0.0f, 0.0f));
}
```

### **3. 在场景中渲染**

```cpp
// 在渲染循环中（例如在RenderView::Render中）
void SomeRenderFunction(DolasRHI* rhi)
{
    // 渲染所有对象
    g_dolas_engine.m_render_object_manager->DrawAllObjects(rhi);
    
    // 渲染所有模型
    g_dolas_engine.m_render_model_manager->DrawAllModels(rhi);
    
    // 或者带相机剪裁的渲染
    RenderCamera* camera = g_dolas_engine.m_render_camera_manager->GetRenderCameraByID(some_camera_id);
    if (camera)
    {
        g_dolas_engine.m_render_model_manager->DrawVisibleModels(rhi, camera);
    }
}
```

## 🎯 **高级功能**

### **LOD (Level of Detail) 管理**

```cpp
// 配置LOD等级
std::vector<RenderModelManager::LODConfig> lod_configs = {
    {50.0f, 0},   // 50单位内高精度
    {100.0f, 1},  // 100单位内中精度
    {200.0f, 2}   // 200单位内低精度
};

g_dolas_engine.m_render_model_manager->SetLODConfig(lod_configs);
g_dolas_engine.m_render_model_manager->EnableAutoLOD(true);

// 在更新循环中更新LOD
void Update(Float delta_time)
{
    RenderCamera* camera = g_dolas_engine.m_render_camera_manager->GetRenderCameraByID(camera_id);
    if (camera)
    {
        Vector3 camera_pos = camera->GetPosition();
        g_dolas_engine.m_render_model_manager->UpdateLOD(camera_pos);
    }
}
```

### **批量操作**

```cpp
// 隐藏所有对象
g_dolas_engine.m_render_object_manager->SetAllObjectsVisible(false);

// 清除所有模型
g_dolas_engine.m_render_model_manager->ClearAllModels();

// 获取统计信息
UInt object_count = g_dolas_engine.m_render_object_manager->GetObjectCount();
UInt model_count = g_dolas_engine.m_render_model_manager->GetModelCount();
```

## 🔍 **查找和管理**

```cpp
// 通过名称查找
RenderObject* obj = g_dolas_engine.m_render_object_manager->GetRenderObjectByName("我的立方体");
RenderModel* model = g_dolas_engine.m_render_model_manager->GetRenderModelByName("汽车模型");

// 获取所有ID
std::vector<RenderObjectID> all_objects = g_dolas_engine.m_render_object_manager->GetAllObjectIDs();
std::vector<RenderModelID> all_models = g_dolas_engine.m_render_model_manager->GetAllModelIDs();

// 删除对象
g_dolas_engine.m_render_object_manager->DestroyRenderObject(object_id);
g_dolas_engine.m_render_model_manager->DestroyRenderModel(model_id);
```

## 🎮 **与现有场景文件集成**

您可以通过编程方式从场景文件创建对象：

```cpp
// 加载default.scene中的实体
RenderObjectID plane_id = g_dolas_engine.m_render_object_manager->CreateRenderObjectFromEntity("plane.entity");
RenderObjectID cube_id = g_dolas_engine.m_render_object_manager->CreateRenderObjectFromEntity("cube.entity");

// 设置它们的变换（匹配场景文件）
RenderObject* plane = g_dolas_engine.m_render_object_manager->GetRenderObjectByID(plane_id);
RenderObject* cube = g_dolas_engine.m_render_object_manager->GetRenderObjectByID(cube_id);

if (plane) plane->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
if (cube) cube->SetPosition(Vector3(0.0f, 1.5f, 0.0f));
```

## ✅ **优势**

1. **层次化管理**: RenderModel可以管理复杂的多部件对象
2. **变换继承**: 移动RenderModel会影响所有子对象
3. **LOD支持**: 自动根据距离调整细节级别
4. **高效渲染**: 支持批量操作和视锥剪裁
5. **灵活扩展**: 易于添加动画、物理等功能

现在您可以使用这些新类来创建更复杂和高效的3D场景了！
