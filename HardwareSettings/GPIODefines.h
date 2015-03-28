#ifndef _GPIO_DEFINES_H_
#define _GPIO_DEFINES_H_

/***************************I2C1***********************************/

#define I2C1_SCL_PORT                                   B
    #define I2C1_SCL_PIN                                6
    
#define I2C1_SDA_PORT                                   B
    #define I2C1_SDA_PIN                                7
    
/**************************SPI2************************************/

#define SPI2_MISO_PORT                                  C
    #define SPI2_MISO_PIN                               2
    
#define SPI2_MOSI_PORT                                  C
    #define SPI2_MOSI_PIN                               3
    
#define SPI2_SCK_PORT                                   B
    #define SPI2_SCK_PIN                                10
    
/**************************SPI3************************************/

#define SPI3_MISO_PORT                                  C
    #define SPI3_MISO_PIN                               11
    
#define SPI3_MOSI_PORT                                  C
    #define SPI3_MOSI_PIN                               12
    
#define SPI3_SCK_PORT                                   C
    #define SPI3_SCK_PIN                                10

/***************************UART 1*********************************/

#define UART1_TX_PORT                                   A
    #define UART1_TX_PIN                                9
    
#define UART1_RX_PORT                                   A
    #define UART1_RX_PIN                                10

/***************************UART 2*********************************/

#define UART2_TX_PORT                                   A
    #define UART2_TX_PIN                                2
    
#define UART2_RX_PORT                                   A
    #define UART2_RX_PIN                                3

/***************************DRV 595********************************/

#define DRV595_FAULT_PORT                               C
    #define DRV595_FAULT_PIN                            0
    
#define DRV595_SHUTDOWN_PORT                            C
    #define DRV595_SHUTDOWN_PIN                         1

/****************LMP90100 CONTROL SYSTEM***************************/

#define LMP90100_CONTROL_SYSTEM_CSB_PORT                B
    #define LMP90100_CONTROL_SYSTEM_CSB_PIN             0
    
#define LMP90100_CONTROL_SYSTEM_DRDYB_PORT              B
    #define LMP90100_CONTROL_SYSTEM_DRDYB_PIN           13
    
/*************LMP90100 SIGNALS MEASUREMENT*************************/

#define LMP90100_SIGNALS_MEASUREMENT_CSB_PORT           A
    #define LMP90100_SIGNALS_MEASUREMENT_CSB_PIN        4
    
#define LMP90100_SIGNALS_MEASUREMENT_DRDYB_PORT         B
    #define LMP90100_SIGNALS_MEASUREMENT_DRDYB_PIN      4

/**************************ADS1248*********************************/

#define ADS1248_CSB_PORT                                C
    #define ADS1248_CSB_PIN                             9
    
#define ADS1248_DRDY_PORT                               C
    #define ADS1248_DRDY_PIN                            8
    
#define ADS1248_START_PORT                              C
    #define ADS1248_START_PIN                           6
    
#define ADS1248_RESET_PORT                              B
    #define ADS1248_RESET_PIN                           9

/***************************LED************************************/

#define LED_PORT                                        A
    #define LED_PIN                                     5

#endif
