#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_msp.h"
#include "stm32f4xx_hal_uart.h"
#include "Defines/GPIOMacros.h"
#include "HardwareSettings/GPIODefines.h"
#include "FaultManagement/FaultIndication.h"

#include "Peripherals/UART2.h"

static bool mIsInitialized = false;
static UART_HandleTypeDef mUartHandle;

static void mspInit(UART_HandleTypeDef *uartHandle);
static void mspDeInit(UART_HandleTypeDef *uartHandle);

void UART2_initializeDefault(void)
{
    UART2_initialize(115200, UART_MODE_TX_RX);
}

void UART2_initialize(TUartBaudRate baudRate, TUartMode mode)
{
    if (!mIsInitialized)
    {
        MSP_setHAL_UART_MspInitCallback(mspInit);
        
        mUartHandle.Instance          = USART2;
        mUartHandle.Init.BaudRate     = baudRate;
        mUartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
        mUartHandle.Init.StopBits     = UART_STOPBITS_1;
        mUartHandle.Init.Parity       = UART_PARITY_NONE;
        mUartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
        mUartHandle.Init.Mode         = mode;
        mUartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
        
        if(HAL_OK != HAL_UART_Init(&mUartHandle))
        {
            /* Initialization Error */
            FaultIndication_start(EFaultId_Uart, EUnitId_Nucleo, EUnitId_Empty);
            mIsInitialized = false;
        }
        else
        {
            mIsInitialized = true;
        }
    }
}

void UART2_uninitialize(void)
{
    MSP_setHAL_UART_MspDeInitCallback(mspDeInit);
    
    if(HAL_OK != HAL_UART_DeInit(&mUartHandle))
    {
        FaultIndication_start(EFaultId_Uart, EUnitId_Nucleo, EUnitId_Empty);
        assert_param(false);
    }
    else
    {
        mIsInitialized = false;
    }
}

void UART2_send(const TByte* data, const u8 dataLength)
{
    if (mIsInitialized)
    {
        for (u8 iter = 0; dataLength > iter; ++iter)
        {
            UART2_sendByte(data[iter]);
        }
    }
}

void UART2_sendByte(TByte byte)
{
    if (mIsInitialized)
    {
        while(__HAL_UART_GET_FLAG(&mUartHandle, UART_FLAG_TXE) == RESET);
        HAL_UART_Transmit(&mUartHandle, &byte, 1, 0xFFFF);
    }
}

//void UART2_setReceiverThreadId();

bool UART2_isInitialized(void)
{
    return mIsInitialized;
}

void mspInit(UART_HandleTypeDef *uartHandle)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  GPIO_CLOCK_ENABLE(UART2_TX_PORT);
  GPIO_CLOCK_ENABLE(UART2_RX_PORT);
  
  /* Enable USARTx clock */
  __HAL_RCC_USART2_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = GET_GPIO_PIN(UART2_TX_PIN);
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  
  HAL_GPIO_Init(GET_GPIO_PORT(UART2_TX_PORT), &GPIO_InitStruct);
    
  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = GET_GPIO_PIN(UART2_RX_PIN);
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    
  HAL_GPIO_Init(GET_GPIO_PORT(UART2_RX_PORT), &GPIO_InitStruct);
}

void mspDeInit(UART_HandleTypeDef *uartHandle)
{
  /*##-1- Reset peripherals ##################################################*/
  __HAL_RCC_USART2_FORCE_RESET();
  __HAL_RCC_USART2_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(GET_GPIO_PORT(UART2_TX_PORT), GET_GPIO_PIN(UART2_TX_PIN));
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(GET_GPIO_PORT(UART2_RX_PORT), GET_GPIO_PIN(UART2_RX_PIN));    
}
