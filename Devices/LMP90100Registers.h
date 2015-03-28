#ifndef _LMP90100_REGISTERS_H_

#define _LMP90100_REGISTERS_H_

#define LMP90100_REGISTERS_COUNT                                80U

// URA SETUP

#define LMP90100_URA_WRITE                                      0x10
#define LMP90100_URA_READ                                       0x90

// DATA ACCESS

#define LMP90100_DA_WRITE                                       (0 << 7)
#define LMP90100_DA_READ                                        (1 << 7)

#define LMP90100_DA_ONE_BYTE_READ                               (0x00 << 5)
#define LMP90100_DA_TWO_BYTES_READ                              (0x01 << 5)
#define LMP90100_DA_THREE_BYTES_READ                            (0x02 << 5)
#define LMP90100_DA_STREAMING_READ                              (0x03 << 5)

// REGISTERS

#define LMP90100_REG_RESETCN                                    0x00
#define LMP90100_REG_RESETCN_DEFAULT                            0x00
    #define LMP90100_REG_AND_CNV_RST                            0xC3
    
#define LMP90100_REG_SPI_RESET                                  0x02
#define LMP90100_REG_SPI_RESET_DEFAULT                          0x00
    #define LMP90100_SPI_RST_DISABLED                           0x00
    #define LMP90100_SPI_RST_ENABLED                            0x01
    
#define LMP90100_REG_PWRCN                                      0x08
#define LMP90100_REG_PWRCN_DEFAULT                              0x00
    #define LMP90100_PWRCN_ACTIVE_MODE                          0x00
    #define LMP90100_PWRCN_POWER_DOWN_MODE                      0x01
    #define LMP90100_PWRCN_STAND_BY_MODE                        0x03
    
#define LMP90100_REG_ADC_RESTART                                0x0B
#define LMP90100_REG_ADC_RESTART_DEFAULT                        0x00
    #define LMP90100_ADC_RESTART_DISABLED                       0x00
    #define LMP90100_ADC_RESTART_ENABLED                        0x01
    
#define LMP90100_REG_ADC_AUXCN                                  0x12
#define LMP90100_REG_ADC_AUXCN_DEFAULT                          0x00
    #define LMP90100_ADC_AUXCN_RESET_SYSCAL_NO                  (0 << 6)
    #define LMP90100_ADC_AUXCN_RESET_SYSCAL_YES                 (1 << 6)
    #define LMP90100_ADC_AUXCN_CLK_EXT_DET_OPERATIONAL          (0 << 5)
    #define LMP90100_ADC_AUXCN_CLK_EXT_DET_BYPASSED             (1 << 5)
    #define LMP90100_ADC_AUXCN_CLK_SEL_INTERNAL                 (0 << 4)
    #define LMP90100_ADC_AUXCN_CLK_SEL_EXTERNAL                 (1 << 4)
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_0UA                  0x00
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_100UA                0x01
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_200UA                0x02
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_300UA                0x03
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_400UA                0x04
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_500UA                0x05
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_600UA                0x06
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_700UA                0x07
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_800UA                0x08
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_900UA                0x09
    #define LMP90100_ADC_AUXCN_RTD_CUR_SEL_1000UA               0x0A
    
#define LMP90100_REG_ADC_DONE                                   0x18
#define LMP90100_REG_ADC_DONE_DEFAULT                           0xFF
    #define LMP90100_ADC_DONE_DT_AVAIL_B_AVAILABLE              0x00
    #define LMP90100_ADC_DONE_DT_AVAIL_B_NOT_AVAILABLE          0xFF
    
#define LMP90100_REG_ADC_DOUTH                                  0x1A
#define LMP90100_REG_ADC_DOUTM                                  0x1B
#define LMP90100_REG_ADC_DOUTL                                  0x1C

#define LMP90100_REG_CH_STS                                     0x1E
#define LMP90100_REG_CH_STS_DEFAULT                             0x00
    #define LMP90100_CH_STS_CH_SCAN_NRDY_BIT                    1
    #define LMP90100_CH_STS_CH_SCAN_NRDY_OKAY                   (0 << 1)
    #define LMP90100_CH_STS_CH_SCAN_NRDY_NOT_READY              (1 << 1)
    #define LMP90100_CH_STS_INV_OR_RPT_RD_STS_VALID             0x00
    #define LMP90100_CH_STS_INV_OR_RPT_RD_STS_INVALID           0x01
    
#define LMP90100_REG_CH_SCAN                                    0x1F
#define LMP90100_REG_CH_SCAN_DEFAULT                            0x00
    #define LMP90100_CH_SCAN_CH_SCAN_SEL_RESET                  0x3F
    #define LMP90100_CH_SCAN_CH_SCAN_SEL_SCANMODE0              (0x00 << 6)
    #define LMP90100_CH_SCAN_CH_SCAN_SEL_SCANMODE1              (0x01 << 6)
    #define LMP90100_CH_SCAN_CH_SCAN_SEL_SCANMODE2              (0x02 << 6)
    #define LMP90100_CH_SCAN_CH_SCAN_SEL_SCANMODE3              (0x03 << 6)
    #define LMP90100_CH_SCAN_LAST_CH_CH0                        (0x00 << 3)
    #define LMP90100_CH_SCAN_LAST_CH_CH1                        (0x01 << 3)
    #define LMP90100_CH_SCAN_LAST_CH_CH2                        (0x02 << 3)
    #define LMP90100_CH_SCAN_LAST_CH_CH3                        (0x03 << 3)
    #define LMP90100_CH_SCAN_LAST_CH_CH4                        (0x04 << 3)
    #define LMP90100_CH_SCAN_LAST_CH_CH5                        (0x05 << 3)
    #define LMP90100_CH_SCAN_LAST_CH_CH6                        (0x06 << 3)
    #define LMP90100_CH_SCAN_FIRST_CH_CH0                       0x00
    #define LMP90100_CH_SCAN_FIRST_CH_CH1                       0x01
    #define LMP90100_CH_SCAN_FIRST_CH_CH2                       0x02
    #define LMP90100_CH_SCAN_FIRST_CH_CH3                       0x03
    #define LMP90100_CH_SCAN_FIRST_CH_CH4                       0x04
    #define LMP90100_CH_SCAN_FIRST_CH_CH5                       0x05
    #define LMP90100_CH_SCAN_FIRST_CH_CH6                       0x06
    
#define LMP90100_REG_CHx_INPUTCN_CH0                            0x20
#define LMP90100_REG_CHx_INPUTCN_CH0_DEFAULT                    0x01
#define LMP90100_REG_CHx_INPUTCN_CH1                            0x22
#define LMP90100_REG_CHx_INPUTCN_CH1_DEFAULT                    0x13
#define LMP90100_REG_CHx_INPUTCN_CH2                            0x24
#define LMP90100_REG_CHx_INPUTCN_CH2_DEFAULT                    0x25
#define LMP90100_REG_CHx_INPUTCN_CH3                            0x26
#define LMP90100_REG_CHx_INPUTCN_CH3_DEFAULT                    0x37
#define LMP90100_REG_CHx_INPUTCN_CH4                            0x28
#define LMP90100_REG_CHx_INPUTCN_CH4_DEFAULT                    0x01
#define LMP90100_REG_CHx_INPUTCN_CH5                            0x2A
#define LMP90100_REG_CHx_INPUTCN_CH5_DEFAULT                    0x13
#define LMP90100_REG_CHx_INPUTCN_CH6                            0x2C
#define LMP90100_REG_CHx_INPUTCN_CH6_DEFAULT                    0x25
    #define LMP90100_CHx_INPUTCN_BURNOUT_EN_DISABLED            (0 << 7)
    #define LMP90100_CHx_INPUTCN_BURNOUT_EN_ENABLED             (1 << 7)
    #define LMP90100_CHx_INPUTCN_VREF_SEL_1                     (0 << 6)
    #define LMP90100_CHx_INPUTCN_VREF_SEL_2                     (1 << 6)
    #define LMP90100_CHx_INPUTCN_VINP_VIN0                      (0x00 << 3)
    #define LMP90100_CHx_INPUTCN_VINP_VIN1                      (0x01 << 3)
    #define LMP90100_CHx_INPUTCN_VINP_VIN2                      (0x02 << 3)
    #define LMP90100_CHx_INPUTCN_VINP_VIN3                      (0x03 << 3)
    #define LMP90100_CHx_INPUTCN_VINP_VIN4                      (0x04 << 3)
    #define LMP90100_CHx_INPUTCN_VINP_VIN5                      (0x05 << 3)
    #define LMP90100_CHx_INPUTCN_VINP_VIN6                      (0x06 << 3)
    #define LMP90100_CHx_INPUTCN_VINP_VIN7                      (0x07 << 3)
    #define LMP90100_CHx_INPUTCN_VINN_VIN0                      0x00
    #define LMP90100_CHx_INPUTCN_VINN_VIN1                      0x01
    #define LMP90100_CHx_INPUTCN_VINN_VIN2                      0x02
    #define LMP90100_CHx_INPUTCN_VINN_VIN3                      0x03
    #define LMP90100_CHx_INPUTCN_VINN_VIN4                      0x04
    #define LMP90100_CHx_INPUTCN_VINN_VIN5                      0x05
    #define LMP90100_CHx_INPUTCN_VINN_VIN6                      0x06
    #define LMP90100_CHx_INPUTCN_VINN_VIN7                      0x07
    
#define LMP90100_REG_CHx_CONFIG_CH0                             0x21
#define LMP90100_REG_CHx_CONFIG_CH0_DEFAULT                     0x70
#define LMP90100_REG_CHx_CONFIG_CH1                             0x23
#define LMP90100_REG_CHx_CONFIG_CH1_DEFAULT                     0x70
#define LMP90100_REG_CHx_CONFIG_CH2                             0x25
#define LMP90100_REG_CHx_CONFIG_CH2_DEFAULT                     0x70
#define LMP90100_REG_CHx_CONFIG_CH3                             0x27
#define LMP90100_REG_CHx_CONFIG_CH3_DEFAULT                     0x70
#define LMP90100_REG_CHx_CONFIG_CH4                             0x29
#define LMP90100_REG_CHx_CONFIG_CH4_DEFAULT                     0x70
#define LMP90100_REG_CHx_CONFIG_CH5                             0x2B
#define LMP90100_REG_CHx_CONFIG_CH5_DEFAULT                     0x70
#define LMP90100_REG_CHx_CONFIG_CH6                             0x2D
#define LMP90100_REG_CHx_CONFIG_CH6_DEFAULT                     0x70
    #define LMP90100_CHx_CONFIG_ODR_SEL_RESET                   0x8F
    #define LMP90100_CHx_CONFIG_ODR_SEL_1_6775_SPS              (0x00 << 4)
    #define LMP90100_CHx_CONFIG_ODR_SEL_3_355_SPS               (0x01 << 4)
    #define LMP90100_CHx_CONFIG_ODR_SEL_6_71_SPS                (0x02 << 4)
    #define LMP90100_CHx_CONFIG_ODR_SEL_13_42_SPS               (0x03 << 4)
    #define LMP90100_CHx_CONFIG_ODR_SEL_26_83125_SPS            (0x04 << 4)
    #define LMP90100_CHx_CONFIG_ODR_SEL_53_6625_SPS             (0x05 << 4)
    #define LMP90100_CHx_CONFIG_ODR_SEL_107_325_SPS             (0x06 << 4)
    #define LMP90100_CHx_CONFIG_ODR_SEL_214_65_SPS              (0x07 << 4)
    #define LMP90100_CHx_CONFIG_GAIN_SEL_1_FGA_OFF              (0x00 << 1)
    #define LMP90100_CHx_CONFIG_GAIN_SEL_2_FGA_OFF              (0x01 << 1)
    #define LMP90100_CHx_CONFIG_GAIN_SEL_4_FGA_OFF              (0x02 << 1)
    #define LMP90100_CHx_CONFIG_GAIN_SEL_8_FGA_OFF              (0x03 << 1)
    #define LMP90100_CHx_CONFIG_GAIN_SEL_16_FGA_ON              (0x04 << 1)
    #define LMP90100_CHx_CONFIG_GAIN_SEL_32_FGA_ON              (0x05 << 1)
    #define LMP90100_CHx_CONFIG_GAIN_SEL_64_FGA_ON              (0x06 << 1)
    #define LMP90100_CHx_CONFIG_GAIN_SEL_128_FGA_ON             (0x07 << 1)
    #define LMP90100_CHx_CONFIG_BUF_EN_BUFFER_INCLUDED          0x00
    #define LMP90100_CHx_CONFIG_BUF_EN_BUFFER_EXCLUDED          0x01
    
#define LMP90100_REG_BGCALCN                                    0x10
#define LMP90100_REG_BGCALCN_DEFAULT                            0x10
    #define LMP90100_BGCALN_BACKGROUND_CALIBRATION_OFF          0x00
    #define LMP90100_BGCALN_OFFSET_CORRECTION_GAIN_ESTIMATION   0x01
    #define LMP90100_BGCALN_OFFSET_CORRECTION_GAIN_CORRECTION   0x02
    #define LMP90100_BGCALN_OFFSET_ESTIMATION_GAIN_ESTIMATION   0x03
    
#define LMP90100_REG_SCALCN                                     0x17
#define LMP90100_REG_SCALCN_DEFAULT                             0x00
    #define LMP90100_SCALCN_NORMAL_MODE                         0x00
    #define LMP90100_SCALCN_OFFSET_COEFFICIENT_DETERMINATION    0x01
    #define LMP90100_SCALCN_GAIN_COEFFICIENT_DETERMINATION      0x02
    
#define LMP90100_REG_CHx_SCAL_OFFSETH_CH0                       0x30
#define LMP90100_REG_CHx_SCAL_OFFSETH_CH0_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETH_CH1                       0x38
#define LMP90100_REG_CHx_SCAL_OFFSETH_CH1_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETH_CH2                       0x40
#define LMP90100_REG_CHx_SCAL_OFFSETH_CH2_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETH_CH3                       0x48
#define LMP90100_REG_CHx_SCAL_OFFSETH_CH3_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETM_CH0                       0x31
#define LMP90100_REG_CHx_SCAL_OFFSETM_CH0_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETM_CH1                       0x39
#define LMP90100_REG_CHx_SCAL_OFFSETM_CH1_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETM_CH2                       0x41
#define LMP90100_REG_CHx_SCAL_OFFSETM_CH2_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETM_CH3                       0x49
#define LMP90100_REG_CHx_SCAL_OFFSETM_CH3_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETL_CH0                       0x32
#define LMP90100_REG_CHx_SCAL_OFFSETL_CH0_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETL_CH1                       0x3A
#define LMP90100_REG_CHx_SCAL_OFFSETL_CH1_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETL_CH2                       0x42
#define LMP90100_REG_CHx_SCAL_OFFSETL_CH2_DEFAULT               0x00
#define LMP90100_REG_CHx_SCAL_OFFSETL_CH3                       0x4A
#define LMP90100_REG_CHx_SCAL_OFFSETL_CH3_DEFAULT               0x00

#define LMP90100_REG_CHx_SCAL_GAINH_CH0                         0x33
#define LMP90100_REG_CHx_SCAL_GAINH_CH0_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINH_CH1                         0x3B
#define LMP90100_REG_CHx_SCAL_GAINH_CH1_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINH_CH2                         0x43
#define LMP90100_REG_CHx_SCAL_GAINH_CH2_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINH_CH3                         0x4B
#define LMP90100_REG_CHx_SCAL_GAINH_CH3_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINM_CH0                         0x34
#define LMP90100_REG_CHx_SCAL_GAINM_CH0_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINM_CH1                         0x3C
#define LMP90100_REG_CHx_SCAL_GAINM_CH1_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINM_CH2                         0x44
#define LMP90100_REG_CHx_SCAL_GAINM_CH2_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINM_CH3                         0x4C
#define LMP90100_REG_CHx_SCAL_GAINM_CH3_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINL_CH0                         0x35
#define LMP90100_REG_CHx_SCAL_GAINL_CH0_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINL_CH1                         0x3D
#define LMP90100_REG_CHx_SCAL_GAINL_CH1_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINL_CH2                         0x45
#define LMP90100_REG_CHx_SCAL_GAINL_CH2_DEFAULT                 0x80
#define LMP90100_REG_CHx_SCAL_GAINL_CH3                         0x4D
#define LMP90100_REG_CHx_SCAL_GAINL_CH3_DEFAULT                 0x80

#define LMP90100_REG_CHx_SCAL_SCALING_CH0                       0x36
#define LMP90100_REG_CHx_SCAL_SCALING_CH0_DEFAULT               0x01
#define LMP90100_REG_CHx_SCAL_SCALING_CH1                       0x3E
#define LMP90100_REG_CHx_SCAL_SCALING_CH1_DEFAULT               0x01
#define LMP90100_REG_CHx_SCAL_SCALING_CH2                       0x46
#define LMP90100_REG_CHx_SCAL_SCALING_CH2_DEFAULT               0x01
#define LMP90100_REG_CHx_SCAL_SCALING_CH3                       0x4E
#define LMP90100_REG_CHx_SCAL_SCALING_CH3_DEFAULT               0x01

#define LMP90100_REG_CHx_SCAL_BITS_SELECTOR_CH0                 0x37
#define LMP90100_REG_CHx_SCAL_BITS_SELECTOR_CH0_DEFAULT         0x00
#define LMP90100_REG_CHx_SCAL_BITS_SELECTOR_CH1                 0x3F
#define LMP90100_REG_CHx_SCAL_BITS_SELECTOR_CH1_DEFAULT         0x00
#define LMP90100_REG_CHx_SCAL_BITS_SELECTOR_CH2                 0x47
#define LMP90100_REG_CHx_SCAL_BITS_SELECTOR_CH2_DEFAULT         0x00
#define LMP90100_REG_CHx_SCAL_BITS_SELECTOR_CH3                 0x4F
#define LMP90100_REG_CHx_SCAL_BITS_SELECTOR_CH3_DEFAULT         0x00

#define LMP90100_REG_SENDIAG_THLDH                              0x14
#define LMP90100_REG_SENDIAG_THLDH_DEFAULT                      0x00
#define LMP90100_REG_SENDIAG_THLDL                              0x15
#define LMP90100_REG_SENDIAG_THLDL_DEFAULT                      0x00

#define LMP90100_REG_SENDIAG_FLAGS                              0x19
    #define LMP90100_SENDIAG_FLAGS_SHORT_THLD_FLAG              (1 << 7)
    #define LMP90100_SENDIAG_FLAGS_SHORT_RAILS_FLAG             (1 << 6)
    #define LMP90100_SENDIAG_FLAGS_POR_AFT_LST_RD               (1 << 5)
    #define LMP90100_SENDIAG_FLAGS_OFLO_FLAGS_NORMAL            (0x00 << 3)
    #define LMP90100_SENDIAG_FLAGS_OFLO_FLAGS_OVERARRANGED      (0x01 << 3)
    #define LMP90100_SENDIAG_FLAGS_OFLO_FLAGS_OVER_RANGED_PLUS  (0x02 << 3)
    #define LMP90100_SENDIAG_FLAGS_OFLO_FLAGS_OVER_RANGED_MINUS (0x03 << 3)
    #define LMP90100_SENDIAG_FLAGS_OFLO_SAMPLED_CHANNEL_MASK    0x07
    
#define LMP90100_REG_SPI_HANDSHAKECN                            0x01
#define LMP90100_REG_SPI_HANDSHAKECN_DEFAULT                    0x00
    #define LMP90100_SPI_HANDSHAKECN_DRDY_DRIVING_HIGH_Z        (0x00 << 1)
    #define LMP90100_SPI_HANDSHAKECN_DRDY_DRIVING_DRDY_DRIVING  (0x03 << 1)
    #define LMP90100_SPI_HANDSHAKECN_HIGH_Z_HIGH_Z              (0x04 << 1)
    #define LMP90100_SPI_HANDSHAKECN_SW_OFF_TRG_HIGH_Z          0x00
    #define LMP90100_SPI_HANDSHAKECN_SW_OFF_TRG_POSTPONED       0x01
    
#define LMP90100_REG_SPI_STREAMCN                               0x03
#define LMP90100_REG_SPI_STREAMCN_DEFAULT                       0x00
    #define LMP90100_SPI_STREAMCN_STRM_TYPE_NORMAL_MODE         (0x00 << 7)
    #define LMP90100_SPI_STREAMCN_STRM_TYPE_CONTROLLED_MODE     (0x01 << 7)
    #define LMP90100_SPI_STREAMCN_STRM_RANGE                    0x00
    
#define LMP90100_REG_DATA_ONLY_1                                0x09
#define LMP90100_REG_DATA_ONLY_1_DEFAULT                        0x1A
    #define LMP90100_DATA_ONLY_1_DATA_ONLY_ADR                  0x00
    
#define LMP90100_REG_DATA_ONLY_2                                0x0A
#define LMP90100_REG_DATA_ONLY_2_DEFAULT                        0x02
    #define LMP90100_DATA_ONLY_2_DATA_ONLY_SZ                   0x00
    
#define LMP90100_REG_SPI_DRDYBCN                                0x11
#define LMP90100_REG_SPI_DRDYBCN_DEFAULT                        0x03
    #define LMP90100_SPI_DRDYBCN_SPI_DRDYB_D6_GPIO              (0x00 << 7)
    #define LMP90100_SPI_DRDYBCN_SPI_DRDYB_D6_DRDYB_SIGNAL      (0x01 << 7)
    #define LMP90100_SPI_DRDYBCN_CRC_RST_ENABLED                (0x00 << 5)
    #define LMP90100_SPI_DRDYBCN_CRC_RST_DISABLED               (0x01 << 5)
    #define LMP90100_SPI_DRDYBCN_FGA_BGCAL_DEFAULT              (0x00 << 3)
    #define LMP90100_SPI_DRDYBCN_FGA_BGCAL_LAST_KNOWN           (0x01 << 3)
    
#define LMP90100_REG_SPI_CRC_CN                                 0x13
#define LMP90100_REG_SPI_CRC_CN_DEFAULT                         0x02
    #define LMP90100_SPI_SRC_CN_EN_CRC_DISABLED                 (0x00 << 4)
    #define LMP90100_SPI_SRC_CN_EN_CRC_ENABLED                  (0x01 << 4)
    #define LMP90100_SPI_SRC_CN_EN_CRC_DRDYB_AFT_CRC_DEFAULT    (0x00 << 2)
    #define LMP90100_SPI_SRC_CN_EN_CRC_DRDYB_AFT_CRC_CRC        (0x01 << 2)
    
#define LMP90100_REG_SPI_SRC_DAT                                0x1D

#define LMP90100_REG_GPIO_DIRCN                                 0x0E
#define LMP90100_REG_GPIO_DIRCN_DEFAULT                         0x00
    #define LMP90100_GPIO_DIRCN_GPIO0                           0
    #define LMP90100_GPIO_DIRCN_GPIO1                           1
    #define LMP90100_GPIO_DIRCN_GPIO2                           2
    #define LMP90100_GPIO_DIRCN_GPIO3                           3
    #define LMP90100_GPIO_DIRCN_GPIO4                           4
    #define LMP90100_GPIO_DIRCN_GPIO5                           5
    #define LMP90100_GPIO_DIRCN_GPIO6                           6

#define LMP90100_REG_GPIO_DAT                                   0x0F

#endif
