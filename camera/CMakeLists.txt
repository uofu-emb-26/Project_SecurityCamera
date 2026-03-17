# Add C source files here.
add_executable(camera
    Src/main.c
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
