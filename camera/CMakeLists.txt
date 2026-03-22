# Add C source files here.
add_executable(camera
    Src/main.c
    Src/camera.c
    Src/ArduCAM.c
    Src/sccb_bus.c
    Src/spi.c
    Src/stm32f0xx_it.c
    Src/stm32f0xx_hal_msp.c
)

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(camera PROPERTIES ADDITIONAL_CLEAN_FILES camera.map)

# Add user defined include paths
target_include_directories(camera PRIVATE
    Inc
)

# Add project symbols (macros)
target_compile_definitions(camera PRIVATE
)

# Add linked libraries
target_link_libraries(camera
    STM32_Drivers
)

flash_target(camera)

# Debug target - UART streaming for Python verification
add_executable(camera_debug
    Src/main_uart_debug.c
    Src/ArduCAM.c
    Src/sccb_bus.c
    Src/spi.c
    Src/stm32f0xx_it.c
    Src/stm32f0xx_hal_msp.c
)
set_target_properties(camera_debug PROPERTIES ADDITIONAL_CLEAN_FILES camera_debug.map)
target_include_directories(camera_debug PRIVATE Inc)
target_compile_definitions(camera_debug PRIVATE)
target_link_libraries(camera_debug STM32_Drivers)
flash_target(camera_debug)
