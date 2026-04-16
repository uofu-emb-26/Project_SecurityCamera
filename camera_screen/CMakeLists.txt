# Add C source files here.
add_executable(camera_screen
    Src/main.c
    Src/camera.c
    Src/ArduCAM.c
    Src/sccb_bus.c
    Src/spi.c
    Src/stm32f0xx_it.c
    Src/stm32f0xx_hal_msp.c
    Src/ili9341_font.c
    Src/ili9341_gfx.c
    Src/ili9341.c
    Src/ili9341_ext.c
    Src/jpeg_decode.c
    Src/images.c
    Lib/tjpgd.c
)

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(camera_screen PROPERTIES ADDITIONAL_CLEAN_FILES camera_screen.map)

# Add user defined include paths
target_include_directories(camera_screen PRIVATE
    Inc
    Lib
)

# Add project symbols (macros)
target_compile_definitions(camera_screen PRIVATE
)

# Add linked libraries
target_link_libraries(camera_screen
    STM32_Drivers
)

flash_target(camera_screen)