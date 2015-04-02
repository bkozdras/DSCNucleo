#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_msp.h"
#include "stm32f4xx_hal_uart.h"
#include "Defines/GPIOMacros.h"
#include "HardwareSettings/GPIODefines.h"
#include "FaultManagement/FaultIndication.h"
#include "Utilities/Logger/Logger.h"

#include "Peripherals/UART1.h"

static bool mIsInitialized = false;
UART_HandleTypeDef mUart1Handle;
static DMA_HandleTypeDef mDMAHandleTx;
static DMA_HandleTypeDef mDMAHandleRx;
static void (*mTransmittingDoneCallback)(void) = NULL;
static void (*mReceivingDoneCallback)(void) = NULL;

static void mspInit(UART_HandleTypeDef *uartHandle);
static void mspDeInit(UART_HandleTypeDef *uartHandle);

void UART1_initializeDefault(void)
{
    UART1_initialize(1000000, UART_MODE_TX_RX);
}

void UART1_initialize(TUartBaudRate baudRate, TUartMode mode)
{
    if (!mIsInitialized)
    {
        MSP_setHAL_UART_MspInitCallback(mspInit);
        
        mUart1Handle.Instance          = USART1;
        mUart1Handle.Init.BaudRate     = baudRate;
        mUart1Handle.Init.WordLength   = UART_WORDLENGTH_8B;
        mUart1Handle.Init.StopBits     = UART_STOPBITS_1;
        mUart1Handle.Init.Parity       = UART_PARITY_NONE;
        mUart1Handle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
        mUart1Handle.Init.Mode         = mode;
        mUart1Handle.Init.OverSampling = UART_OVERSAMPLING_16;
        
        if(HAL_OK != HAL_UART_Init(&mUart1Handle))
        {
            Logger_error("UART1: Initialization failed!");
            /* Initialization Error */
            FaultIndication_start(EFaultId_Uart, EUnitId_Nucleo, EUnitId_Empty);
            mIsInitialized = false;
        }
        else
        {
            Logger_debug("UART1: Initialized!");
            mIsInitialized = true;
        }
    }
}

void UART1_uninitialize(void)
{
    MSP_setHAL_UART_MspDeInitCallback(mspDeInit);
    
    if(HAL_OK != HAL_UART_DeInit(&mUart1Handle))
    {
        FaultIndication_start(EFaultId_Uart, EUnitId_Nucleo, EUnitId_Empty);
        assert_param(false);
    }
    else
    {
        mIsInitialized = false;
    }
}

bool UART1_transmit(TByte* data, const u16 dataLength)
{
    if (mIsInitialized)
    {
        if (HAL_OK != HAL_UART_Transmit_DMA(&mUart1Handle, data, dataLength))
        {
            return false;
        }
        
        Logger_debugSystem("UART1: Transmitted %u bytes:", dataLength);
        for (u16 iter = 0; dataLength > iter; ++iter)
        {
            Logger_debugSystem("UART1: Byte[%u]: 0x%02X.", iter, data[iter]);
        }
        
        return true;
    }
    
    return false;
}

bool UART1_receive(TByte* data, const u16 dataLength)
{
    if (mIsInitialized)
    {
        if (HAL_OK != HAL_UART_Receive_DMA(&mUart1Handle, data, dataLength))
        {
            return false;
        }

        return true;
    }
    
    return false;
}

void UART1_registerDataTransmittingDoneCallback(void (*transmittingDoneCallback)(void))
{
    mTransmittingDoneCallback = transmittingDoneCallback;
}

void UART1_deregisterDataTransmittingDoneCallback(void)
{
    mTransmittingDoneCallback = NULL;
}

void UART1_registerDataReceivingDoneCallback(void (*receivingDoneCallback)(void))
{
    mReceivingDoneCallback = receivingDoneCallback;
}

void UART1_deregisterDataReceivingDoneCallback(void)
{
    mReceivingDoneCallback = NULL;
}

bool UART1_isInitialized(void)
{
    return mIsInitialized;
}

void mspInit(UART_HandleTypeDef* uartHandle)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  GPIO_CLOCK_ENABLE(UART1_TX_PORT);
  GPIO_CLOCK_ENABLE(UART1_RX_PORT);
  
  /* Enable USARTx clock */
  __HAL_RCC_USART1_CLK_ENABLE();
  
  /* Enable DMA2 clock*/
  __HAL_RCC_DMA2_CLK_ENABLE();
    
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = GET_GPIO_PIN(UART1_TX_PIN);
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  
  HAL_GPIO_Init(GET_GPIO_PORT(UART1_TX_PORT), &GPIO_InitStruct);
    
  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = GET_GPIO_PIN(UART1_RX_PIN);
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    
  HAL_GPIO_Init(GET_GPIO_PORT(UART1_RX_PORT), &GPIO_InitStruct);
  
  // DMA Configuration
  
   //##-3- Configure the DMA streams ##########################################
  // Configure the DMA handler for Transmission process 
  mDMAHandleTx.Instance                 = DMA2_Stream7;
  
  mDMAHandleTx.Init.Channel             = DMA_CHANNEL_4;
  mDMAHandleTx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  mDMAHandleTx.Init.PeriphInc           = DMA_PINC_DISABLE;
  mDMAHandleTx.Init.MemInc              = DMA_MINC_ENABLE;
  mDMAHandleTx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  mDMAHandleTx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  mDMAHandleTx.Init.Mode                = DMA_NORMAL;
  mDMAHandleTx.Init.Priority            = DMA_PRIORITY_HIGH;
  mDMAHandleTx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  mDMAHandleTx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  mDMAHandleTx.Init.MemBurst            = DMA_MBURST_SINGLE;
  mDMAHandleTx.Init.PeriphBurst         = DMA_PBURST_SINGLE;
  
  HAL_DMA_Init(&mDMAHandleTx);   
  
  // Associate the initialized DMA handle to the the UART handle
  __HAL_LINKDMA(uartHandle, hdmatx, mDMAHandleTx);
    
  // Configure the DMA handler for Transmission process 
  mDMAHandleRx.Instance                 = DMA2_Stream5;
  
  mDMAHandleRx.Init.Channel             = DMA_CHANNEL_4;
  mDMAHandleRx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  mDMAHandleRx.Init.PeriphInc           = DMA_PINC_DISABLE;
  mDMAHandleRx.Init.MemInc              = DMA_MINC_ENABLE;
  mDMAHandleRx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  mDMAHandleRx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  mDMAHandleRx.Init.Mode                = DMA_NORMAL;
  mDMAHandleRx.Init.Priority            = DMA_PRIORITY_HIGH;
  mDMAHandleRx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  mDMAHandleRx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  mDMAHandleRx.Init.MemBurst            = DMA_MBURST_INC4;
  mDMAHandleRx.Init.PeriphBurst         = DMA_PBURST_INC4; 

  HAL_DMA_Init(&mDMAHandleRx);
    
  // Associate the initialized DMA handle to the the UART handle 
  __HAL_LINKDMA(uartHandle, hdmarx, mDMAHandleRx);
    
  //##-4- Configure the NVIC for DMA #########################################
  // NVIC configuration for DMA transfer complete interrupt (USARTx_TX)
  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 7, 1);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
    
  // NVIC configuration for DMA transfer complete interrupt (USARTx_RX)
  HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 7, 0);   
  HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
  
  // NVIC configuration for USART TC interrupt
  HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void mspDeInit(UART_HandleTypeDef* uartHandle)
{
  /*##-1- Reset peripherals ##################################################*/
  __HAL_RCC_USART1_FORCE_RESET();
  __HAL_RCC_USART1_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(GET_GPIO_PORT(UART1_TX_PORT), GET_GPIO_PIN(UART1_TX_PIN));
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(GET_GPIO_PORT(UART1_RX_PORT), GET_GPIO_PIN(UART1_RX_PIN));
    
  /*##-3- Disable the DMA Streams ############################################*/
  /* De-Initialize the DMA Stream associate to transmission process */
  HAL_DMA_DeInit(&mDMAHandleTx); 
  /* De-Initialize the DMA Stream associate to reception process */
  HAL_DMA_DeInit(&mDMAHandleRx);
  
  /*##-4- Disable the NVIC for DMA ###########################################*/
  HAL_NVIC_DisableIRQ(DMA2_Stream7_IRQn);
  HAL_NVIC_DisableIRQ(DMA2_Stream5_IRQn);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* uartHandle)
{
    __HAL_UART_FLUSH_DRREGISTER(uartHandle);
    if (&mUart1Handle == uartHandle)
    {
        if (mTransmittingDoneCallback)
        {
            (*mTransmittingDoneCallback)();
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* uartHandle)
{
    if (&mUart1Handle == uartHandle)
    {
        if (mReceivingDoneCallback)
        {
            (*mReceivingDoneCallback)();
        }
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    Logger_debugSystem("UART1: Transmission failure. Error callback occured.");
    FaultIndication_start(EFaultId_Uart, EUnitId_Nucleo, EUnitId_Empty);
}
