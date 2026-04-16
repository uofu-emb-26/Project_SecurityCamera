add_executable(main_camera_module
    Src/main.c
    Src/stm32f0xx_it.c
    Src/stm32f0xx_hal_msp.c
    Src/nrf24l01p.c
    Src/nrf24l01p_ext.c
    Src/ArduCAM.c
    Src/camera.c
    Src/sccb_bus.c
    Src/spi.c
)
set_target_properties(main_camera_module PROPERTIES ADDITIONAL_CLEAN_FILES main_camera_module.map)
target_include_directories(main_camera_module PRIVATE
    Inc
    Lib
)
target_compile_definitions(main_camera_module PRIVATE
)
target_link_libraries(main_camera_module
    STM32_Drivers
)
flash_target(main_camera_module)
