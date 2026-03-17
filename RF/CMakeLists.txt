# Add C source files here.
add_executable(RF
    Src/main.c
    Src/stm32f0xx_it.c
    Src/stm32f0xx_hal_msp.c
)

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(RF PROPERTIES ADDITIONAL_CLEAN_FILES RF.map)

# Add user defined include paths
target_include_directories(RF PRIVATE
    Inc
)

# Add project symbols (macros)
target_compile_definitions(RF PRIVATE
)

# Add linked libraries
target_link_libraries(RF
    STM32_Drivers
)

flash_target(RF)
