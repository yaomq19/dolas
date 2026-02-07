#ifndef DOLAS_BASE_H
#define DOLAS_BASE_H

namespace Dolas
{
    // Base header file content will be here
    #define DOLAS_NEW(type, ...) new type(__VA_ARGS__)
    #define DOLAS_DELETE(ptr) if (ptr) { delete ptr; ptr = nullptr; }

    #define DOLAS_RETURN_IF_NULL(ptr) if (!ptr) { return; }
    #define DOLAS_RETURN_IF_FALSE(condition) if (!(condition)) { return; }
    #define DOLAS_RETURN_IF_TRUE(condition) if ((condition)) { return; }

    #define DOLAS_RETURN_TRUE_IF_NULL(ptr) if (!ptr) { return true; }
    #define DOLAS_RETURN_TRUE_IF_FALSE(condition) if (!(condition)) { return true; }
    #define DOLAS_RETURN_TRUE_IF_TRUE(condition) if ((condition)) { return true; }

    #define DOLAS_RETURN_FALSE_IF_NULL(ptr) if (!ptr) { return false; }
    #define DOLAS_RETURN_FALSE_IF_FALSE(condition) if (!(condition)) { return false; }
    #define DOLAS_RETURN_FALSE_IF_TRUE(condition) if ((condition)) { return false; }

    #define DOLAS_RETURN_NULL_IF_NULL(ptr) if (!ptr) { return nullptr; }
    #define DOLAS_RETURN_NULL_IF_FALSE(condition) if (!(condition)) { return nullptr; }
    #define DOLAS_RETURN_NULL_IF_TRUE(condition) if ((condition)) { return nullptr; }

    #define DOLAS_CONTINUE_IF_NULL(ptr) if (!ptr) { continue; }
    #define DOLAS_CONTINUE_IF_FALSE(condition) if (!(condition)) { continue; }
    #define DOLAS_CONTINUE_IF_TRUE(condition) if ((condition)) { continue; }

    #define DOLAS_STATIC_CONST static const

    typedef int Int;
    typedef float Float;
    typedef double Double;
    typedef bool Bool;
    typedef unsigned long ULong;
    typedef unsigned int UInt;
    typedef unsigned long long ULongLong;
    typedef unsigned char UByte;
    
    typedef UInt StringID;
    typedef UInt FileID;
    typedef UInt MaterialID;
    typedef UInt TextureID;
    typedef UInt BufferID;
    typedef UInt ShaderID;
    typedef UInt RenderEntityID;
    typedef UInt RenderObjectID;
    typedef UInt RenderPrimitiveID;

    typedef UInt RenderViewID;
    typedef UInt RenderPipelineID;
    typedef UInt RenderResourceID;
    typedef UInt RenderCameraID;
    typedef UInt RenderSceneID;
    
    inline constexpr StringID STRING_ID_EMPTY = static_cast<StringID>(0);
    inline constexpr FileID FILE_ID_EMPTY = static_cast<FileID>(0);
    inline constexpr MaterialID MATERIAL_ID_EMPTY = static_cast<MaterialID>(0);
    inline constexpr TextureID TEXTURE_ID_EMPTY = static_cast<TextureID>(0);
    inline constexpr BufferID BUFFER_ID_EMPTY = static_cast<BufferID>(0);
    inline constexpr ShaderID SHADER_ID_EMPTY = static_cast<ShaderID>(0);
    inline constexpr RenderEntityID RENDER_ENTITY_ID_EMPTY = static_cast<RenderEntityID>(0);
    inline constexpr RenderObjectID RENDER_OBJECT_ID_EMPTY = static_cast<RenderObjectID>(0);
    inline constexpr RenderResourceID RENDER_RESOURCE_ID_EMPTY = static_cast<RenderResourceID>(0);
    inline constexpr RenderPrimitiveID RENDER_PRIMITIVE_ID_EMPTY = static_cast<RenderPrimitiveID>(0);
    inline constexpr RenderCameraID RENDER_CAMERA_ID_EMPTY = static_cast<RenderCameraID>(0);
    inline constexpr RenderSceneID RENDER_SCENE_ID_EMPTY = static_cast<RenderSceneID>(0);
    inline constexpr RenderViewID RENDER_VIEW_ID_EMPTY = static_cast<RenderViewID>(0);
    inline constexpr RenderPipelineID RENDER_PIPELINE_ID_EMPTY = static_cast<RenderPipelineID>(0);
}
    
#endif // DOLAS_BASE_H

