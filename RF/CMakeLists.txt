# TX
add_executable(RF_TX
    TX/Src/main.c
    TX/Src/stm32f0xx_it.c
    TX/Src/stm32f0xx_hal_msp.c
    TX/Src/nrf24l01p.c
    TX/Src/nrf24l01_ext.c
)
target_include_directories(RF_TX PRIVATE TX/Inc)
target_link_libraries(RF_TX STM32_Drivers)
flash_target(RF_TX)

# RX
add_executable(RF_RX
    RX/Src/main.c
    RX/Src/stm32f0xx_it.c
    RX/Src/stm32f0xx_hal_msp.c
    RX/Src/nrf24l01p.c
    RX/Src/nrf24l01_ext.c
)
target_include_directories(RF_RX PRIVATE RX/Inc)
target_link_libraries(RF_RX STM32_Drivers)
flash_target(RF_RX)