# Add C source files here.
add_executable(screen
    Src/main.c
    Src/stm32f0xx_it.c
    Src/stm32f0xx_hal_msp.c
)

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(screen PROPERTIES ADDITIONAL_CLEAN_FILES screen.map)

# Add user defined include paths
target_include_directories(screen PRIVATE
    Inc
)

# Add project symbols (macros)
target_compile_definitions(screen PRIVATE
)

# Add linked libraries
target_link_libraries(screen
    STM32_Drivers
)

flash_target(screen)
