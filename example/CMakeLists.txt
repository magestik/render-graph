add_executable(RenderGraphSDL2 WIN32 main.cpp)
target_link_libraries(RenderGraphSDL2 PRIVATE RenderGraph)
target_link_libraries(RenderGraphSDL2 PRIVATE SDL2)
