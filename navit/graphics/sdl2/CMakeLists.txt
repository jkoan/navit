module_add_library(graphics_sdl2 graphics_sdl2.c)

target_link_libraries(graphics_sdl2 ${SDL2_LIBRARIES}  ${SDL2_GFX_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
