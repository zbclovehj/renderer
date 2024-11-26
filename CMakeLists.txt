# 要求最低Cmake版本
cmake_minimum_required(VERSION 3.15)
# 项目名称
project(RGS LANGUAGES CXX)
# C++标准
set(CMAKE_CXX_STANDARD 17)

set(HEADERS
"src/Application.h"
 "src/Window.h"
  "src/WindowsWindow.h" 
  "src/base.h" "src/Framebuffer.h" "src/InputCodes.h" "src/Maths.h" "src/ShaderBase.h" "src/Texture.h" "src/BlinnShader.h" "src/Renderer.h")

set(SOURCES
       "src/Main.cpp"
 "src/Application.cpp"
 "src/Window.cpp"
 "src/WindowsWindow.cpp"
  "src/Framebuffer.cpp" "src/Math.cpp" "src/Renderer.cpp" "src/BlinnShader.cpp" "src/Texture.cpp" "src/stb_image.cpp")

set(INCLUDE_PATH "src")

set(TARGET RGS)

add_executable(${TARGET} ${HEADERS} ${SOURCES})
target_include_directories(${TARGET} PRIVATE ${INCLUDE_PATH})