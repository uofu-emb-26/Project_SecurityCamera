add_executable(main_base_station
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
    Src/nrf24l01p.c
    Src/nrf24l01p_ext.c
)
set_target_properties(main_base_station PROPERTIES ADDITIONAL_CLEAN_FILES main_base_station.map)
target_include_directories(main_base_station PRIVATE
    Inc
    Lib
)
target_compile_definitions(main_base_station PRIVATE
)
target_link_libraries(main_base_station
    STM32_Drivers
)
flash_target(main_base_station)
