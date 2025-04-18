cmake_minimum_required(VERSION 3.16)
project(Teapot)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES
    code/src/*.cpp
    code/src/shaderrenderers/*.cpp
	code/include/*.h
	code/include/shaderrenderers/*.h
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
	code/include/
	build/packages/Microsoft.DXSDK.D3DX.9.29.952.8/build/native/include
)

file(COPY assets/models DESTINATION ${CMAKE_BINARY_DIR})
file(COPY assets/textures DESTINATION ${CMAKE_BINARY_DIR})
file(COPY assets/shaders DESTINATION ${CMAKE_BINARY_DIR})
