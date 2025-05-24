cmake_minimum_required(VERSION 3.10)
project(TextEditor)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate Visual Studio solution
if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD TRUE)
endif()

# Find OpenGL package
find_package(OpenGL REQUIRED)

# Add GLFW submodule from vendor directory
add_subdirectory(vendor/glfw)

# ImGui doesn't have a CMakeLists.txt by default, so we need to set it up manually
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Create an ImGui library
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw ${OPENGL_LIBRARIES})

# Add source files for your project using your custom directory structure
file(GLOB_RECURSE PROJECT_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
)
file(GLOB_RECURSE PROJECT_HEADERS 
    "${CMAKE_CURRENT_SOURCE_DIR}/source/header/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/source/header/*.hpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source/header
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends

    ${OPENGL_INCLUDE_DIR}  # Add OpenGL include directory
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    imgui
    ${OPENGL_LIBRARIES}  # Link with OpenGL libraries
)

# Platform-specific OpenGL linkage
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE GL)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif()

# Copy DLLs to output directory for Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "TextEditor")