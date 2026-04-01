add_executable(display
    Src/main.c
    Src/stm32f0xx_it.c
    Src/stm32f0xx_hal_msp.c
    Src/ili9341_font.c
    Src/ili9341_gfx.c
    Src/ili9341.c
    Src/ili9341_ext.c
    Src/images.c
    Src/jpeg_decode.c
    Lib/tjpgd.c
)
set_target_properties(display PROPERTIES ADDITIONAL_CLEAN_FILES display.map)
target_include_directories(display PRIVATE
    Inc
    Lib
)
target_compile_definitions(display PRIVATE
)
target_link_libraries(display
    STM32_Drivers
)
flash_target(display)
