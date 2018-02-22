cmake_minimum_required(VERSION 3.1)

add_library(RenderGraph RenderGraph.h Factory.cpp Factory.h Queue.cpp Queue.h Pass.h PassDescriptor.h)

target_link_libraries(RenderGraph PUBLIC Graph)
target_include_directories(RenderGraph INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
