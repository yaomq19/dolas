# ============ 第三方库静态库管理 ============
# 将所有第三方库编译成一个统一的静态库：dolas_third_party.lib

# ============ 源文件收集 ============
# 收集所有第三方库的源文件
set(THIRD_PARTY_SOURCES "")
set(THIRD_PARTY_HEADERS "")

# ============ DirectXTex 库 ============
message(STATUS "Adding DirectXTex library")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/DirectXTex")
    # 添加 DirectXTex 的源文件
    file(GLOB DIRECTXTEX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/DirectXTex/*.cpp")
    file(GLOB DIRECTXTEX_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/DirectXTex/*.h")
    
    list(APPEND THIRD_PARTY_SOURCES ${DIRECTXTEX_SOURCES})
    list(APPEND THIRD_PARTY_HEADERS ${DIRECTXTEX_HEADERS})
    
    message(STATUS "DirectXTex library found and added")
    foreach(source ${DIRECTXTEX_SOURCES})
        message(STATUS "  DirectXTex source: ${source}")
    endforeach()
else()
    message(WARNING "DirectXTex library not found")
endif()

# ============ nlohmann/json 库 ============
# nlohmann/json 是 header-only 库，不需要编译源文件
message(STATUS "Adding nlohmann/json library (header-only)")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann")
    # 只添加头文件，用于包含目录
    file(GLOB NLOHMANN_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann/*.hpp")
    list(APPEND THIRD_PARTY_HEADERS ${NLOHMANN_HEADERS})
    message(STATUS "nlohmann/json library found and added")
else()
    message(WARNING "nlohmann/json library not found")
endif()

# ============ ThreadPool 库 ============
# ThreadPool 是 header-only 库，不需要编译源文件
message(STATUS "Adding ThreadPool library (header-only)")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool")
    # 只添加头文件，用于包含目录
    file(GLOB THREADPOOL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool/*.h")
    list(APPEND THIRD_PARTY_HEADERS ${THREADPOOL_HEADERS})
    message(STATUS "ThreadPool library found and added")
else()
    message(WARNING "ThreadPool library not found")
endif()

# ============ 其他源文件库 ============
# 在这里添加其他需要编译的第三方库源文件
# 例如：
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/other_lib")
#     file(GLOB OTHER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/other_lib/*.cpp")
#     file(GLOB OTHER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/other_lib/*.h")
#     list(APPEND THIRD_PARTY_SOURCES ${OTHER_SOURCES})
#     list(APPEND THIRD_PARTY_HEADERS ${OTHER_HEADERS})
# endif()

# ============ 创建静态库 ============
if(THIRD_PARTY_SOURCES)
    # 创建 dolas_third_party 静态库
    add_library(dolas_third_party STATIC ${THIRD_PARTY_SOURCES} ${THIRD_PARTY_HEADERS})
    
    # 设置包含目录
    target_include_directories(dolas_third_party PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Windows 平台特定设置
    if(WIN32)
        target_link_libraries(dolas_third_party PRIVATE
            d3d11.lib
            dxgi.lib
            d3dcompiler.lib
        )
    endif()
    
    message(STATUS "dolas_third_party static library created successfully")
    message(STATUS "Total third-party source files: ${THIRD_PARTY_SOURCES}")
else()
    # 如果没有源文件，创建一个interface库用于头文件包含
    add_library(dolas_third_party INTERFACE)
    target_include_directories(dolas_third_party INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    message(STATUS "dolas_third_party interface library created (header-only)")
endif()

# ============ IDE文件组织 ============
# 为IDE组织文件结构
foreach(source IN LISTS THIRD_PARTY_SOURCES THIRD_PARTY_HEADERS)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" relative_source_path "${source_path}")
    string(REPLACE "/" "\\" group_path "${relative_source_path}")
    source_group("third_party${group_path}" FILES "${source}")
endforeach()

message(STATUS "Third-party libraries configuration completed") 