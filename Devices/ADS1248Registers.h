#ifndef _ADS_1248_REGISTERS_H_

#define _ADS_1248_REGISTERS_H_

#define ADS1248_REGISTER_MUX0                                                               0x00

    #define ADS1248_REGISTER_MUX0_BCS_MASK                                                  0x3F
    #define ADS1248_REGISTER_MUX0_BCS_OFF                                                   (0x00 << 6)
    #define ADS1248_REGISTER_MUX0_BCS_0_5_UA                                                (0x01 << 6)
    #define ADS1248_REGISTER_MUX0_BCS_2_UA                                                  (0x02 << 6)
    #define ADS1248_REGISTER_MUX0_BCS_10_UA                                                 (0x03 << 6)
    
    #define ADS1248_REGISTER_MUX0_MUX_SP_MASK                                               0xC7
    #define ADS1248_REGISTER_MUX0_MUX_SP_AIN0                                               (0x00 << 3)
    #define ADS1248_REGISTER_MUX0_MUX_SP_AIN1                                               (0x01 << 3)
    #define ADS1248_REGISTER_MUX0_MUX_SP_AIN2                                               (0x02 << 3)
    #define ADS1248_REGISTER_MUX0_MUX_SP_AIN3                                               (0x03 << 3)
    #define ADS1248_REGISTER_MUX0_MUX_SP_AIN4                                               (0x04 << 3)
    #define ADS1248_REGISTER_MUX0_MUX_SP_AIN5                                               (0x05 << 3)
    #define ADS1248_REGISTER_MUX0_MUX_SP_AIN6                                               (0x06 << 3)
    #define ADS1248_REGISTER_MUX0_MUX_SP_AIN7                                               (0x07 << 3)
    
    #define ADS1248_REGISTER_MUX0_MUX_SN_MASK                                               0xF8
    #define ADS1248_REGISTER_MUX0_MUX_SN_AIN0                                               0x00
    #define ADS1248_REGISTER_MUX0_MUX_SN_AIN1                                               0x01
    #define ADS1248_REGISTER_MUX0_MUX_SN_AIN2                                               0x02
    #define ADS1248_REGISTER_MUX0_MUX_SN_AIN3                                               0x03
    #define ADS1248_REGISTER_MUX0_MUX_SN_AIN4                                               0x04
    #define ADS1248_REGISTER_MUX0_MUX_SN_AIN5                                               0x05
    #define ADS1248_REGISTER_MUX0_MUX_SN_AIN6                                               0x06
    #define ADS1248_REGISTER_MUX0_MUX_SN_AIN7                                               0x07
    
#define ADS1248_REGISTER_VBIAS                                                              0x01

    #define ADS1248_REGISTER_VBIAS_VBIAS_MASK                                               0x00
    #define ADS1248_REGISTER_VBIAS_VBIAS_AIN0                                               0x01
    #define ADS1248_REGISTER_VBIAS_VBIAS_AIN1                                               0x02
    #define ADS1248_REGISTER_VBIAS_VBIAS_AIN2                                               0x04
    #define ADS1248_REGISTER_VBIAS_VBIAS_AIN3                                               0x08
    #define ADS1248_REGISTER_VBIAS_VBIAS_AIN4                                               0x10
    #define ADS1248_REGISTER_VBIAS_VBIAS_AIN5                                               0x20
    #define ADS1248_REGISTER_VBIAS_VBIAS_AIN6                                               0x40
    #define ADS1248_REGISTER_VBIAS_VBIAS_AIN7                                               0x80
    
#define ADS1248_REGISTER_MUX1                                                               0x02

    #define ADS1248_REGISTER_MUX1_CLKSTAT_MASK                                              0x7F
    #define ADS1248_REGISTER_MUX1_CLKSTAT_INTERNAL_OSCILLATOR                               (0 << 7)
    #define ADS1248_REGISTER_MUX1_CLKSTAT_EXTERNAL_OSCILLATOR                               (1 << 7)
    
    #define ADS1248_REGISTER_MUX1_VREFCON_MASK                                              0x9F
    #define ADS1248_REGISTER_MUX1_VREFCON_INTERNAL_REFERENCE_OFF                            (0x00 << 5)
    #define ADS1248_REGISTER_MUX1_VREFCON_INTERNAL_REFERENCE_ON                             (0x01 << 5)
    #define ADS1248_REGISTER_MUX1_VREFCON_INTERNAL_REFERENCE_ON_CONVERSION_IN_PROGRESS      (0x03 << 5)
    
    #define ADS1248_REGISTER_MUX1_REFSELT_MASK                                              0xE7
    #define ADS1248_REGISTER_MUX1_REFSELT_REF0                                              (0x00 << 3)
    #define ADS1248_REGISTER_MUX1_REFSELT_REF1                                              (0x01 << 3)
    #define ADS1248_REGISTER_MUX1_REFSELT_ONBOARD_REFERENCE                                 (0x02 << 3)
    #define ADS1248_REGISTER_MUX1_REFSELT_ONBOARD_REFERENCE_INTERNALLY_CONNECTED_TO_REF0    (0x03 << 3)
    
    #define ADS1248_REGISTER_MUX1_MUXCAL_MASK                                               0xFC
    #define ADS1248_REGISTER_MUX1_MUXCAL_NORMAL_OPERATION                                   0x00
    #define ADS1248_REGISTER_MUX1_MUXCAL_OFFSET_MEASUREMENT                                 0x01
    #define ADS1248_REGISTER_MUX1_MUXCAL_GAIN_MEASUREMENT                                   0x02
    #define ADS1248_REGISTER_MUX1_MUXCAL_TEMPERATURE_DIODE                                  0x03
    #define ADS1248_REGISTER_MUX1_MUXCAL_EXTERNAL_REF1_MEASUREMENT                          0x04
    #define ADS1248_REGISTER_MUX1_MUXCAL_EXTERNAL_REF0_MEASUREMENT                          0x05
    #define ADS1248_REGISTER_MUX1_MUXCAL_AVDD_MEASUREMENT                                   0x06
    #define ADS1248_REGISTER_MUX1_MUXCAL_DVDD_MEASUREMENT                                   0x07
    
#define ADS1248_REGISTER_SYS0                                                               0x03

    #define ADS1248_REGISTER_SYS0_MASK                                                      0x00
    #define ADS1248_REGISTER_SYS0_PGA_MASK                                                  0x8F
    #define ADS1248_REGISTER_SYS0_PGA_1                                                     (0x00 << 4)
    #define ADS1248_REGISTER_SYS0_PGA_2                                                     (0x01 << 4)
    #define ADS1248_REGISTER_SYS0_PGA_4                                                     (0x02 << 4)
    #define ADS1248_REGISTER_SYS0_PGA_8                                                     (0x03 << 4)
    #define ADS1248_REGISTER_SYS0_PGA_16                                                    (0x04 << 4)
    #define ADS1248_REGISTER_SYS0_PGA_32                                                    (0x05 << 4)
    #define ADS1248_REGISTER_SYS0_PGA_64                                                    (0x06 << 4)
    #define ADS1248_REGISTER_SYS0_PGA_128                                                   (0x07 << 4)
    
    #define ADS1248_REGISTER_SYS0_DOR_MASK                                                  0xF0
    #define ADS1248_REGISTER_SYS0_DOR_5_SPS                                                 0x00
    #define ADS1248_REGISTER_SYS0_DOR_10_SPS                                                0x01
    #define ADS1248_REGISTER_SYS0_DOR_20_SPS                                                0x02
    #define ADS1248_REGISTER_SYS0_DOR_40_SPS                                                0x03
    #define ADS1248_REGISTER_SYS0_DOR_80_SPS                                                0x04
    #define ADS1248_REGISTER_SYS0_DOR_160_SPS                                               0x05
    #define ADS1248_REGISTER_SYS0_DOR_320_SPS                                               0x06
    #define ADS1248_REGISTER_SYS0_DOR_640_SPS                                               0x07
    #define ADS1248_REGISTER_SYS0_DOR_1000_SPS                                              0x08
    #define ADS1248_REGISTER_SYS0_DOR_2000_SPS                                              0x09
    
#define ADS1248_REGISTER_OFC0                                                               0x04

    #define ADS1248_REGISTER_OFC0_MASK                                                      0x00
    
#define ADS1248_REGISTER_OFC1                                                               0x05

    #define ADS1248_REGISTER_OFC1_MASK                                                      0x00
    
#define ADS1248_REGISTER_OFC2                                                               0x06

    #define ADS1248_REGISTER_OFC2_MASK                                                      0x00

#define ADS1248_REGISTER_FSC0                                                               0x07

    #define ADS1248_REGISTER_FSC0_MASK                                                      0x00
    
#define ADS1248_REGISTER_FSC1                                                               0x08

    #define ADS1248_REGISTER_FSC1_MASK                                                      0x00
    
#define ADS1248_REGISTER_FSC2                                                               0x09

    #define ADS1248_REGISTER_FSC2_MASK                                                      0x00

#define ADS1248_REGISTER_IDAC0                                                              0x0A

    #define ADS1248_REGISTER_IDAC0_ID_MASK                                                  0x0F
    
    #define ADS1248_REGISTER_IDAC0_DRDY_MODE_MASK                                           0xF7
    #define ADS1248_REGISTER_IDAC0_DRDY_MODE_ONLY_DATA_OUT                                  (0x00 << 3)
    #define ADS1248_REGISTER_IDAC0_DRDY_MODE_BOTH_DATA_OUT_DATA_READY                       (0x01 << 3)
    
    #define ADS1248_REGISTER_IDAC0_IMAG_MASK                                                0xF8
    #define ADS1248_REGISTER_IDAC0_IMAG_OFF                                                 0x00
    #define ADS1248_REGISTER_IDAC0_IMAG_50_UA                                               0x01
    #define ADS1248_REGISTER_IDAC0_IMAG_100_UA                                              0x02
    #define ADS1248_REGISTER_IDAC0_IMAG_250_UA                                              0x03
    #define ADS1248_REGISTER_IDAC0_IMAG_500_UA                                              0x04
    #define ADS1248_REGISTER_IDAC0_IMAG_750_UA                                              0x05
    #define ADS1248_REGISTER_IDAC0_IMAG_1000_UA                                             0x06
    #define ADS1248_REGISTER_IDAC0_IMAG_1500_UA                                             0x07
    
#define ADS1248_REGISTER_IDAC1                                                              0x0B

    #define ADS1248_REGISTER_IDAC1_I1DIR_MASK                                               0x0F
    #define ADS1248_REGISTER_IDAC1_I1DIR_AIN0                                               (0x00 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_AIN1                                               (0x01 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_AIN2                                               (0x02 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_AIN3                                               (0x03 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_AIN4                                               (0x04 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_AIN5                                               (0x05 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_AIN6                                               (0x06 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_AIN7                                               (0x07 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_IEXT1                                              (0x08 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_IEXT2                                              (0x09 << 4)
    #define ADS1248_REGISTER_IDAC1_I1DIR_DISCONNECTED                                       (0xFF << 4)
    
    #define ADS1248_REGISTER_IDAC1_I2DIR_MASK                                               0x0F
    #define ADS1248_REGISTER_IDAC1_I2DIR_AIN0                                               0x00
    #define ADS1248_REGISTER_IDAC1_I2DIR_AIN1                                               0x01
    #define ADS1248_REGISTER_IDAC1_I2DIR_AIN2                                               0x02
    #define ADS1248_REGISTER_IDAC1_I2DIR_AIN3                                               0x03
    #define ADS1248_REGISTER_IDAC1_I2DIR_AIN4                                               0x04
    #define ADS1248_REGISTER_IDAC1_I2DIR_AIN5                                               0x05
    #define ADS1248_REGISTER_IDAC1_I2DIR_AIN6                                               0x06
    #define ADS1248_REGISTER_IDAC1_I2DIR_AIN7                                               0x07
    #define ADS1248_REGISTER_IDAC1_I2DIR_IEXT1                                              0x08
    #define ADS1248_REGISTER_IDAC1_I2DIR_IEXT2                                              0x09
    #define ADS1248_REGISTER_IDAC1_I2DIR_DISCONNECTED                                       0xFF

#define ADS1248_REGISTER_GPIOCFG                                                            0x0C

    #define ADS1248_REGISTER_GPIOCFG_MASK                                                   0x00

#define ADS1248_COMMAND_WAKEUP                                                              0x01
#define ADS1248_COMMAND_SLEEP                                                               0x03
#define ADS1248_COMMAND_SYNC                                                                0x05
#define ADS1248_COMMAND_RESET                                                               0x07
#define ADS1248_COMMAND_NOP                                                                 0xFF
#define ADS1248_COMMAND_RDATA                                                               0x13
#define ADS1248_COMMAND_RDATAC                                                              0x15
#define ADS1248_COMMAND_SDATAC                                                              0x17
#define ADS1248_COMMAND_RREG                                                                (0x02 << 4)
#define ADS1248_COMMAND_WREG                                                                (0x04 << 4)
#define ADS1248_COMMAND_SYSOCAL                                                             0x60
#define ADS1248_COMMAND_SYSGCAL                                                             0x61
#define ADS1248_COMMAND_SELFOCAL                                                            0x62

#define ADS1248_COMMAND_RREG_REG_ADDRESS_MASK                                               0x0F
#define ADS1248_COMMAND_WREG_REG_ADDRESS_MASK                                               0x0F

#endif
