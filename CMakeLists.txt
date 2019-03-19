cmake_minimum_required(VERSION 3.9)

project(RenderGraph)

enable_testing()

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug) # Default
endif ()

add_subdirectory(external)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Werror=return-local-addr -Werror=return-type")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")
endif (NOT MSVC)

option(ENABLE_ASAN OFF)

if (ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address ")
endif (ENABLE_ASAN)

# OpenGL
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

find_path(HAVE_GL_COREARB_H "GL/glcorearb.h" PATHS ${OPENGL_INCLUDE_DIR} NO_DEFAULT_PATH)
find_path(HAVE_GL3_GL3_H "GL3/gl3.h" PATHS ${OPENGL_INCLUDE_DIR} NO_DEFAULT_PATH)
find_path(HAVE_GL_GL_H "GL/gl.h" PATHS ${OPENGL_INCLUDE_DIR} NO_DEFAULT_PATH)
find_path(HAVE_OPENGL_GL3_H "OpenGL/gl3.h" PATHS ${OPENGL_INCLUDE_DIR})
find_path(HAVE_OPENGL_GL_H "OpenGL/gl.h" PATHS ${OPENGL_INCLUDE_DIR})

configure_file(OpenGL.h.in OpenGL.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Sources
add_subdirectory(src)
add_subdirectory(example)

# Export / Install

set(EXPORT_PATH "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "Export path")

configure_file(RenderGraphConfig.cmake.in "${EXPORT_PATH}/RenderGraphConfig.cmake")

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if (is_multi_config)
    export(TARGETS RenderGraph Graph FILE "${EXPORT_PATH}/render-graph-targets.cmake")
else(is_multi_config)
    export(TARGETS RenderGraph Graph FILE "${EXPORT_PATH}/${CMAKE_BUILD_TYPE}/render-graph-targets.cmake")
endif(is_multi_config)

