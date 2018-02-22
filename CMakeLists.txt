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

add_subdirectory(src)

# Export / Install

set(EXPORT_PATH "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "Export path")

configure_file(RenderGraphConfig.cmake.in "${EXPORT_PATH}/RenderGraphConfig.cmake")

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if (is_multi_config)
    export(TARGETS RenderGraph Graph FILE "${EXPORT_PATH}/render-graph-targets.cmake")
else(is_multi_config)
    export(TARGETS RenderGraph Graph FILE "${EXPORT_PATH}/${CMAKE_BUILD_TYPE}/render-graph-targets.cmake")
endif(is_multi_config)

