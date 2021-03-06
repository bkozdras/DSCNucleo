#include "Devices/LMP90100ControlSystem.h"
#include "Devices/LMP90100Registers.h"

#include "Peripherals/EXTI.h"
#include "Peripherals/SPI2.h"

#include "Utilities/Logger/Logger.h"
#include "Utilities/Printer/CStringConverter.h"
#include "Utilities/Comparator.h"

#include "Defines/CommonDefines.h"
#include "Defines/CommonMacros.h"
#include "Defines/GPIOMacros.h"
#include "HardwareSettings/GPIODefines.h"

#include "FaultManagement/FaultIndication.h"

#include "cmsis_os.h"

#include "stm32f4xx_hal.h"

#include "string.h"

THREAD_DEFINES(LMP90100ControlSystemController, LMP90100ControlSystem)
EVENT_HANDLER_PROTOTYPE(DeviceDataReadyInd)

static bool mIsInitialized = false;
static EThreadId mObserverThreadId = EThreadId_Unknown;
static TByte mLastUraByte = 0xFF;
static float mGainValue = 1.0F;
//static const float mComparatorResistanceValue = 0.0F; // 1 kOhm
static const float mComparatorResistanceValue = 1000.0F; // 1 kOhm
//static const float mReferenceResistanceValue = 3212.0F; // 100 Ohm
static const float mReferenceResistanceValue = 1487.0F; // 1.5 kOhm
static float mActualRTDValue = 0.0F;
static ELMP90100Mode mActualDeviceMode = ELMP90100Mode_Off;

static void initializeGpio(void);
static bool configureRegisters(ELMP90100Mode newMode);
static bool turnOffDevice(void);
static void dataReadyCallback(void);
static bool readAdcData(u32* adcData);
//static bool isAdcDataReady(void);
static float convertAdcDataToRTDValue(u32 adcData);
static void notifyObserverAboutNewRTDValue(void);

static bool transmitData(const TByte firstRegisterAddress, const TByte* data, const u8 dataLength, const TTimeMs timeout);
static bool receiveData(const TByte firstRegisterAddress, TByte* receivedData, const u8 dataLength, const TTimeMs timeout);
static bool transmitByteAndValidate(const TByte registerAddress, const TByte data);
static bool verifyCrcByte(const TByte crcByte, TByte* readAdcData, const u8 adcDataLength);
static void assertCSB(void);
static void deassertCSB(void);
static bool sendRegisterSetupInstructions(TSpiOperation spiOperation, TByte firstRegisterAddress, u8 dataLength);
static void updateLastUraByte(TByte byte);
static bool isUraConfiguringNeeded(TByte firstRegisterAddress);
static TByte getUraInstructionByte(void);
static TByte getUraSetupByte(TByte registerWithUra);
static TByte getDataAccessByte(TSpiOperation spiOperation, TByte registerWithDataAccess, u8 dataLength);
static TByte getSPSRegisterContent(ELMP90100Mode newMode);

THREAD(LMP90100ControlSystemController)
{
    THREAD_SKELETON_START
    
        EVENT_HANDLING(DeviceDataReadyInd)
    
    THREAD_SKELETON_END
}

EVENT_HANDLER(DeviceDataReadyInd)
{
    static u8 wrongDataCounter = 0;
    
    Logger_debug("%s: Reading ADC Data.", getLoggerPrefix());
    u32 readData;
    bool status = readAdcData(&readData);
    
    if (!status)
    {
        Logger_debug("%s: Received wrong data from device. New data skipped.", getLoggerPrefix());
        ++wrongDataCounter;
        
        if (5 == wrongDataCounter)
        {
            Logger_error("%s: Wrong data counter from device happened 5 times in a row! Permanent error...", getLoggerPrefix());
            FaultIndication_start(EFaultId_WrongData, EUnitId_Nucleo, EUnitId_LMP90100ControlSystem);
        }
    }
    else
    {
        wrongDataCounter = 0;
        
        Logger_debug("%s: Read ADC value: %u.", getLoggerPrefix(), readData);
        mActualRTDValue = convertAdcDataToRTDValue(readData);
        Logger_debug("%s: Actual RTD value: %.4f Ohm.", getLoggerPrefix(), mActualRTDValue);
        notifyObserverAboutNewRTDValue();
    }
}

void LMP90100ControlSystem_setup(void)
{
    THREAD_INITIALIZE_MUTEX
    
    initializeGpio();
}

bool LMP90100ControlSystem_initialize(void)
{
    Logger_debug("%s: Device initialization.", getLoggerPrefix());
    
    if (!SPI2_isInitialized())
    {
        SPI2_initialize();
    }
    
    osMutexWait(mMutexId, osWaitForever);
    
    //EXTI_setCallback(GET_GPIO_PIN(LMP90100_CONTROL_SYSTEM_DRDYB_PIN), dataReadyCallback);
    //configureRegisters();
    bool result = turnOffDevice();
    
    osMutexRelease(mMutexId);
    
    if (result)
    {
        mIsInitialized = true;
        Logger_info("%s: Device initialized!", getLoggerPrefix());
    }
    else
    {
        Logger_error("%s: Device initialization failed!", getLoggerPrefix());
    }
    
    return result;
}

bool LMP90100ControlSystem_isInitialized(void)
{
    bool isInitialized;
    osMutexWait(mMutexId, osWaitForever);
    isInitialized = mIsInitialized;
    osMutexRelease(mMutexId);
    return isInitialized;
}

bool LMP90100ControlSystem_changeMode(ELMP90100Mode newMode)
{
    osMutexWait(mMutexId, osWaitForever);
    
    Logger_info("%s: Setting new device mode: %s.", getLoggerPrefix(), CStringConverter_ELMP90100Mode(newMode));
    
    bool status = true;
    
    if (mActualDeviceMode != newMode)
    {
        if ( ( ELMP90100Mode_Off != mActualDeviceMode ) || ( ELMP90100Mode_Off == newMode ) )
        {
            EXTI_unsetCallback(GET_GPIO_PIN(LMP90100_CONTROL_SYSTEM_DRDYB_PIN));
            status = turnOffDevice();
        }
        
        if ( status && ( ELMP90100Mode_Off != newMode ) )
        {
            status = configureRegisters(newMode);
            EXTI_setCallback(GET_GPIO_PIN(LMP90100_CONTROL_SYSTEM_DRDYB_PIN), dataReadyCallback);
        }
        
        if (status)
        {
            mActualDeviceMode = newMode;
            Logger_info("%s: Device new mode: %s.", getLoggerPrefix(), CStringConverter_ELMP90100Mode(newMode));
        }
        else
        {
            Logger_error("%s: Device mode changing to %s failed!", getLoggerPrefix(), CStringConverter_ELMP90100Mode(newMode));
        }
    }
    else
    {
        status = false;
        Logger_warning("%s: Operation skipped. New mode is the same as actual!", getLoggerPrefix());
    }
    
    osMutexRelease(mMutexId);
    
    return status;
}

void LMP90100ControlSystem_registerNewValueObserver(EThreadId threadId)
{
    osMutexWait(mMutexId, osWaitForever);
    mObserverThreadId = threadId;
    Logger_debug("%s: ADC data new value observer (%s) registered.", getLoggerPrefix(), CStringConverter_EThreadId(mObserverThreadId));
    osMutexRelease(mMutexId);
}

void LMP90100ControlSystem_deregisterNewValueObserver(void)
{
    osMutexWait(mMutexId, osWaitForever);
    Logger_debug("%s: ADC data new value observer (%s) deregistered.", getLoggerPrefix(), CStringConverter_EThreadId(mObserverThreadId));
    mObserverThreadId = EThreadId_Unknown;
    osMutexRelease(mMutexId);
}

void initializeGpio(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // CSB
    
    GPIO_CLOCK_ENABLE(LMP90100_CONTROL_SYSTEM_CSB_PORT);
    
    GPIO_InitStruct.Pin = GET_GPIO_PIN(LMP90100_CONTROL_SYSTEM_CSB_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GET_GPIO_PORT(LMP90100_CONTROL_SYSTEM_CSB_PORT), &GPIO_InitStruct);
    
    deassertCSB();
    
    // DRDYB
    
    GPIO_CLOCK_ENABLE(LMP90100_CONTROL_SYSTEM_DRDYB_PORT);

    GPIO_InitStruct.Pin = GET_GPIO_PIN(LMP90100_CONTROL_SYSTEM_DRDYB_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    //GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GET_GPIO_PORT(LMP90100_CONTROL_SYSTEM_DRDYB_PORT), &GPIO_InitStruct);
}

bool configureRegisters(ELMP90100Mode newMode)
{
    TByte writeData[14];
    TByte registerData[14];
    u8 counter = 0;
    
    TByte writeData2[6];
    TByte registerData2[6];
    u8 counter2 = 0;
    
    writeData[counter] = LMP90100_SPI_HANDSHAKECN_SW_OFF_TRG_POSTPONED;
    registerData[counter++] = LMP90100_REG_SPI_HANDSHAKECN;
    
    writeData[counter] = LMP90100_SPI_STREAMCN_STRM_TYPE_NORMAL_MODE;
    registerData[counter++] = LMP90100_REG_SPI_STREAMCN;
    
    writeData[counter] = LMP90100_PWRCN_ACTIVE_MODE;
    registerData[counter++] = LMP90100_REG_PWRCN;
    
    writeData[counter] = LMP90100_ADC_RESTART_DISABLED;
    registerData[counter++] = LMP90100_REG_ADC_RESTART;
    
    writeData[counter] = LMP90100_REG_GPIO_DIRCN_DEFAULT;
    RESET_BIT_IN_BYTE(writeData[counter], LMP90100_GPIO_DIRCN_GPIO5);
    registerData[counter++] = LMP90100_REG_GPIO_DIRCN;
    
    writeData[counter] = LMP90100_BGCALN_OFFSET_ESTIMATION_GAIN_ESTIMATION;
    registerData[counter++] = LMP90100_REG_BGCALCN;
    
    writeData[counter] = LMP90100_REG_SPI_DRDYBCN_DEFAULT;
    writeData[counter] |= LMP90100_SPI_DRDYBCN_SPI_DRDYB_D6_DRDYB_SIGNAL;
    registerData[counter++] = LMP90100_REG_SPI_DRDYBCN;

    writeData[counter] = LMP90100_REG_ADC_AUXCN_DEFAULT;
    writeData[counter] |= LMP90100_ADC_AUXCN_CLK_EXT_DET_BYPASSED;
    writeData[counter] |= LMP90100_ADC_AUXCN_CLK_SEL_INTERNAL;
    //writeData[counter] |= LMP90100_ADC_AUXCN_RTD_CUR_SEL_300UA;
    writeData[counter] |= LMP90100_ADC_AUXCN_RTD_CUR_SEL_400UA;
    registerData[counter++] = LMP90100_REG_ADC_AUXCN;
    
    writeData[counter] = LMP90100_REG_SPI_CRC_CN_DEFAULT;
    writeData[counter] |= LMP90100_SPI_SRC_CN_EN_CRC_ENABLED;
    writeData[counter] |= LMP90100_SPI_SRC_CN_EN_CRC_DRDYB_AFT_CRC_CRC;
    registerData[counter++] = LMP90100_REG_SPI_CRC_CN;
    
    writeData[counter] = LMP90100_REG_SENDIAG_THLDH_DEFAULT;
    registerData[counter++] = LMP90100_REG_SENDIAG_THLDH;
    
    writeData[counter] = LMP90100_REG_SENDIAG_THLDL_DEFAULT;
    registerData[counter++] = LMP90100_REG_SENDIAG_THLDL;

    writeData[counter] = LMP90100_REG_SCALCN_DEFAULT;
    registerData[counter++] = LMP90100_REG_SCALCN;
    
    writeData[counter] = LMP90100_REG_ADC_DONE_DEFAULT;
    registerData[counter++] = LMP90100_REG_ADC_DONE;
    
    writeData2[counter2] = 0x00;
    writeData2[counter2] |= LMP90100_CHx_INPUTCN_BURNOUT_EN_DISABLED;
    writeData2[counter2] |= LMP90100_CHx_INPUTCN_VREF_SEL_2;
    writeData2[counter2] |= LMP90100_CHx_INPUTCN_VINP_VIN0;
    writeData2[counter2] |= LMP90100_CHx_INPUTCN_VINN_VIN1;
    registerData2[counter2++] = LMP90100_REG_CHx_INPUTCN_CH0;
    
    writeData2[counter2] = LMP90100_REG_CH_SCAN_DEFAULT;
    writeData2[counter2] |= LMP90100_CH_SCAN_CH_SCAN_SEL_SCANMODE2;
    writeData2[counter2] |= LMP90100_CH_SCAN_LAST_CH_CH0;
    writeData2[counter2] |= LMP90100_CH_SCAN_FIRST_CH_CH0;
    registerData2[counter2++] = LMP90100_REG_CH_SCAN;

    writeData2[counter2] = 0x00;
    writeData2[counter2] |= getSPSRegisterContent(newMode);
    writeData2[counter2] |= LMP90100_CHx_CONFIG_GAIN_SEL_1_FGA_OFF;
    writeData2[counter2] |= LMP90100_CHx_CONFIG_BUF_EN_BUFFER_INCLUDED;
    registerData2[counter2++] = LMP90100_REG_CHx_CONFIG_CH0;

    writeData2[counter2] = LMP90100_PWRCN_ACTIVE_MODE;
    registerData2[counter2++] = LMP90100_REG_PWRCN;

    TByte data = LMP90100_REG_AND_CNV_RST;
    transmitData(LMP90100_REG_RESETCN, &data, 1, 100);
    
    for (u8 iter = 0; counter > iter; ++iter)
    {
        if (!transmitData(registerData[iter], &(writeData[iter]), 1, 100))
        {
            Logger_error("%s: Configuring registers: Data validating failure!", getLoggerPrefix());
            return false;
        }
    }
    
    bool isChScanReady = false;
    while (!isChScanReady)
    {
        osDelay(1);
        TByte readData;
        receiveData(LMP90100_REG_CH_STS, &readData, 1, 100);
        isChScanReady = !(readData & 0x02);
    }
    
    for (u8 iter = 0; counter2 > iter; ++iter)
    {
        if (!transmitData(registerData2[iter], &(writeData2[iter]), 1, 100))
        {
            Logger_error("%s: Configuring registers: Data validating failure!", getLoggerPrefix());
            return false;
        }
    }
    
    Logger_debug("%s: Configuring registers: DONE!", getLoggerPrefix());
    return true;
}

bool turnOffDevice(void)
{
    TByte writeData = LMP90100_REG_PWRCN_DEFAULT;
    writeData |= LMP90100_PWRCN_STAND_BY_MODE;
    
    if (transmitData(LMP90100_REG_PWRCN, &writeData, 1, 100))
    {
        Logger_info("%s: Device is in STAND-BY state!", getLoggerPrefix());
        return true;
    }
    else
    {
        Logger_error("%s: Forcing device new STAND-BY state failed!", getLoggerPrefix());
        return false;
    }
}

bool readAdcData(u32* adcData)
{
    TByte readData [5];
    if (!receiveData(LMP90100_REG_SENDIAG_FLAGS, readData, 5, 100))
    {
        Logger_error("%s: Reading ADC Data: Communication failure!", getLoggerPrefix());
        return false;
    }
    
    readData[0] &= LMP90100_SENDIAG_FLAGS_OFLO_SAMPLED_CHANNEL_MASK;
    Logger_debug("%s: Read channel: 0x%02X. CRC data: 0x%02X.", getLoggerPrefix(), readData[0], readData[4]);
    
    if (verifyCrcByte(readData[4], &(readData[1]), 3))
    {
        u32 newValue = 0x00;
        newValue |= (((u32)(readData[3])) & 0xFF);
        newValue |= ((((u32) (readData[2]) ) << 8) & 0xFF00);
        newValue |= ((((u32) (readData[1]) ) << 16) & 0xFF0000);
        *adcData = newValue;
        return true;
    }
    else
    {
        Logger_error("%s: CRC verification failed. ADC data is not correct!", getLoggerPrefix());
        return false;
    }
}
/*
bool isAdcDataReady(void)
{
    Logger_debug("%s: Checking is ADC data ready.", getLoggerPrefix());
    TByte readData;
    if (!receiveData(LMP90100_REG_ADC_DONE, &readData, 1, 100))
    {
        Logger_warning("%s: IsAdcDataReady: Communication failure!", getLoggerPrefix());
        return false;
    }
    
    if (0xFF == readData)
    {
        Logger_warning("%s: IsAdcDataReady: Not ready (ADC_DONE value: 0x%02X).", getLoggerPrefix(), readData);
        return false;
    }
    
    Logger_debug("%s: ADC data is ready.", getLoggerPrefix());
    
    return true;
}*/

float convertAdcDataToRTDValue(u32 adcData)
{
    const float floatAdcData = (float) adcData;
    return ( ( 4.0F * mReferenceResistanceValue * floatAdcData ) / ( 16777216.0F * mGainValue ) + mComparatorResistanceValue );
}

void notifyObserverAboutNewRTDValue(void)
{
    if (EThreadId_Unknown != mObserverThreadId)
    {
        CREATE_EVENT(NewRTDValueInd, mObserverThreadId);
        CREATE_EVENT_MESSAGE(NewRTDValueInd);
        
        eventMessage->value = mActualRTDValue;
        Logger_debug
        (
            "%s: Sending notification %s to %s about new RTD value: %.4f Ohm.",
            getLoggerPrefix(),
            CStringConverter_EEventId(event->id),
            CStringConverter_EThreadId(mObserverThreadId),
            eventMessage->value
        );
        
        SEND_EVENT();
    }
}

void dataReadyCallback(void)
{
    CREATE_EVENT_ISR(DeviceDataReadyInd, mThreadId);
    SEND_EVENT();
}

bool sendRegisterSetupInstructions(TSpiOperation spiOperation, TByte firstRegisterAddress, u8 dataLength)
{
    TByte setupData [3];
    
    bool isUraConfigureNeeded = isUraConfiguringNeeded(firstRegisterAddress);
    
    setupData[2] = getDataAccessByte(spiOperation, firstRegisterAddress, dataLength);
    
    bool spi2TransmittingStatus = false;
    
    if (isUraConfiguringNeeded(firstRegisterAddress))
    {
        setupData[0] = getUraInstructionByte();
        setupData[1] = getUraSetupByte(firstRegisterAddress);
        spi2TransmittingStatus = SPI2_transmit(setupData, 3, 1000);
    }
    else
    {
        spi2TransmittingStatus = SPI2_transmit(&(setupData[2]), 1, 1000);
    }
    
    if (spi2TransmittingStatus)
    {
        updateLastUraByte(firstRegisterAddress);
    }
    else
    {
        Logger_error("%s: Communication failure during sending setup instructions (Register: 0x%02X).", getLoggerPrefix(), firstRegisterAddress);
        FaultIndication_start(EFaultId_Spi, EUnitId_Nucleo, EUnitId_LMP90100ControlSystem);
        return false;
    }
    
    return true;
}

bool transmitData(const TByte firstRegisterAddress, const TByte* data, const u8 dataLength, const TTimeMs timeout)
{
    Logger_debug("%s: Sending data (First register address: 0x%02X, Data Length: %u).", getLoggerPrefix(), firstRegisterAddress, dataLength);
    assertCSB();
    sendRegisterSetupInstructions(TSpiOperation_Write, firstRegisterAddress, dataLength);
    bool status = SPI2_transmit(data, dataLength, timeout);
    deassertCSB();
    if (!status)
    {
        Logger_error("%s: Communication failure during sending data (Length: %u).", getLoggerPrefix(), dataLength);
        FaultIndication_start(EFaultId_Spi, EUnitId_Nucleo, EUnitId_LMP90100ControlSystem);
        return false;
    }
    return true;
}

bool receiveData(const TByte firstRegisterAddress, TByte* receivedData, const u8 dataLength, const TTimeMs timeout)
{
    Logger_debug("%s: Receiving data (First register address: 0x%02X, Data Length: %u).", getLoggerPrefix(), firstRegisterAddress, dataLength);
    assertCSB();
    sendRegisterSetupInstructions(TSpiOperation_Read, firstRegisterAddress, dataLength);
    bool status = SPI2_receive(receivedData, dataLength, timeout);
    deassertCSB();
    if (!status)
    {
        Logger_error("%s: Communication failure during reading data (Length: %u).", getLoggerPrefix(), dataLength);
        FaultIndication_start(EFaultId_Spi, EUnitId_Nucleo, EUnitId_LMP90100ControlSystem);
        return false;
    }
    
    return true;
}

bool transmitByteAndValidate(const TByte registerAddress, const TByte data)
{
    if (!transmitData(registerAddress, &data, 1, 100))
    {
        Logger_error("%s: Validating transmitting data: Trasmitting failed!", getLoggerPrefix());
        return false;
    }
    
    TByte readData;
    
    if (!receiveData(registerAddress, &readData, 1, 100))
    {
        Logger_error("%s: Validating transmitting data: Receiving failed!", getLoggerPrefix());
        return false;
    }
    
    if (!Comparator_u8(&data, &readData, 1))
    {
        Logger_error("%s: Validating transmitting data: Comparing failed (Device register: 0x%02X)!", getLoggerPrefix(), registerAddress);
        return false;
    }
    
    return true;
}

bool verifyCrcByte(const TByte crcByte, TByte* readAdcData, const u8 adcDataLength)
{
    // CRC-8: x8 + x5 + x4 + 1
    const TByte crcPoly = 0x31;
    TByte crc = 0x0;
    const TByte crcFinalXor = 0xFF;
    
    u8 msg;

    for(u8 i = 0; adcDataLength > i; ++i)
    {
        msg = (*readAdcData++ << 0);
        for(u8 j = 0; 8 > j; ++j)
        {
            if( (msg ^ crc) >> 7 )
            {
                crc = (crc << 1) ^ crcPoly;
            }
            else
            {
                crc <<= 1;
            }
            msg <<= 1;
        }
    }
    
    TByte crcCalculated = ( crc ^ crcFinalXor );
    
    if (crcByte == crcCalculated)
    {
        Logger_debug("%s: Received CRC: 0x%02X is equal to the calculated: 0x%02X.", getLoggerPrefix(), crcByte, crcCalculated);
        return true;
    }
    else
    {
        Logger_error("%s: Received CRC: 0x%02X is not equal to the calculated: 0x%02X.", getLoggerPrefix(), crcByte, crcCalculated);
        return false;
    }
}

void assertCSB(void)
{
    GPIO_SET_LOW_LEVEL(LMP90100_CONTROL_SYSTEM_CSB_PORT, LMP90100_CONTROL_SYSTEM_CSB_PIN);
}

void deassertCSB(void)
{
    GPIO_SET_HIGH_LEVEL(LMP90100_CONTROL_SYSTEM_CSB_PORT, LMP90100_CONTROL_SYSTEM_CSB_PIN);
}

void updateLastUraByte(TByte byte)
{
    mLastUraByte = ((byte >> 4) & 0x0F);
}

bool isUraConfiguringNeeded(TByte firstRegisterAddress)
{
    const TByte uraByte = ((firstRegisterAddress >> 4) & 0x0F);
    return ( !(uraByte == mLastUraByte) );
}

TByte getUraInstructionByte(void)
{
    return LMP90100_URA_WRITE;
}

TByte getUraSetupByte(TByte registerWithUra)
{
    return ((registerWithUra >> 4) & 0x0F);
}

TByte getDataAccessByte(TSpiOperation spiOperation, TByte registerWithDataAccess, u8 dataLength)
{
    TByte byte = 0x00;
    if (TSpiOperation_Read == spiOperation)
    {
        byte |= LMP90100_DA_READ;
    }
    else
    {
        byte |= LMP90100_DA_WRITE;
    }
    
    switch(dataLength)
    {
        case 1 :
        {
            byte |= LMP90100_DA_ONE_BYTE_READ;
            break;
        }
        
        case 2 :
        {
            byte |= LMP90100_DA_TWO_BYTES_READ;
            break;
        }
        
        case 3 :
        {
            byte |= LMP90100_DA_THREE_BYTES_READ;
            break;
        }
        
        default :
        {
            byte |= LMP90100_DA_STREAMING_READ;
            break;
        }
    }
    
    byte |= (registerWithDataAccess & 0x0F);
    
    return byte;
}

TByte getSPSRegisterContent(ELMP90100Mode newMode)
{
    switch (newMode)
    {
        case ELMP90100Mode_On_1_6775_SPS :
            return LMP90100_CHx_CONFIG_ODR_SEL_1_6775_SPS;
        
        case ELMP90100Mode_On_3_355_SPS :
            return LMP90100_CHx_CONFIG_ODR_SEL_3_355_SPS;
        
        case ELMP90100Mode_On_6_71_SPS :
            return LMP90100_CHx_CONFIG_ODR_SEL_6_71_SPS;
        
        case ELMP90100Mode_On_13_42_SPS :
            return LMP90100_CHx_CONFIG_ODR_SEL_13_42_SPS;
        
        case ELMP90100Mode_On_26_83125_SPS :
            return LMP90100_CHx_CONFIG_ODR_SEL_26_83125_SPS;
        
        case ELMP90100Mode_On_53_6625_SPS :
            return LMP90100_CHx_CONFIG_ODR_SEL_53_6625_SPS;
        
        case ELMP90100Mode_On_107_325_SPS :
            return LMP90100_CHx_CONFIG_ODR_SEL_107_325_SPS;
        
        case ELMP90100Mode_On_214_65_SPS :
            return LMP90100_CHx_CONFIG_ODR_SEL_214_65_SPS;
        
        default :
            return 0x00;
    }
}
