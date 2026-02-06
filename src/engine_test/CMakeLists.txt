add_executable(DolasTestMain main.cpp)
target_sources(DolasTestMain PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(DolasTestMain PRIVATE DolasCore)
set_target_properties(DolasTestMain PROPERTIES FOLDER "EngineRuntime/Test")